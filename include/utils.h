#ifndef REPLIKON_UTILS_H
#define REPLIKON_UTILS_H

#include <cstdint>
namespace replikon {
enum class MergeStatus : uint8_t { MERGED, SKIPPED };

struct Interval {
  uint64_t start;
  uint64_t len;
};
} // namespace replikon

#endif // REPLIKON_UTILS_H