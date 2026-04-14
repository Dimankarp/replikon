#include "constants.h"
#include "crdt/map.h"
#include "crdt/register.h"
#include "dao/message.h"
#include "sqlite.h"
#include "time.h"
#include "utils.h"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
using namespace replikon::crdt;

int main() {
  MapCRDT<std::string, Register<int>> crdt;
  crdt.LocalUpdate("me", 20);
  crdt.LocalUpdate("Her", 30);

  auto h = crdt.GetHeader();
  auto r = crdt.GetRequest(h);
  auto u = crdt.GetUpdate(r);

  auto db = std::make_shared<replikon::db::Sqlite>();
  db->Connect(".db/file.sqlite");
  // std::cout << res << "\n";
  char *err;
  auto res = sqlite3_exec(db->Get(), replikon::INIT_MESSAGES.c_str(), nullptr,
                          nullptr, nullptr);
  std::cout << res << "\n";
  res = sqlite3_exec(db->Get(), replikon::INDEX_MESSAGES.c_str(), nullptr,
                     nullptr, &err);
  std::cout << res << "\n";
  res = sqlite3_exec(db->Get(), replikon::TEMP_SEARCH_INTERVALS.c_str(),
                     nullptr, nullptr, &err);
  std::cout << res << "\n";
  replikon::dao::Messages dao{db};

  dao.NewMessage("me", "1", 1);
  dao.NewMessage("me", "2", 1);
  dao.NewMessage("me", "3", 1);

  auto headers = dao.GetHeaders();
  for (auto &&[author, vec] : headers) {
    for (auto &i : vec) {
      printf("%s: %lld - %lld\n", author.c_str(), i.start, i.len);
    }
  }

  auto msgs = dao.GetAllMessages("me", {{65, 3}});
  for (auto &i : msgs) {
    printf("Message: %s\n", i.body.c_str());
  }
}