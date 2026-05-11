#ifndef REPLIKON_DAO_USER_META_H
#define REPLIKON_DAO_USER_META_H

#include "expected.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <tuple>
#include <vector>
namespace replikon::dao {

namespace internal {

static const std::string SET_META =
    "INSERT OR REPLACE INTO user_meta (author, lamport, meta, signature) "
    "VALUES (?1, ?2, ?3, ?4) ";

static const std::string GET_META =
    "SELECT lamport, meta, signature FROM user_meta "
    "WHERE author = ?1 ";

static const std::string GET_META_HEADERS = "SELECT author, lamport FROM user_meta ";
} // namespace internal

template <typename Meta> class UserMetaDao {

public:
  UserMetaDao(std::shared_ptr<db::Sqlite> db) : _db{db} {
    // empty
  }

  Expected<std::optional<std::tuple<int64_t, Meta, Signature>>, db::SqliteError>
  getMeta(const Author &author) const {
    auto statement_res = _db->prepareStatement(internal::GET_META);
    RETURN_IF_ERROR(statement_res);
    auto statement = std::move(statement_res).value();
    statement.bindText(1, author);
    if (statement.step() == db::SqliteResult::ROW) {
      auto lamport = statement.columnInt64(0);
      auto meta_opt = statement.columnBlobAsSerializable<Meta>(1);
      REPLIKON_ASSERT(meta_opt.has_value());
      auto meta = meta_opt.value();
      auto sign_opt = statement.columnBlobAsSerializable<Signature>(2);
      REPLIKON_ASSERT(sign_opt.has_value());
      auto sign = sign_opt.value();
      
      std::optional<std::tuple<int64_t, Meta, Signature>> opt_tuple = std::make_tuple(lamport, meta, sign);
      return opt_tuple;
    } else {
      std::optional<std::tuple<int64_t, Meta, Signature>> opt_tuple = std::nullopt;
      return opt_tuple;
    }
  }

  db::SqliteResult setMeta(const Author &author, uint16_t lamport,
                           const Meta &meta, const Signature &signature) {
    auto statement_res = _db->prepareStatement(internal::SET_META);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement set_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= set_message.bindText(1, author);
    res |= set_message.bindInt64(2, lamport);
    res |= set_message.bindSerializableAsBlob(3, meta);
    res |= set_message.bindBlob(4, signature);
    res |= set_message.step();
    return res;
  }

  Expected<std::map<std::string, uint16_t>, db::SqliteError>
  getHeaders() const {
    std::map<std::string, uint16_t> headers;
    auto statement_res = _db->prepareStatement(internal::GET_META_HEADERS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();
    while (get_headers.step() == db::SqliteResult::ROW) {
      std::string author_val = get_headers.columnText(0);
      int64_t lamport = get_headers.columnInt64(1);
      headers[author_val] = lamport;
    }
    return headers;
  }

private:
  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao

#endif // REPLIKON_DAO_USER_META_H