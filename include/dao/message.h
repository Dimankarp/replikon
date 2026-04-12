#ifndef REPLIKON_DAO_MESSAGE_H
#define REPLIKON_DAO_MESSAGE_H

#include "sqlite.h"
#include "sqlite3.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace replikon::dao {

namespace internal {

static const std::string GET_ALL_BY_AUTHOR =
    "SELECT body, origin_ts, lamport FROM messages WHERE author = ?1";
static const std::string INSERT_MESSAGE =
    "INSERT OR IGNORE INTO messages (author, body, origin_ts, lamport) "
    "SELECT ?1, ?2, ?3, COALESCE(MAX(lamport), 0) + 1 "
    "FROM messages "
    "WHERE author = ?1";
} // namespace internal

class Messages {

public:
  Messages(std::shared_ptr<db::Sqlite> db) : _db{db} {}

  std::vector<std::string> GetAllMessage(std::string author) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db->Get(), internal::GET_ALL_BY_AUTHOR.c_str(),
                       internal::GET_ALL_BY_AUTHOR.size(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, author.c_str(), author.size(), SQLITE_STATIC);
    sqlite3_step(stmt);
    printf("The result is %s, %lld, %lld", sqlite3_column_text(stmt, 0),
           sqlite3_column_int64(stmt, 1), sqlite3_column_int64(stmt, 2));
    sqlite3_finalize(stmt);
    return {};
  }

  void InsertMessage(std::string author, std::string body, int64_t origin_ts) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db->Get(), internal::INSERT_MESSAGE.c_str(),
                       internal::INSERT_MESSAGE.size(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, author.c_str(), author.size(), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, body.c_str(), body.size(), SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, origin_ts);
    // sqlite3_bind_int64(stmt, 4, lamport);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }

private:
  // TODO: maybe do lazy prepared statements;

  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao

#endif // REPLIKON_DAO_MESSAGE_H