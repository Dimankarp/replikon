#ifndef REPLIKON_SQLITE_CPP
#define REPLIKON_SQLITE_CPP

#include "sqlite.h"

namespace replikon::db {

SqliteResult ToEnum(int code) {
  switch (code) {
  case SQLITE_OK:
    return SqliteResult::OK;
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

} // namespace replikon::db
#endif // REPLIKON_SQLITE_CPP