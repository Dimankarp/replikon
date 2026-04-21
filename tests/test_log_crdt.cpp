#include "gtest/gtest.h"
#include <gtest/gtest.h>

#include "constants.h"
#include "crdt/log.h"
#include "dao/message.h"
#include "sqlite.h"
#include "utils.h"

#include <iostream>
#include <memory>

using namespace replikon;

class LogCRDTTest : public ::testing::Test {
protected:
  void SetUp() override {
    db1 = std::make_shared<db::Sqlite>();
    db1->Connect(":memory:");
    initDb(db1);

    db2 = std::make_shared<db::Sqlite>();
    db2->Connect(":memory:");
    initDb(db2);

    dao1 = std::make_shared<dao::Messages>(db1);
    dao2 = std::make_shared<dao::Messages>(db2);

    log1 = std::make_unique<Log<ChatMessage>>(dao1);
    log2 = std::make_unique<Log<ChatMessage>>(dao2);
  }

  void initDb(std::shared_ptr<db::Sqlite> &db) {
    db->PrepareStatement(INIT_MESSAGES).value().Step();
    db->PrepareStatement(INDEX_MESSAGES).value().Step();
    db->PrepareStatement(TEMP_SEARCH_INTERVALS).value().Step();
  }

  std::shared_ptr<db::Sqlite> db1, db2;
  std::shared_ptr<dao::Messages> dao1, dao2;
  std::unique_ptr<Log<ChatMessage>> log1, log2;
};

TEST_F(LogCRDTTest, SyncBetweenTwoPeers) {
  dao1->NewMessage("alice", "Hello from Alice", 100);
  dao1->NewMessage("ivan", "Hello from Ivan1", 50);
  dao1->NewMessage("ivan", "Hello from Ivan2", 51);
  dao2->NewMessage("bob", "Hi from Bob", 101);
  dao2->NewMessage("bob", "Hi from Bob", 103);
  dao2->NewMessage("bob", "Hi from Bob", 105);

  auto log1_headers = log1->GetHeader();
  auto req_from_2_to_1 = log2->GetRequest(log1_headers);
  auto update_from_1_for_2 = log1->GetUpdate(req_from_2_to_1);

  auto res = log2->Merge(update_from_1_for_2);

  auto log2_headers = log2->GetHeader();
  auto req_from_1_to_2 = log1->GetRequest(log2_headers);
  auto update_from_2_for_1 = log2->GetUpdate(req_from_1_to_2);
  log1->Merge(update_from_2_for_1);

  auto final_log1_headers = log1->GetHeader();
  auto final_log2_headers = log2->GetHeader();

  ASSERT_EQ(final_log1_headers.size(), 3);
  ASSERT_EQ(final_log2_headers.size(), 3);
  ASSERT_EQ(final_log1_headers["alice"].size(), 1);
  ASSERT_EQ(final_log1_headers["bob"].size(), 1);
  EXPECT_EQ(final_log1_headers["alice"][0].len, 1);
  EXPECT_EQ(final_log1_headers["bob"][0].len, 3);
}
