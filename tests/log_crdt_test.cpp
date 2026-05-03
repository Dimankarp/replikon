#include "gtest/gtest.h"
#include <gtest/gtest.h>

#include "constants.h"
#include "crdt/log.h"
#include "dao/message.h"
#include "instance.h"
#include "security/provider.h"
#include "sqlite.h"
#include "utils.h"

#include <iostream>
#include "test_utils.h"

using namespace replikon;

class LogCRDTTest : public test::ReplikonTest {
protected:
  std::unique_ptr<Instance> createInstance(const std::string &name) {
    return test::createTestInstance(name);
  }
};

TEST_F(LogCRDTTest, SyncBetweenTwoPeers) {
  auto instance1 = createInstance("alice");
  auto instance2 = createInstance("bob");

  auto &log1 = instance1->messagesCrdt();
  auto &log2 = instance2->messagesCrdt();

  log1.addNewMessage("alice", {"alice", 0, 100, "Hello from Alice"});
  log1.addNewMessage("alice", {"alice", 0, 50, "Hello from Ivan1"});
  log1.addNewMessage("alice", {"alice", 0, 51, "Hello from Ivan2"});

  log2.addNewMessage("bob", {"bob", 0, 101, "Hi from Bob"});
  log2.addNewMessage("bob", {"bob", 0, 103, "Hi from Bob"});
  log2.addNewMessage("bob", {"bob", 0, 105, "Hi from Bob"});

  auto admin = instance1->self();
  instance1->setAdmin(admin.author, admin.pub_key);
  instance2->setAdmin(admin.author, admin.pub_key);

  auto instance2_user = instance2->self();
  instance1->keysCrdt().addKey(instance2_user.author, instance2_user.pub_key);

  auto header1 = instance1->keysCrdt().getHeader();
  auto request = instance2->keysCrdt().getRequest(header1);
  auto update1For2 = instance1->keysCrdt().getUpdate(request.value());
  instance2->keysCrdt().merge(std::move(update1For2));


  // Sync Logs
  auto log1_headers = log1.getHeader();
  auto req_from_2_to_1 = log2.getRequest(log1_headers);
  auto update_from_1_for_2 = log1.getUpdate(req_from_2_to_1);

  auto res = log2.merge(update_from_1_for_2);

  auto log2_headers = log2.getHeader();
  auto req_from_1_to_2 = log1.getRequest(log2_headers);
  auto update_from_2_for_1 = log2.getUpdate(req_from_1_to_2);
  log1.merge(update_from_2_for_1);

  auto final_log1_headers = log1.getHeader();
  auto final_log2_headers = log2.getHeader();

  ASSERT_EQ(final_log1_headers.size(), 2);
  ASSERT_EQ(final_log2_headers.size(), 2);
  ASSERT_EQ(final_log1_headers["alice"].size(), 1);
  ASSERT_EQ(final_log1_headers["bob"].size(), 1);
  EXPECT_EQ(final_log1_headers["alice"][0].len, 3);
  EXPECT_EQ(final_log1_headers["bob"][0].len, 3);
}
