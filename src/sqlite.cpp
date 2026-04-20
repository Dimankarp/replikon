#ifndef REPLIKON_SQLITE_CPP
#define REPLIKON_SQLITE_CPP

#include "sqlite.h"
#include "expected.h"
#include "sqlite3.h"
#include <variant>

namespace replikon::db {

SqliteResult ToEnum(int code) {
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

[[nodiscard]] SqliteResult Sqlite::Connect(std::string filename) {
  if (_conn != nullptr) {
    return ToEnum(SQLITE_ERROR);
  }
  auto result = sqlite3_open(filename.c_str(), &_conn);
  if (result == SQLITE_OK) {
    return ToEnum(result);
  }
  sqlite3_close(_conn);
  return ToEnum(SQLITE_ERROR);
}

[[nodiscard]] Expected<PreparedStatement, SqliteError>
Sqlite::PrepareStatement(std::string statement) {
  sqlite3_stmt *stmt;
  auto result =
      sqlite3_prepare_v2(_conn, statement.c_str(), -1, &stmt, nullptr);
  if (ToEnum(result) == SqliteResult::OK) {
    return PreparedStatement{stmt};
  }
  return Unexpected<SqliteError>{{}};
}

[[nodiscard]] SqliteResult PreparedStatement::Reset() {
  return ToEnum(sqlite3_reset(_stmt));
}
[[nodiscard]] SqliteResult PreparedStatement::Step() {
  return ToEnum(sqlite3_step(_stmt));
}

[[nodiscard]] SqliteResult PreparedStatement::BindInt64(uint index,
                                                        int64_t arg) {
  return ToEnum(sqlite3_bind_int64(_stmt, index, arg));
}
[[nodiscard]] SqliteResult PreparedStatement::BindText(uint index,
                                                       const std::string &arg) {
  return ToEnum(
      sqlite3_bind_text(_stmt, index, arg.c_str(), arg.size(), SQLITE_STATIC));
}
std::string PreparedStatement::ColumnText(uint index) {
  auto res = (char *)sqlite3_column_text(_stmt, index);
  return res != nullptr ? std::string{res} : "";
}
int64_t PreparedStatement::ColumnInt64(uint index) {
  return sqlite3_column_int64(_stmt, index);
}

} // namespace replikon::db
#endif // REPLIKON_SQLITE_CPP