#include "constants.h"
#include "crdt/map.h"
#include "crdt/register.h"
#include "dao/message.h"
#include "sqlite.h"
#include "time.h"
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
  replikon::dao::Messages dao{db};
  std::cout << res << "\n";
  for (int i = 0; i < 10; i++)
    dao.InsertMessage(
        "me", "This is my body!!!",
        std::chrono::system_clock::now().time_since_epoch().count());
  dao.GetAllMessage("me");
  dao.GetAllMessage("Her");
}