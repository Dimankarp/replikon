#ifndef REPLIKON_DAO_MESSAGE_H
#define REPLIKON_DAO_MESSAGE_H

#include "expected.h"
#include "serial/serde.h"
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
    "INSERT OR IGNORE INTO messages (author, body, origin_ts, lamport, "
    "signature) "
    "VALUES (?1, ?2, ?3, ?4, ?5) ";

static const std::string NEW_MESSAGE =
    "INSERT OR IGNORE INTO messages (author, body, origin_ts, lamport, "
    "signature) "
    "SELECT ?1, ?2, ?3, COALESCE(MAX(lamport), 0) + 1, ?4 "
    "FROM messages "
    "WHERE author = ?1";

static const std::string GET_NEXT_LAMPORT =
    "SELECT COALESCE(MAX(lamport), 0) + 1 "
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
    "SELECT author, body, origin_ts, lamport, signature FROM messages as m "
    "JOIN search_intervals as si "
    "ON m.lamport >= si.start AND m.lamport <= si.end "
    "WHERE author = ?1";

} // namespace internal

class MessagesDao {

public:
  MessagesDao(std::shared_ptr<db::Sqlite> db) : _db{db} {
    // empty
  }

  Expected<std::vector<SignedChatMeesage>, db::SqliteError>
  getAllMessages(const std::string &author,
                 const std::vector<Interval> &intervals) const {
    auto statement_res = _db->prepareStatement(internal::CLEAR_INTERVALS);
    RETURN_IF_ERROR(statement_res);
    auto res = std::move(statement_res).value().step();
    RETURN_IF_RESULT_ERROR(res, db::SqliteError{});

    statement_res = _db->prepareStatement(internal::INSERT_INTO_INTERVALS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement insert_into_intervals =
        std::move(statement_res).value();

    for (auto [start, len] : intervals) {
      auto end = start + len - 1;
      res |= insert_into_intervals.bindInt64(1, start);
      res |= insert_into_intervals.bindInt64(2, end);
      res |= insert_into_intervals.step();
      res |= insert_into_intervals.reset();
      RETURN_IF_RESULT_ERROR(res, db::SqliteError{});
    }

    statement_res = _db->prepareStatement(internal::GET_MSGS_BY_INTERVALS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_msgs_by_intervals =
        std::move(statement_res).value();
    res |= get_msgs_by_intervals.bindText(1, author);

    std::vector<SignedChatMeesage> messages;
    while (get_msgs_by_intervals.step() == db::SqliteResult::ROW) {
      std::string author_val = get_msgs_by_intervals.columnText(0);
      std::string body_val = get_msgs_by_intervals.columnText(1);
      int64_t origin_ts = get_msgs_by_intervals.columnInt64(2);
      int64_t lamport = get_msgs_by_intervals.columnInt64(3);
      serde::Buffer sign = get_msgs_by_intervals.columnBlobAsBuffer(4);
      messages.push_back(
          SignedChatMeesage{{author_val, static_cast<uint64_t>(lamport),
                             static_cast<uint64_t>(origin_ts), body_val},
                            std::move(sign)});
    }
    return messages;
  }

  db::SqliteResult insertMessage(const SignedChatMeesage &signed_msg) {
    auto statement_res = _db->prepareStatement(internal::INSERT_MESSAGE);
    if (!statement_res.hasValue()) {
      return db::SqliteResult::ERROR;
    }
    db::PreparedStatement insert_message = std::move(statement_res).value();
    db::SqliteResult res;
    auto &&message = signed_msg.msg;
    res |= insert_message.bindText(1, message.author);
    res |= insert_message.bindText(2, message.body);
    res |= insert_message.bindInt64(3, message.origin_ts);
    res |= insert_message.bindInt64(4, message.lamport);
    res |= insert_message.bindBlob(5, signed_msg.sign);
    res |= insert_message.step();
    return res;
  }

  Expected<uint64_t, db::SqliteError>
  getNextLamport(const Author &author) const {
    auto statement_res = _db->prepareStatement(internal::GET_NEXT_LAMPORT);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_lamport = std::move(statement_res).value();

    auto res = get_lamport.bindText(1, author);

    if (get_lamport.step() == db::SqliteResult::ROW) {
      int64_t lamport = get_lamport.columnInt64(0);
      return lamport;
    }
    return Unexpected<db::SqliteError>{{}};
  }

  Expected<std::map<std::string, std::vector<Interval>>, db::SqliteError>
  getHeaders() const {
    std::map<std::string, std::vector<Interval>> headers;
    auto statement_res = _db->prepareStatement(internal::GET_HEADERS);
    RETURN_IF_ERROR(statement_res);
    db::PreparedStatement get_headers = std::move(statement_res).value();

    while (get_headers.step() == db::SqliteResult::ROW) {
      std::string author_val = get_headers.columnText(0);
      int64_t lamport_start = get_headers.columnInt64(1);
      int64_t lamport_end = get_headers.columnInt64(2);
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