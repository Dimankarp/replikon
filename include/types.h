#ifndef REPLIKON_TYPES_H
#define REPLIKON_TYPES_H

#include "serial/serde.h"
#include <cstdint>
#include <string>
namespace replikon {

struct ChatMessage {
  std::string author;
  uint64_t lamport;
  uint64_t origin_ts;
  std::string body;
};

} // namespace replikon

SERIALIZABLE(replikon::ChatMessage, 4);

#endif // REPLIKON_TYPES_H