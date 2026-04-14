#ifndef REPLIKON_DAO_MESSAGE_H
#define REPLIKON_DAO_MESSAGE_H

#include "sqlite.h"
#include "sqlite3.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <map>
#include <memory>
#include <vector>
namespace replikon::dao {

namespace internal {

static const std::string INSERT_MESSAGE =
    "INSERT OR IGNORE INTO messages (author, body, origin_ts, lamport) "
    "SELECT ?1, ?2, ?3, ?4 "
    "FROM messages "
    "WHERE author = ?1";

static const std::string NEW_MESSAGE =
    "INSERT OR IGNORE INTO messages (author, body, origin_ts, lamport) "
    "SELECT ?1, ?2, ?3, COALESCE(MAX(lamport), 0) + 1 "
    "FROM messages "
    "WHERE author = ?1";

// Look for Gaps & Islands problem
static const std::string GET_HEADERS =
    "WITH islands AS ( "
    "SELECT author, lamport, "
    "lamport - row_number() OVER (PARTITION BY author ORDER BY "
    "lamport) as shift "
    "FROM messages   ) "
    "SELECT author, "
    "MIN(lamport) as interval_start, "
    "MAX(lamport) AS interval_end "
    "FROM islands GROUP BY shift, author";

static const std::string CLEAR_INTERVALS = "DELETE FROM search_intervals ";
static const std::string INSERT_INTO_INTERVALS =
    "INSERT INTO search_intervals VALUES (?1, ?2) ";
static const std::string GET_MSGS_BY_INTERVALS =
    "SELECT author, body, origin_ts, lamport FROM messages as m "
    "JOIN search_intervals as si "
    "ON m.lamport >= si.start AND m.lamport <= si.end "
    "WHERE author = ?1";

} // namespace internal

class Messages {

public:
  Messages(std::shared_ptr<db::Sqlite> db) : _db{db} {}

  std::vector<ChatMessage>
  GetAllMessages(const std::string &author,
                const std::vector<Interval> &intervals) const {
    sqlite3_exec(_db->Get(), internal::CLEAR_INTERVALS.c_str(), nullptr,
                 nullptr, nullptr);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db->Get(), internal::INSERT_INTO_INTERVALS.c_str(),
                       internal::INSERT_INTO_INTERVALS.size(), &stmt, nullptr);
    for (auto [start, len] : intervals) {
      auto end = start + len - 1;
      sqlite3_bind_int64(stmt, 1, start);
      sqlite3_bind_int64(stmt, 2, end);
      sqlite3_step(stmt);
      sqlite3_reset(stmt);
    }

    sqlite3_stmt *stmt_get_all;
    sqlite3_prepare_v2(_db->Get(), internal::GET_MSGS_BY_INTERVALS.c_str(),
                       internal::GET_MSGS_BY_INTERVALS.size(), &stmt_get_all,
                       nullptr);
    sqlite3_bind_text(stmt_get_all, 1, author.c_str(), author.size(),
                      SQLITE_STATIC);

    std::vector<ChatMessage> messages;
    while (sqlite3_step(stmt_get_all) == SQLITE_ROW) {
      const unsigned char *author_val = sqlite3_column_text(stmt_get_all, 0);
      const unsigned char *body_val = sqlite3_column_text(stmt_get_all, 1);
      int64_t origin_ts = sqlite3_column_int64(stmt_get_all, 2);
      int64_t lamport = sqlite3_column_int64(stmt_get_all, 3);
      messages.push_back({
          author_val ? std::string{(const char *)author_val} : std::string{},
          static_cast<uint64_t>(lamport),
          static_cast<uint64_t>(origin_ts),
          body_val ? std::string{(const char *)body_val} : std::string{},
      });
    }
    sqlite3_finalize(stmt);
    sqlite3_finalize(stmt_get_all);
    return messages;
  }

  void InsertMessage(const ChatMessage &message) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db->Get(), internal::INSERT_MESSAGE.c_str(),
                       internal::INSERT_MESSAGE.size(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, message.author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, message.body.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, message.lamport);
    sqlite3_bind_int64(stmt, 4, message.origin_ts);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }

  void NewMessage(const std::string &author, const std::string &body,
                  uint64_t origin_ts) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db->Get(), internal::NEW_MESSAGE.c_str(),
                       internal::NEW_MESSAGE.size(), &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, body.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, origin_ts);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }

  std::map<std::string, std::vector<Interval>> GetHeaders() const {
    std::map<std::string, std::vector<Interval>> headers;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db->Get(), internal::GET_HEADERS.c_str(),
                       internal::GET_HEADERS.size(), &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char *author_cstr = sqlite3_column_text(stmt, 0);
      printf("Received %s\n", author_cstr);
      int64_t lamport_start = sqlite3_column_int64(stmt, 1);
      int64_t lamport_end = sqlite3_column_int64(stmt, 2);
      Interval interval = {
          static_cast<uint64_t>(lamport_start),
          static_cast<uint64_t>(lamport_end - lamport_start + 1)};
      std::string author{(const char *)author_cstr};
      headers[author].push_back(interval);
    }
    sqlite3_finalize(stmt);
    return headers;
  }

private:
  // TODO: maybe do lazy prepared statements;

  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao

#endif // REPLIKON_DAO_MESSAGE_H