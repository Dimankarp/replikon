#include "gtest/gtest.h"
#include <gtest/gtest.h>

#include "constants.h"
#include "crdt/log.h"
#include "dao/message.h"
#include "security/no_sec_provider.h"
#include "sqlite.h"
#include "utils.h"

#include <iostream>
#include <memory>

using namespace replikon;

class LogCRDTTest : public ::testing::Test {
protected:
  void SetUp() override {
    db1 = std::make_shared<db::Sqlite>();
    db1->connect(":memory:");
    initDb(db1);

    db2 = std::make_shared<db::Sqlite>();
    db2->connect(":memory:");
    initDb(db2);

    dao1 = std::make_shared<dao::MessagesDao>(db1);
    dao2 = std::make_shared<dao::MessagesDao>(db2);

    log1 =
        std::make_unique<crdt::Log<ChatMessage, sec::NoSecurityProvider>>(dao1);
    log2 =
        std::make_unique<crdt::Log<ChatMessage, sec::NoSecurityProvider>>(dao2);
  }

  void initDb(std::shared_ptr<db::Sqlite> &db) {
    db->prepareStatement(INIT_MESSAGES).value().step();
    db->prepareStatement(INDEX_MESSAGES).value().step();
    db->prepareStatement(TEMP_SEARCH_INTERVALS).value().step();
  }

  std::shared_ptr<db::Sqlite> db1, db2;
  std::shared_ptr<dao::MessagesDao> dao1, dao2;
  std::unique_ptr<crdt::Log<ChatMessage, sec::NoSecurityProvider>> log1, log2;
};

TEST_F(LogCRDTTest, SyncBetweenTwoPeers) {
  dao1->newMessage("alice", "Hello from Alice", 100);
  dao1->newMessage("ivan", "Hello from Ivan1", 50);
  dao1->newMessage("ivan", "Hello from Ivan2", 51);
  dao2->newMessage("bob", "Hi from Bob", 101);
  dao2->newMessage("bob", "Hi from Bob", 103);
  dao2->newMessage("bob", "Hi from Bob", 105);

  auto log1_headers = log1->getHeader();
  auto req_from_2_to_1 = log2->getRequest(log1_headers);
  auto update_from_1_for_2 = log1->getUpdate(req_from_2_to_1);

  auto res = log2->merge(update_from_1_for_2);

  auto log2_headers = log2->getHeader();
  auto req_from_1_to_2 = log1->getRequest(log2_headers);
  auto update_from_2_for_1 = log2->getUpdate(req_from_1_to_2);
  log1->merge(update_from_2_for_1);

  auto final_log1_headers = log1->getHeader();
  auto final_log2_headers = log2->getHeader();

  ASSERT_EQ(final_log1_headers.size(), 3);
  ASSERT_EQ(final_log2_headers.size(), 3);
  ASSERT_EQ(final_log1_headers["alice"].size(), 1);
  ASSERT_EQ(final_log1_headers["bob"].size(), 1);
  EXPECT_EQ(final_log1_headers["alice"][0].len, 1);
  EXPECT_EQ(final_log1_headers["bob"][0].len, 3);
}
