#ifndef REPLIKON_SQLITE_H
#define REPLIKON_SQLITE_H

#include "sqlite3.h"
#include <string>
namespace replikon::db {

enum class SqliteResult : char { OK, ERROR };

SqliteResult ToEnum(int code);

class Sqlite {
public:
  Sqlite() {
    // empty
  }
  Sqlite(const Sqlite &) = delete;
  Sqlite &operator=(const Sqlite &) = delete;
  ~Sqlite() { sqlite3_close(_conn); }

public:
  [[nodiscard]] SqliteResult Connect(std::string filename);
  sqlite3 *Get() { return _conn; }

private:
  sqlite3 *_conn = nullptr;
};
} // namespace replikon::db

#endif // REPLIKON_SQLITE_H
