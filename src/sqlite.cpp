#ifndef REPLIKON_SQLITE_CPP
#define REPLIKON_SQLITE_CPP

#include "sqlite.h"
#include "expected.h"
#include "serial/serde.h"
#include "sqlite3.h"
#include <variant>

namespace replikon::db {

SqliteResult toEnum(int code) {
  switch (code) {
  case SQLITE_OK:
  case SQLITE_DONE:
    return SqliteResult::OK;
  case SQLITE_ROW:
    return SqliteResult::ROW;
  default:
    return SqliteResult::ERROR;
  }
}

[[nodiscard]] SqliteResult Sqlite::connect(std::string filename) {
  if (_conn != nullptr) {
    return toEnum(SQLITE_ERROR);
  }
  auto result = sqlite3_open(filename.c_str(), &_conn);
  if (result == SQLITE_OK) {
    return toEnum(result);
  }
  sqlite3_close(_conn);
  return toEnum(SQLITE_ERROR);
}

[[nodiscard]] Expected<PreparedStatement, SqliteError>
Sqlite::prepareStatement(std::string statement) {
  sqlite3_stmt *stmt;
  auto result =
      sqlite3_prepare_v2(_conn, statement.c_str(), -1, &stmt, nullptr);
  if (toEnum(result) == SqliteResult::OK) {
    return PreparedStatement{stmt};
  }
  return Unexpected<SqliteError>{{}};
}

[[nodiscard]] SqliteResult PreparedStatement::reset() {
  return toEnum(sqlite3_reset(_stmt));
}
[[nodiscard]] SqliteResult PreparedStatement::step() {
  return toEnum(sqlite3_step(_stmt));
}

[[nodiscard]] SqliteResult PreparedStatement::bindInt64(uint index,
                                                        int64_t arg) {
  return toEnum(sqlite3_bind_int64(_stmt, index, arg));
}
[[nodiscard]] SqliteResult PreparedStatement::bindText(uint index,
                                                       const std::string &arg) {
  return toEnum(
      sqlite3_bind_text(_stmt, index, arg.c_str(), arg.size(), SQLITE_STATIC));
}
[[nodiscard]] SqliteResult
PreparedStatement::bindBlob(uint index, const serde::BufferView view) {
  return toEnum(
      sqlite3_bind_blob(_stmt, index, view.data(), view.size(), SQLITE_STATIC));
}
std::string PreparedStatement::columnText(uint index) {
  auto res = (char *)sqlite3_column_text(_stmt, index);
  return res != nullptr ? std::string{res} : "";
}
int64_t PreparedStatement::columnInt64(uint index) {
  return sqlite3_column_int64(_stmt, index);
}

const void *PreparedStatement::columnBlob(uint index) {
  return sqlite3_column_blob(_stmt, index);
}

size_t PreparedStatement::columnBytes(uint index) {
  return sqlite3_column_bytes(_stmt, index);
}

} // namespace replikon::db
#endif // REPLIKON_SQLITE_CPP