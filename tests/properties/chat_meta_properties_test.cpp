#include "crdt/chat_meta.h"
#include "../test_utils.h"
#include <gtest/gtest.h>

using namespace replikon;

class ChatMetaPropertiesTest : public test::ReplikonTest {
protected:
  std::unique_ptr<Instance> createInstance(const std::string &name) {
    return test::createTestInstance(name);
  }

  std::unique_ptr<Instance> createInstance(const std::string &name,
                                           const Author &admin_author) {
    return test::createTestInstance(name, ":memory:", admin_author);
  }
};

TEST_F(ChatMetaPropertiesTest, Idempotence) {
  auto admin = createInstance("alice");
  auto bob = createInstance("bob", admin->self().author);

  admin->setAdmin(admin->self().author, admin->self().pub_key);
  bob->setAdmin(admin->self().author, admin->self().pub_key);

  auto &admin_meta = admin->chatMetaCrdt();
  auto &bob_meta = bob->chatMetaCrdt();

  admin_meta.setMeta("Chat Name");
  auto update = admin_meta.getUpdate(0);

  bob_meta.merge(update);
  int header1 = bob_meta.getHeader();

  bob_meta.merge(update);
  int header2 = bob_meta.getHeader();

  EXPECT_EQ(header1, header2);
}