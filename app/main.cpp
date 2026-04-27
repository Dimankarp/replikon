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
using namespace replikon::crdt;

int main() {
  auto log_file = fopen("app.log", "w");
  replikon::SetLogFile(log_file);

  replikon::ChatMessage msg{"me", 10, 10, "body"};
  replikon::serde::Buffer buf;
  replikon::serde::serialize(buf, msg);
  replikon::ChatMessage m =
      *replikon::serde::deserialize<replikon::ChatMessage>(buf);

  replikon::LOGI("%s, %d, %d, %s", msg.author.c_str(), msg.lamport,
                 msg.origin_ts, msg.body.c_str());

  MapCRDT<std::string, Register<int>> crdt;
  crdt.LocalUpdate("me", 20);
  crdt.LocalUpdate("Her", 30);

  auto h = crdt.GetHeader();
  auto r = crdt.GetRequest(h);
  auto u = crdt.GetUpdate(r);
  replikon::LOGE("This is an error! %s", u.back().first.c_str());
  auto db = std::make_shared<replikon::db::Sqlite>();
  auto res = db->Connect(".db/file.sqlite");
  std::cout << ToString(res) << "\n";

  auto exp = db->PrepareStatement(replikon::INIT_MESSAGES);
  auto statement = std::move(exp.value());
  res = statement.Step();
  std::cout << ToString(res) << "\n";

  exp = db->PrepareStatement(replikon::INDEX_MESSAGES);
  statement = std::move(exp.value());
  res = statement.Step();
  std::cout << ToString(res) << "\n";

  exp = db->PrepareStatement(replikon::TEMP_SEARCH_INTERVALS);
  statement = std::move(exp.value());
  res = statement.Step();
  std::cout << ToString(res) << "\n";

  replikon::dao::Messages dao{db};

  dao.NewMessage("me", "1", 1);
  dao.NewMessage("me", "2", 1);
  dao.NewMessage("me", "3", 1);

  auto headers = dao.GetHeaders().value();
  for (auto &&[author, vec] : headers) {
    for (auto &i : vec) {
      printf("%s: %lld - %lld\n", author.c_str(), i.start, i.len);
    }
  }

  auto msgs = dao.GetAllMessages("me", {{65, 3}}).value();
  for (auto &i : msgs) {
    printf("Message: %s\n", i.body.c_str());
  }
}