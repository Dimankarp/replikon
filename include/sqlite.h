#ifndef REPLIKON_SQLITE_H
#define REPLIKON_SQLITE_H

#include "expected.h"
#include "serial/serde.h"
#include "sqlite3.h"
#include "utils.h"
#include <string>
#include <sys/types.h>
#include <variant>
namespace replikon::db {

enum class SqliteResult : char { OK, ERROR, ROW };
SqliteResult toEnum(int code);

inline SqliteResult operator|(const SqliteResult a, const SqliteResult b) {
  if (a == SqliteResult::OK && b == SqliteResult::OK) {
    return SqliteResult::OK;
  }
  return SqliteResult::ERROR;
}
inline SqliteResult &operator|=(SqliteResult &a, const SqliteResult b) {
  a = a | b;
  return a;
}

struct SqliteError {};

inline std::string toString(SqliteResult res) {
  switch (res) {
  case SqliteResult::OK:
    return "OK";
  case SqliteResult::ROW:
    return "ROW";
  case SqliteResult::ERROR:
    return "ERROR";
  }
}

class PreparedStatement {
public:
  PreparedStatement(sqlite3_stmt *stmt) : _stmt{stmt} {
    REPLIKON_ASSERT(_stmt != nullptr);
  }
  PreparedStatement(const PreparedStatement &s) = delete;
  PreparedStatement(PreparedStatement &&s) : _stmt{s._stmt} {
    s._stmt = nullptr;
  }
  PreparedStatement &operator=(const PreparedStatement &) = delete;
  PreparedStatement &operator=(PreparedStatement &&s) {
    auto temp = s._stmt;
    s._stmt = _stmt;
    _stmt = temp;
    return *this;
  };
  ~PreparedStatement() { sqlite3_finalize(_stmt); }

public:
  [[nodiscard]] SqliteResult reset();
  [[nodiscard]] SqliteResult step();
  [[nodiscard]] SqliteResult bindInt64(uint index, int64_t arg);
  [[nodiscard]] SqliteResult bindText(uint index, const std::string &arg);
  [[nodiscard]] SqliteResult bindBlob(uint index, const serde::BufferView view);
  std::string columnText(uint index);
  int64_t columnInt64(uint index);
  const void *columnBlob(uint index);
  const serde::Buffer columnBlobAsBuffer(uint index);
  size_t columnBytes(uint index);

private:
  // TODO: Add state machine
  sqlite3_stmt *_stmt;
};

class Sqlite {
public:
  Sqlite() {
    // empty
  }
  Sqlite(const Sqlite &) = delete;
  Sqlite &operator=(const Sqlite &) = delete;
  ~Sqlite() { sqlite3_close(_conn); }
  operator bool() { return _conn != nullptr; }

public:
  [[nodiscard]] SqliteResult connect(std::string filename);
  [[nodiscard]] Expected<PreparedStatement, SqliteError>
  prepareStatement(std::string statement);

private:
  sqlite3 *_conn = nullptr;
};

} // namespace replikon::db

#endif // REPLIKON_SQLITE_H
