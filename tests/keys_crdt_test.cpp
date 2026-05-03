#include "crdt/keys.h"
#include "test_utils.h"
#include <gtest/gtest.h>

using namespace replikon;

using test::ReplikonTest;
TEST_F(ReplikonTest, KeysCrdtBasicAddAndGet) {
  auto instance1 = test::createTestInstance("alice");
  auto &keys1 = instance1->keysCrdt();

  // Alice adds Bob
  auto keys = sec::ED25519SecurityProvider::generateKeys();
  PubKey test_pub_key = keys.first;
  keys1.addKey("bob", test_pub_key);

  auto bob_info = instance1->securityDao()->getUserInfo("bob");
  ASSERT_TRUE(bob_info.hasValue());
  ASSERT_TRUE(bob_info.value().has_value());

  auto retrieved_key = bob_info.value().value().pub_key;
  ASSERT_TRUE(std::equal(test_pub_key.begin(), test_pub_key.end(),
                         retrieved_key.begin()));
}

TEST_F(ReplikonTest, KeysCrdtMergeEmptyAndConflicts) {
  auto instance1 = test::createTestInstance("alice");
  auto instance2 = test::createTestInstance("bob", "test");

  auto admin = instance1->self();
  instance1->setAdmin(admin.author, admin.pub_key);
  instance2->setAdmin(admin.author, admin.pub_key);
  instance1->keysCrdt().addKey(instance2->self().author,
                               instance2->self().pub_key);
  auto &keys1 = instance1->keysCrdt();
  auto &keys2 = instance2->keysCrdt();
  LOGD("Start");
  // Bob merges empty update from missing remote
  auto req = keys2.getRequest(keys1.getHeader());
  auto update = keys1.getUpdate(req.value());
  auto merge_res = keys2.merge(std::move(update));
  ASSERT_EQ(merge_res, replikon::MergeStatus::MERGED);

  // Bob merges old version (Bob is at version 1 from first merge, now trying
  // version 0)
  std::vector<std::pair<Author, PubKey>> pub_keys;
  Signature bad_sign{};
  auto bad_update = std::make_tuple(admin.author, 0, pub_keys, bad_sign);
  LOGD("Before merge");
  auto skip_res = keys2.merge(bad_update);
  LOGD("After merge");
  ASSERT_EQ(skip_res, replikon::MergeStatus::SKIPPED);
}
