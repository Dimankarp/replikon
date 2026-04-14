#ifndef REPLIKON_TYPES_H
#define REPLIKON_TYPES_H

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

#endif // REPLIKON_TYPES_H