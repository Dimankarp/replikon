#include <gtest/gtest.h>
#include "test_utils.h"
#include "dao/key_value.h"

using namespace replikon;

using test::ReplikonTest;
TEST_F(ReplikonTest, KeyValueBasicOperations) {
    auto instance1 = test::createTestInstance("alice");
    
    auto db = instance1->db();
    dao::KeyValueDao kv_dao(db);

    // Test text insertion and retrieval
    auto insert_res = kv_dao.insertTextValue("key1", "value1");
    ASSERT_EQ(insert_res, db::SqliteResult::OK);

    auto text_res = kv_dao.getStringValue("key1");
    ASSERT_TRUE(text_res.hasValue());
    ASSERT_TRUE(text_res.value().has_value());
    ASSERT_EQ(text_res.value().value(), "value1");

    // Test integer insertion and retrieval
    insert_res = kv_dao.insertIntegerValue("key2", 42);
    ASSERT_EQ(insert_res, db::SqliteResult::OK);

    auto int_res = kv_dao.getIntegerValue("key2");
    ASSERT_TRUE(int_res.hasValue());
    ASSERT_TRUE(int_res.value().has_value());
    ASSERT_EQ(int_res.value().value(), 42);

    // Test missing key
    auto missing_res = kv_dao.getStringValue("missing");
    ASSERT_TRUE(missing_res.hasValue());
    ASSERT_FALSE(missing_res.value().has_value());
}

TEST_F(ReplikonTest, KeyValueBlobAndReplace) {
    auto instance = test::createTestInstance("alice");
    auto db = instance->db();
    dao::KeyValueDao kv_dao(db);

    // Initial replace string with another string
    kv_dao.insertTextValue("k1", "first");
    kv_dao.insertTextValue("k1", "second");
    auto str_res = kv_dao.getStringValue("k1");
    ASSERT_TRUE(str_res.value().has_value());
    ASSERT_EQ(str_res.value().value(), "second");

    // Replace text with integer
    kv_dao.insertIntegerValue("k1", 100);
    auto int_res = kv_dao.getIntegerValue("k1");
    ASSERT_TRUE(int_res.value().has_value());
    ASSERT_EQ(int_res.value().value(), 100);

    // Insert Blob
    std::string test_blob = "binary\0data";
    auto view = replikon::serde::BufferView{reinterpret_cast<const std::byte*>(test_blob.data()), test_blob.size()};
    auto res = kv_dao.insertBlobValue("blob_key", view);
    ASSERT_EQ(res, db::SqliteResult::OK);

    // Type mismatch checking depending on SQLite: get integer from text/blob. Sqlite does casting, but let's check it doesn't crash
    auto mismatch_res = kv_dao.getIntegerValue("blob_key");
    ASSERT_TRUE(mismatch_res.hasValue());
}
