#ifndef REPLIKON_DAO_KEY_VALUE_H
#define REPLIKON_DAO_KEY_VALUE_H

#include "expected.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "utils.h"
#include <memory>
#include <optional>
namespace replikon::dao {
namespace internal {

static const std::string INSERT_VALUE =
    "INSERT OR REPLACE INTO key_value (key, value) "
    "VALUES (?1, ?2) ";
static const std::string GET_VALUE = "SELECT value FROM key_value "
                                     "WHERE key = ?1 ";

} // namespace internal

class KeyValueDao {

public:
  KeyValueDao(std::shared_ptr<db::Sqlite> db) : _db{db} {}

  db::SqliteResult insertTextValue(const std::string &key,
                                   const std::string &value) {
    auto statement_res = _db->prepareStatement(internal::INSERT_VALUE);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    auto statement = std::move(statement_res).value();
    auto res = statement.bindText(1, key);
    res |= statement.bindText(2, value);
    res |= statement.step();
    return res;
  }

  db::SqliteResult insertIntegerValue(const std::string &key, int64_t value) {
    auto statement_res = _db->prepareStatement(internal::INSERT_VALUE);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    auto statement = std::move(statement_res).value();
    auto res = statement.bindText(1, key);
    res |= statement.bindInt64(2, value);
    res |= statement.step();
    return res;
  }

  db::SqliteResult insertBlobValue(const std::string &key,
                                   const serde::BufferView view) {
    auto statement_res = _db->prepareStatement(internal::INSERT_VALUE);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    auto statement = std::move(statement_res).value();
    auto res = statement.bindText(1, key);
    res |= statement.bindBlob(2, view);
    res |= statement.step();
    return res;
  }

  Expected<std::optional<std::string>, db::SqliteError>
  getStringValue(const std::string &key) {
    auto statement_res = _db->prepareStatement(internal::GET_VALUE);
    RETURN_IF_ERROR(statement_res);
    auto statement = std::move(statement_res).value();
    auto res = statement.bindText(1, key);

    res = statement.step();
    if (res == db::SqliteResult::ROW) {
      return std::make_optional(statement.columnText(0));
    }

    if (res == db::SqliteResult::OK) {
      return Expected<std::optional<std::string>, db::SqliteError>{
          std::nullopt};
    }
    return Unexpected{db::SqliteError{}};
  }

  Expected<std::optional<int64_t>, db::SqliteError>
  getIntegerValue(const std::string &key) {
    auto statement_res = _db->prepareStatement(internal::GET_VALUE);
    RETURN_IF_ERROR(statement_res);
    auto statement = std::move(statement_res).value();
    auto res = statement.bindText(1, key);

    res = statement.step();
    if (res == db::SqliteResult::ROW) {
      return std::make_optional(statement.columnInt64(0));
    }

    if (res == db::SqliteResult::OK) {
      return Expected<std::optional<int64_t>, db::SqliteError>{std::nullopt};
    }
    return Unexpected{db::SqliteError{}};
  }

  Expected<std::optional<serde::Buffer>, db::SqliteError>
  getBlobValue(const std::string &key) {
    auto statement_res = _db->prepareStatement(internal::GET_VALUE);
    RETURN_IF_ERROR(statement_res);
    auto statement = std::move(statement_res).value();
    auto res = statement.bindText(1, key);

    res = statement.step();
    if (res == db::SqliteResult::ROW) {
      auto ptr = statement.columnBlob(0);
      auto size = statement.columnBytes(0);
      serde::Buffer buf;
      if (ptr && size > 0) {
        buf.assign(reinterpret_cast<const std::byte *>(ptr),
                   reinterpret_cast<const std::byte *>(ptr) + size);
      }
      return std::make_optional(std::move(buf));
    }

    if (res == db::SqliteResult::OK) {
      return Expected<std::optional<serde::Buffer>, db::SqliteError>{
          std::nullopt};
    }
    return Unexpected{db::SqliteError{}};
  }

private:
  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao
#endif // REPLIKON_DAO_KEY_VALUE_H