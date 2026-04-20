#ifndef REPLIKON_DAO_MESSAGE_H
#define REPLIKON_DAO_MESSAGE_H

#include "expected.h"
#include "sqlite.h"
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
  Messages(std::shared_ptr<db::Sqlite> db) : _db{db} {
    // empty
  }

  Expected<std::vector<ChatMessage>, db::SqliteError>
  GetAllMessages(const std::string &author,
                 const std::vector<Interval> &intervals) const {
    auto statement_res = _db->PrepareStatement(internal::CLEAR_INTERVALS);
    RETURN_IF_ERROR(statement_res);
    auto res = std::move(statement_res).value().Step();
    RETURN_IF_RESULT_ERROR(res, db::SqliteError{});

    statement_res = _db->PrepareStatement(internal::INSERT_INTO_INTERVALS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement insert_into_intervals =
        std::move(statement_res).value();

    for (auto [start, len] : intervals) {
      auto end = start + len - 1;
      res |= insert_into_intervals.BindInt64(1, start);
      res |= insert_into_intervals.BindInt64(2, end);
      res |= insert_into_intervals.Step();
      res |= insert_into_intervals.Reset();
      RETURN_IF_RESULT_ERROR(res, db::SqliteError{});
    }

    statement_res = _db->PrepareStatement(internal::GET_MSGS_BY_INTERVALS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_msgs_by_intervals =
        std::move(statement_res).value();
    res |= get_msgs_by_intervals.BindText(1, author);

    std::vector<ChatMessage> messages;
    while (get_msgs_by_intervals.Step() == db::SqliteResult::OK) {
      std::string author_val = get_msgs_by_intervals.ColumnText(0);
      std::string body_val = get_msgs_by_intervals.ColumnText(1);
      int64_t origin_ts = get_msgs_by_intervals.ColumnInt64(2);
      int64_t lamport = get_msgs_by_intervals.ColumnInt64(3);
      messages.push_back({author_val, static_cast<uint64_t>(lamport),
                          static_cast<uint64_t>(origin_ts), body_val});
    }
    return messages;
  }

  db::SqliteResult InsertMessage(const ChatMessage &message) {
    auto statement_res = _db->PrepareStatement(internal::INSERT_MESSAGE);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.BindText(1, message.author);
    res |= insert_message.BindText(2, message.body);
    res |= insert_message.BindInt64(3, message.lamport);
    res |= insert_message.BindInt64(4, message.origin_ts);
    res |= insert_message.Step();
    return res;
  }

  db::SqliteResult NewMessage(const std::string &author,
                              const std::string &body, uint64_t origin_ts) {
    auto statement_res = _db->PrepareStatement(internal::NEW_MESSAGE);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    res |= insert_message.BindText(1, author);
    res |= insert_message.BindText(2, body);
    res |= insert_message.BindInt64(3, origin_ts);
    res |= insert_message.Step();
    return res;
  }

  Expected<std::map<std::string, std::vector<Interval>>, db::SqliteError>
  GetHeaders() const {
    std::map<std::string, std::vector<Interval>> headers;
    auto statement_res = _db->PrepareStatement(internal::GET_HEADERS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();

    while (get_headers.Step() == db::SqliteResult::ROW) {
      std::string author_val = get_headers.ColumnText(0);
      int64_t lamport_start = get_headers.ColumnInt64(1);
      int64_t lamport_end = get_headers.ColumnInt64(2);
      Interval interval = {
          static_cast<uint64_t>(lamport_start),
          static_cast<uint64_t>(lamport_end - lamport_start + 1)};
      headers[author_val].push_back(interval);
    }
    return headers;
  }

private:
  std::shared_ptr<db::Sqlite> _db;
};

} // namespace replikon::dao

#endif // REPLIKON_DAO_MESSAGE_H