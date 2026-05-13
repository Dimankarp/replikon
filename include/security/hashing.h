#ifndef REPLIKON_SECURITY_HASHING_H
#define REPLIKON_SECURITY_HASHING_H

#include "constants.h"
#include "serial/serde.h"
#include "utils.h"
#include <array>
#include <cstddef>
#include <sodium/crypto_generichash_blake2b.h>
namespace replikon::sec {

std::array<std::byte, HASH_LEN> blakE2bHash(serde::BufferView &buf);

} // namespace replikon::sec

#endif // REPLIKON_SECURITY_HASHING_H