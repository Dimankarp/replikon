#ifndef REPLIKON_TYPES_H
#define REPLIKON_TYPES_H

#include "constants.h"
#include "serial/serde.h"
#include <array>
#include <cstdint>
#include <string>
namespace replikon {

struct ChatMessage {
  std::string author;
  uint64_t lamport;
  uint64_t origin_ts;
  std::string body;
};

using Author = std::string;
using PubKey = std::array<std::byte, crypto_sign_PUBLICKEYBYTES>;
using PrivKey = std::array<std::byte, crypto_sign_SECRETKEYBYTES>;
using Signature = std::array<std::byte, crypto_sign_BYTES>;

struct SecurityUserInfo {
  Author author;
  PubKey pub_key;
  std::optional<PrivKey> priv_key;
};

using Hash = std::array<std::byte, HASH_LEN>;

} // namespace replikon

SERIALIZABLE(replikon::ChatMessage, 4);
SERIALIZABLE(replikon::SecurityUserInfo, 3);

#endif // REPLIKON_TYPES_H