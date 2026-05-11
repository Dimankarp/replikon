#include "crdt/chat_meta.h"
#include "test_utils.h"
#include <gtest/gtest.h>

using namespace replikon;

using test::ReplikonTest;
using ChatMeta = crdt::ChatMetaCrdt<sec::ED25519SecurityProvider, std::string>;

TEST_F(ReplikonTest, ChatMetaCrdtBasicSetAndGet) {
  auto admin_instance = test::createTestInstance("alice");
  auto admin = admin_instance->self().author;
  
  auto &meta_crdt = admin_instance->chatMetaCrdt();
  
  // Header should be 0 initially
  ASSERT_EQ(meta_crdt.getHeader(), 0);

  // Set meta
  std::string my_chat_name = "test_chat";
  meta_crdt.setMeta(my_chat_name);

  // Check version bumped
  ASSERT_EQ(meta_crdt.getHeader(), 1);

  auto update = meta_crdt.getUpdate(0);
  auto meta_val = std::get<1>(update);
  ASSERT_EQ(meta_val, my_chat_name);
}

TEST_F(ReplikonTest, ChatMetaCrdtMergeValid) {
  auto admin_instance = test::createTestInstance("alice");
  auto admin = admin_instance->self().author;

  auto bob_instance = test::createTestInstance("bob", ":memory:", admin);
  auto bob = bob_instance->self().author;

  // Add keys so they can verify signatures
  admin_instance->setAdmin(admin, admin_instance->self().pub_key);
  bob_instance->setAdmin(admin, admin_instance->self().pub_key);

  auto &admin_meta = admin_instance->chatMetaCrdt();
  auto &bob_meta = bob_instance->chatMetaCrdt();

  std::string my_chat_name = "secret_chat";
  admin_meta.setMeta(my_chat_name);

  auto admin_update = admin_meta.getUpdate(0);
  
  auto merge_res = bob_meta.merge(admin_update);
  ASSERT_EQ(merge_res, replikon::MergeStatus::MERGED);

  ASSERT_EQ(bob_meta.getHeader(), 1);
  auto bob_update = bob_meta.getUpdate(0);
  ASSERT_EQ(std::get<1>(bob_update), my_chat_name);
}

TEST_F(ReplikonTest, ChatMetaCrdtMergeInvalidSignature) {
  auto admin_instance = test::createTestInstance("alice");
  auto admin = admin_instance->self().author;

  auto bob_instance = test::createTestInstance("bob", ":memory:", admin);
  auto bob = bob_instance->self().author;

  bob_instance->setAdmin(admin, admin_instance->self().pub_key);

  auto &bob_meta = bob_instance->chatMetaCrdt();

  std::string malicious_chat_name = "hacked_chat";
  sec::ED25519SecurityProvider::Signature bad_sign{};
  // Fill with junk
  for (auto& b : bad_sign) { b = std::byte{0xFF}; }

  ChatMeta::Update bad_update = { 1, malicious_chat_name, bad_sign };

  auto merge_res = bob_meta.merge(bad_update);
  ASSERT_EQ(merge_res, replikon::MergeStatus::SKIPPED);
  
  // Bob should remain at version 0
  ASSERT_EQ(bob_meta.getHeader(), 0);
}

TEST_F(ReplikonTest, ChatMetaCrdtMergeOlderVersion) {
  auto admin_instance = test::createTestInstance("alice");
  auto admin = admin_instance->self().author;

  auto bob_instance = test::createTestInstance("bob", ":memory:", admin);
  auto bob = bob_instance->self().author;

  admin_instance->setAdmin(admin, admin_instance->self().pub_key);
  bob_instance->setAdmin(admin, admin_instance->self().pub_key);

  auto &admin_meta = admin_instance->chatMetaCrdt();
  auto &bob_meta = bob_instance->chatMetaCrdt();

  std::string chat_name1 = "chat1";
  admin_meta.setMeta(chat_name1);
  auto update1 = admin_meta.getUpdate(0);

  std::string chat_name2 = "chat2";
  admin_meta.setMeta(chat_name2);
  auto update2 = admin_meta.getUpdate(0);

  // Bob merges the latest update
  bob_meta.merge(update2);
  ASSERT_EQ(bob_meta.getHeader(), 2);

  // Bob tries to merge the older update
  auto merge_res = bob_meta.merge(update1);
  ASSERT_EQ(merge_res, replikon::MergeStatus::SKIPPED);
  ASSERT_EQ(bob_meta.getHeader(), 2); // Still at 2
}
