#include "dao/user_meta.h"
#include "test_utils.h"
#include <gtest/gtest.h>

using namespace replikon;

using test::ReplikonTest;
using UserMetaDao = dao::UserMetaDao<std::string>;

TEST_F(ReplikonTest, UserMetaDaoBasicSetAndGet) {
  auto db = std::make_shared<db::Sqlite>();
  auto res = db->connect(":memory:");
  ASSERT_EQ(res, db::SqliteResult::OK);

  auto statement = db->prepareStatement(replikon::INIT_USER_META).value();
  ASSERT_EQ(statement.step(), db::SqliteResult::OK);

  UserMetaDao dao(db);
  
  Author alice = "alice";
  std::string meta_data = "alice_meta";
  sec::ED25519SecurityProvider::Signature sign{};
  sign[0] = static_cast<std::byte>(0x01);

  auto set_res = dao.setMeta(alice, 1, meta_data, sign);
  ASSERT_EQ(set_res, db::SqliteResult::OK);

  auto get_res = dao.getMeta(alice);
  ASSERT_TRUE(get_res.hasValue());
  ASSERT_TRUE(get_res.value().has_value());

  auto [lamport, meta, retrieved_sign] = get_res.value().value();
  ASSERT_EQ(lamport, 1);
  ASSERT_EQ(meta, meta_data);
  ASSERT_EQ(retrieved_sign, sign);
}

TEST_F(ReplikonTest, UserMetaDaoGetHeaders) {
  auto db = std::make_shared<db::Sqlite>();
  auto res = db->connect(":memory:");
  ASSERT_EQ(res, db::SqliteResult::OK);

  auto statement = db->prepareStatement(replikon::INIT_USER_META).value();
  ASSERT_EQ(statement.step(), db::SqliteResult::OK);

  UserMetaDao dao(db);
  
  Author alice = "alice";
  Author bob = "bob";
  
  sec::ED25519SecurityProvider::Signature sign{};

  dao.setMeta(alice, 1, "alice_meta", sign);
  dao.setMeta(bob, 2, "bob_meta", sign);

  auto headers_res = dao.getHeaders();
  ASSERT_TRUE(headers_res.hasValue());

  auto headers = headers_res.value();
  ASSERT_EQ(headers.size(), 2);
  ASSERT_EQ(headers["alice"], 1);
  ASSERT_EQ(headers["bob"], 2);
}
