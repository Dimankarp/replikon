#include "crdt/map.h"
#include "crdt/register.h"
#include "sqlite3.h"
#include <iostream>
using namespace replikon::crdt;

int main() {
  MapCRDT<std::string, Register<int>> crdt;
  crdt.LocalUpdate("me", 20);
  crdt.LocalUpdate("Her", 30);

  auto h = crdt.GetHeader();
  auto r = crdt.GetRequest(h);
  auto u = crdt.GetUpdate(r);

  sqlite3 *db;
  sqlite3_open("file", &db);
  
  // sqlite3_exec(db, const char *sql, int (*callback)(void *, int, char **, char **), void *, char **errmsg)

  sqlite3_close(db);
}