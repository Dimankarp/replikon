#include "crdt/keys.h"
#include "../test_utils.h"
#include <gtest/gtest.h>

using namespace replikon;

class KeysCrdtPropertiesTest : public test::ReplikonTest {
protected:
  std::unique_ptr<Instance> createInstance(const std::string &name) {
    return test::createTestInstance(name);
  }

  std::unique_ptr<Instance> createInstance(const std::string &name,
                                           const Author &admin_author) {
    return test::createTestInstance(name, ":memory:", admin_author);
  }
};

TEST_F(KeysCrdtPropertiesTest, Commutativity) {
  auto alice = createInstance("alice");
  auto alice_admin = alice->self();
  alice->setAdmin(alice_admin.author, alice_admin.pub_key);

  auto bob = createInstance("bob");
  auto charlie = createInstance("charlie");
  auto target1 = createInstance("target1", alice_admin.author);
  auto target2 = createInstance("target2", alice_admin.author);
  target1->setAdmin(alice_admin.author, alice_admin.pub_key);
  target2->setAdmin(alice_admin.author, alice_admin.pub_key);

  auto &keys_alice = alice->keysCrdt();
  
  keys_alice.addKey(bob->self().author, bob->self().pub_key);
  auto req1 = target1->keysCrdt().getRequest(keys_alice.getHeader());
  auto update_bob = keys_alice.getUpdate(req1.value());

  keys_alice.addKey(charlie->self().author, charlie->self().pub_key);
  
  auto req2 = target2->keysCrdt().getRequest(keys_alice.getHeader());
  auto update_charlie = keys_alice.getUpdate(req2.value());

  auto &keys_t1 = target1->keysCrdt();
  auto &keys_t2 = target2->keysCrdt();

  keys_t1.merge(update_bob);
  keys_t1.merge(update_charlie);

  keys_t2.merge(update_charlie);
  keys_t2.merge(update_bob);

  EXPECT_EQ(keys_t1.getHeader(), keys_t2.getHeader());
}