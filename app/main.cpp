#include "constants.h"
#include "crdt/map.h"
#include "crdt/register.h"
#include "dao/message.h"
#include "instance.h"
#include "logging.h"
#include "security/provider.h"
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

  const auto db_path = ".db/file.sqlite";
  auto keys = replikon::sec::ED25519SecurityProvider::generateKeys();
  replikon::SecurityUserInfo self = {"me", keys.first, keys.second};

  replikon::Instance instance{db_path, self};

  instance.messagesCrdt().addNewMessage(
      self.author, replikon::ChatMessage{self.author, 0, 1, "1"});
  instance.messagesCrdt().addNewMessage(
      self.author, replikon::ChatMessage{self.author, 0, 1, "2"});
  instance.messagesCrdt().addNewMessage(
      self.author, replikon::ChatMessage{self.author, 0, 1, "3"});

  auto headers = instance.messagesCrdt().getHeader();
  for (auto &&[author, vec] : headers) {
    for (auto &i : vec) {
      printf("%s: %lld - %lld\n", author.c_str(), i.start, i.len);
    }
  }

  auto msgs = instance.messagesCrdt().getUpdate(headers);
  for (auto &[a, ms] : msgs) {
    for (auto &&m : ms) {
      printf("Message from %s: %s\n", a.c_str(), m.second.body.c_str());
    }
  }
}