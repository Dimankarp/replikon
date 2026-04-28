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

static const std::string INSERT_KEY_FUlL =
    "INSERT OR REPLACE INTO security (author, public_key, private_key) "
    "VALUES (?1, ?2, ?3) ";
static const std::string INSERT_KEY_PUBLIC_ONLY =
    "INSERT OR REPLACE INTO security (author, public_key) "
    "VALUES (?1, ?2) ";

static const std::string GET_KEYS =
    "SELECT author, public_key, private_key FROM security "
    "WHERE author = ?1 ";
} // namespace internal

class SecurityDao {

public:
  SecurityDao(std::shared_ptr<db::Sqlite> db) : _db{db} {
    // empty
  }

  db::SqliteResult InsertPublicKey(const Author &author,
                                   const PubKey &pub_key) {
    auto statement_res =
        _db->PrepareStatement(internal::INSERT_KEY_PUBLIC_ONLY);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.BindText(1, author);
    auto view = serde::BufferView{pub_key.data(), pub_key.size()};
    res |= insert_message.BindBlob(2, view);
    res |= insert_message.Step();
    return res;
  }

  db::SqliteResult InsertAllKeys(const Author &author, const PubKey &pub_key,
                                 const PrivKey &priv_key) {
    auto statement_res = _db->PrepareStatement(internal::INSERT_KEY_FUlL);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.BindText(1, author);
    auto view = serde::BufferView{pub_key.data(), pub_key.size()};
    res |= insert_message.BindBlob(2, view);
    view = serde::BufferView{priv_key.data(), priv_key.size()};
    res |= insert_message.BindBlob(3, view);
    res |= insert_message.Step();
    return res;
  }

  Expected<std::optional<SecurityUserInfo>, db::SqliteError>
  getUserInfo(const Author &author) const {

    auto statement_res = _db->PrepareStatement(internal::GET_KEYS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();

    auto res = get_headers.BindText(1, author);
    RETURN_IF_RESULT_ERROR(res, db::SqliteError{});

    if (get_headers.Step() == db::SqliteResult::ROW) {
      Author author_val = get_headers.ColumnText(0);
      auto pub_key_val = get_headers.ColumnBlob(1);
      REPLIKON_ASSERT(pub_key_val);

      PubKey pub_key;
      std::memcpy(pub_key.data(), pub_key_val, pub_key.size());

      auto priv_key_val = get_headers.ColumnBlob(2);
      std::optional<PrivKey> priv_key = std::nullopt;
      if (priv_key_val) {
        PrivKey pk;
        std::memcpy(pk.data(), priv_key_val, pk.size());
        priv_key = pk;
      }
      return std::optional(SecurityUserInfo{author_val, pub_key, priv_key});
    } else {
      return Expected<std::optional<SecurityUserInfo>, db::SqliteError>{
          std::nullopt};
    }
    return Unexpected{db::SqliteError{}};
  }

private:
  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao

#endif // REPLIKON_DAO_SECURITY_H