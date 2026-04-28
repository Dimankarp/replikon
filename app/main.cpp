#include "constants.h"
#include "crdt/map.h"
#include "crdt/register.h"
#include "dao/message.h"
#include "logging.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "time.h"
#include "types.h"
#include "utils.h"
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sodium/core.h>
using namespace replikon::crdt;

int main() {
  auto log_file = fopen("app.log", "w");
  replikon::setLogFile(log_file);
  auto sodium_init_res = sodium_init();
  REPLIKON_ASSERT(sodium_init_res >= 0);

  replikon::ChatMessage msg{"me", 10, 10, "body"};
  replikon::serde::Buffer buf;
  replikon::serde::serialize(buf, msg);
  replikon::ChatMessage m =
      *replikon::serde::deserialize<replikon::ChatMessage>(buf);

  replikon::LOGI("%s, %d, %d, %s", msg.author.c_str(), msg.lamport,
                 msg.origin_ts, msg.body.c_str());

  MapCRDT<std::string, Register<int>> crdt;
  crdt.localUpdate("me", 20);
  crdt.localUpdate("Her", 30);

  auto h = crdt.getHeader();
  auto r = crdt.getRequest(h);
  auto u = crdt.getUpdate(r);
  replikon::LOGE("This is an error! %s", u.back().first.c_str());
  auto db = std::make_shared<replikon::db::Sqlite>();
  auto res = db->connect(".db/file.sqlite");
  std::cout << toString(res) << "\n";

  auto exp = db->prepareStatement(replikon::INIT_MESSAGES);
  auto statement = std::move(exp.value());
  res = statement.step();
  std::cout << toString(res) << "\n";

  exp = db->prepareStatement(replikon::INDEX_MESSAGES);
  statement = std::move(exp.value());
  res = statement.step();
  std::cout << toString(res) << "\n";

  exp = db->prepareStatement(replikon::TEMP_SEARCH_INTERVALS);
  statement = std::move(exp.value());
  res = statement.step();
  std::cout << toString(res) << "\n";

  exp = db->prepareStatement(replikon::INIT_SECURITY);
  statement = std::move(exp.value());
  res = statement.step();
  std::cout << toString(res) << "\n";

  replikon::dao::MessagesDao dao{db};

  dao.newMessage("me", "1", 1);
  dao.newMessage("me", "2", 1);
  dao.newMessage("me", "3", 1);

  auto headers = dao.getHeaders().value();
  for (auto &&[author, vec] : headers) {
    for (auto &i : vec) {
      printf("%s: %lld - %lld\n", author.c_str(), i.start, i.len);
    }
  }

  auto msgs = dao.getAllMessages("me", {{65, 3}}).value();
  for (auto &i : msgs) {
    printf("Message: %s\n", i.body.c_str());
  }
}