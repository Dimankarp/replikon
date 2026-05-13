#include "security/hashing.h"

namespace replikon::sec {

std::array<std::byte, HASH_LEN> blakE2bHash(serde::BufferView &buf) {
  std::array<std::byte, HASH_LEN> result;
  auto res = crypto_generichash_blake2b(
      reinterpret_cast<unsigned char *>(result.data()), HASH_LEN,
      reinterpret_cast<const unsigned char *>(buf.data()), buf.size(), nullptr,
      0);
  buf.consume(buf.size());
  REPLIKON_ASSERT(res == 0);
  return result;
}

} // namespace replikon::sec