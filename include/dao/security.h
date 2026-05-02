#ifndef REPLIKON_DAO_SECURITY_H
#define REPLIKON_DAO_SECURITY_H

#include "expected.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "types.h"
#include "utils.h"
#include <memory>
#include <optional>
namespace replikon::dao {

namespace internal {

static const std::string INSERT_KEY_FULL =
    "INSERT INTO security (author, public_key, private_key) "
    "VALUES (?1, ?2, ?3) "
    "ON CONFLICT(author) DO UPDATE SET public_key = excluded.public_key, "
    "private_key = excluded.private_key";

static const std::string INSERT_KEY_PUBLIC_ONLY =
    "INSERT INTO security (author, public_key) "
    "VALUES (?1, ?2) "
    "ON CONFLICT(author) DO UPDATE SET public_key = excluded.public_key ";

static const std::string INSERT_ADMIN_KEY_PUBLIC_ONLY =
    "INSERT INTO security (author, public_key, is_admin) "
    "VALUES (?1, ?2, TRUE) "
    "ON CONFLICT(author) DO UPDATE SET public_key = excluded.public_key, "
    "is_admin = TRUE";

static const std::string GET_KEYS =
    "SELECT author, public_key, private_key, is_admin FROM security "
    "WHERE author = ?1 ";

static const std::string GET_ALL_PUBLIC_KEYS =
    "SELECT author, public_key FROM security ";

static const std::string GET_ADMIN_KEYS =
    "SELECT author, public_key, private_key, is_admin FROM security "
    "WHERE is_admin = TRUE";
} // namespace internal

class SecurityDao {

public:
  SecurityDao(std::shared_ptr<db::Sqlite> db) : _db{db} {
    // empty
  }

  db::SqliteResult insertPublicKey(const Author &author,
                                   const PubKey &pub_key) {
    auto statement_res =
        _db->prepareStatement(internal::INSERT_KEY_PUBLIC_ONLY);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.bindText(1, author);
    auto view = serde::BufferView{pub_key.data(), pub_key.size()};
    res |= insert_message.bindBlob(2, view);
    res |= insert_message.step();
    return res;
  }

  db::SqliteResult insertAdminPublicKey(const Author &author,
                                        const PubKey &pub_key) {
    auto statement_res =
        _db->prepareStatement(internal::INSERT_ADMIN_KEY_PUBLIC_ONLY);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.bindText(1, author);
    auto view = serde::BufferView{pub_key.data(), pub_key.size()};
    res |= insert_message.bindBlob(2, view);
    res |= insert_message.step();
    return res;
  }

  db::SqliteResult insertAllKeys(const Author &author, const PubKey &pub_key,
                                 const PrivKey &priv_key) {
    auto statement_res = _db->prepareStatement(internal::INSERT_KEY_FULL);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.bindText(1, author);
    auto view = serde::BufferView{pub_key.data(), pub_key.size()};
    res |= insert_message.bindBlob(2, view);
    view = serde::BufferView{priv_key.data(), priv_key.size()};
    res |= insert_message.bindBlob(3, view);
    res |= insert_message.step();
    return res;
  }

  Expected<std::optional<SecurityUserInfo>, db::SqliteError>
  getUserInfo(const Author &author) const {

    auto statement_res = _db->prepareStatement(internal::GET_KEYS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();

    auto res = get_headers.bindText(1, author);
    RETURN_IF_RESULT_ERROR(res, db::SqliteError{});

    if (get_headers.step() == db::SqliteResult::ROW) {
      Author author_val = get_headers.columnText(0);
      auto pub_key_val = get_headers.columnBlob(1);
      REPLIKON_ASSERT(pub_key_val);

      PubKey pub_key;
      std::memcpy(pub_key.data(), pub_key_val, pub_key.size());

      auto priv_key_val = get_headers.columnBlob(2);
      std::optional<PrivKey> priv_key = std::nullopt;
      if (priv_key_val) {
        PrivKey pk;
        std::memcpy(pk.data(), priv_key_val, pk.size());
        priv_key = pk;
      }
      bool is_admin = get_headers.columnInt64(3) == 1;
      return std::optional(
          SecurityUserInfo{author_val, pub_key, priv_key, is_admin});
    } else {
      return Expected<std::optional<SecurityUserInfo>, db::SqliteError>{
          std::nullopt};
    }
    return Unexpected{db::SqliteError{}};
  }

  Expected<std::vector<SecurityUserInfo>, db::SqliteError>
  getAdminsUserInfo() const {

    auto statement_res = _db->prepareStatement(internal::GET_ADMIN_KEYS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();

    std::vector<SecurityUserInfo> result;

    while (get_headers.step() == db::SqliteResult::ROW) {
      Author author_val = get_headers.columnText(0);
      auto pub_key_val = get_headers.columnBlob(1);
      REPLIKON_ASSERT(pub_key_val);

      PubKey pub_key;
      std::memcpy(pub_key.data(), pub_key_val, pub_key.size());

      auto priv_key_val = get_headers.columnBlob(2);
      std::optional<PrivKey> priv_key = std::nullopt;
      if (priv_key_val) {
        PrivKey pk;
        std::memcpy(pk.data(), priv_key_val, pk.size());
        priv_key = pk;
      }
      bool is_admin = get_headers.columnInt64(3) == 1;
      result.push_back(
          SecurityUserInfo{author_val, pub_key, priv_key, is_admin});
    }
    return result;
  }

  Expected<std::vector<std::tuple<Author, PubKey>>, db::SqliteError>
  getAllPublicKeys() const {

    auto statement_res = _db->prepareStatement(internal::GET_ALL_PUBLIC_KEYS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();

    std::vector<std::tuple<Author, PubKey>> result;

    while (get_headers.step() == db::SqliteResult::ROW) {
      Author author_val = get_headers.columnText(0);
      auto pub_key_val = get_headers.columnBlob(1);
      REPLIKON_ASSERT(pub_key_val);

      PubKey pub_key;
      std::memcpy(pub_key.data(), pub_key_val, pub_key.size());

      result.push_back({author_val, pub_key});
    }
    return result;
  }

private:
  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao

#endif // REPLIKON_DAO_SECURITY_H