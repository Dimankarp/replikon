#include <gtest/gtest.h>
#include "test_utils.h"
#include "dao/security.h"

using namespace replikon;

using test::ReplikonTest;
TEST_F(ReplikonTest, SecurityDaoOperations) {
    auto instance = test::createTestInstance("alice");
    auto db = instance->db();
    
    dao::SecurityDao sec_dao(db);

    // Mock key addition and retrieval
    auto keys = sec::ED25519SecurityProvider::generateKeys();
    PubKey test_pub_key = keys.first;
    
    auto insert_res = sec_dao.insertPublicKey("bob", test_pub_key);
    ASSERT_EQ(insert_res, db::SqliteResult::OK);
    
    auto get_res = sec_dao.getUserInfo("bob");
    ASSERT_TRUE(get_res.hasValue());
    ASSERT_TRUE(get_res.value().has_value());
    
    // Compare buffers since std::array doesn't have == defined if it doesn't match size
    auto retrieved_key = get_res.value().value().pub_key;
    ASSERT_TRUE(std::equal(test_pub_key.begin(), test_pub_key.end(), retrieved_key.begin()));
}

TEST_F(ReplikonTest, SecurityDaoMissingAndAdmins) {
    auto instance = test::createTestInstance("alice");
    auto db = instance->db();
    dao::SecurityDao sec_dao(db);

    // Get missing user
    auto get_miss = sec_dao.getUserInfo("nobody");
    ASSERT_TRUE(get_miss.hasValue());
    ASSERT_FALSE(get_miss.value().has_value());

    // Generate keys
    auto admin_keys1 = sec::ED25519SecurityProvider::generateKeys();
    auto admin_keys2 = sec::ED25519SecurityProvider::generateKeys();

    // No admins yet
    auto admins_res = sec_dao.getAdminsUserInfo();
    ASSERT_TRUE(admins_res.hasValue());
    ASSERT_EQ(admins_res.value().size(), 0);

    // Insert admin 1 via admin api
    sec_dao.insertAdminPublicKey("admin1", admin_keys1.first);
    
    // Insert admin 2 via all keys mock but not actually admin flag
    sec_dao.insertAllKeys("not_admin", admin_keys2.first, admin_keys2.second);

    // List admins
    admins_res = sec_dao.getAdminsUserInfo();
    ASSERT_EQ(admins_res.value().size(), 1);
    ASSERT_EQ(admins_res.value()[0].author, "admin1");

    // List all public keys
    auto all_keys = sec_dao.getAllPublicKeys();
    ASSERT_TRUE(all_keys.hasValue());
    ASSERT_EQ(all_keys.value().size(), 2); // admin1 and not_admin
}
