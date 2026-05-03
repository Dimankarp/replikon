#pragma once

#include "instance.h"
#include "security/provider.h"
#include "utils.h"
#include <gtest/gtest.h>
#include <memory>

namespace replikon {
namespace test {

inline std::unique_ptr<Instance>
createTestInstance(const std::string &name,
                   const std::string &db_path = ":memory:") {
  auto keys = sec::ED25519SecurityProvider::generateKeys();
  SecurityUserInfo self = {name, keys.first, keys.second};
  return std::make_unique<Instance>(db_path, std::move(self));
}

class ReplikonTest : public ::testing::Test {
protected:
  void SetUp() override {
    replikon::setLogFile(stderr);
    auto sodium_init_res = sodium_init();
    REPLIKON_ASSERT(sodium_init_res >= 0);
  }
};

} // namespace test
} // namespace replikon
