#ifndef REPLIKON_UTILS_H
#define REPLIKON_UTILS_H

#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <vector>

#ifdef NDEBUG
#define REPLIKON_ASSERT(condition) ((void)0)
#else
#define REPLIKON_ASSERT(condition)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: ( %s ), %s\nat %s:%d\n", #condition,  \
              __PRETTY_FUNCTION__, __FILE__, __LINE__);                        \
      std::abort();                                                            \
    }                                                                          \
  } while (0)
#endif

#define REPLIKON_UNREACHABLE REPLIKON_ASSERT(false);

#define RETURN_IF_RESULT_ERROR(result, error)                                  \
  do {                                                                         \
    if (result != replikon::db::SqliteResult::OK) {                            \
      return Unexpected{error};                                                \
    }                                                                          \
  } while (false)

namespace replikon {
enum class MergeStatus : uint8_t { MERGED, SKIPPED };

struct Interval {
  uint64_t start;
  uint64_t len;
};

inline bool IsMonotonicNonColliding(const std::vector<Interval> &a) {
  if (a.empty()) {
    return true;
  }

  for (int i = 0; i < a.size() - 1; i++) {
    if (a[i].len == 0)
      return false;
    auto cur_end = a[i].start + a[i].len - 1;
    auto next_start = a[i + 1].start;
    if (cur_end >= next_start)
      return false;
  }
  return true;
}

inline std::vector<Interval>
IntervalsDifference(std::vector<Interval> a_vec,
                    const std::vector<Interval> &b_vec) {
  REPLIKON_ASSERT(IsMonotonicNonColliding(a_vec));
  REPLIKON_ASSERT(IsMonotonicNonColliding(b_vec));

  auto a_it = a_vec.begin();
  auto a_end = a_vec.end();
  auto b_it = b_vec.begin();
  auto b_end = b_vec.end();

  std::vector<Interval> result;

  while (a_it != a_end) {
    if (b_it == b_end) {
      result.push_back(*a_it++);
      continue;
    }
    auto a_start = a_it->start;
    auto a_end = a_start + a_it->len - 1;
    auto b_start = b_it->start;
    auto b_end = b_start + b_it->len - 1;

    // [a] .. [b]
    if (a_end < b_start) {
      result.push_back(*a_it++);
      continue;
    }

    // [b] .. [a]
    if (a_start > b_end) {
      b_it++;
      continue;
    }
    if (a_start < b_start) {
      auto left_part = Interval{a_start, (b_start - a_start)};
      result.push_back(left_part);
      if (b_end < a_end) {
        // [ a  [b] ]
        *a_it = {b_end + 1, a_end - b_end};
        b_it++;
      } else {
        // [a [_]_b_]
        a_it++;
      }
    } else {
      if (b_end >= a_end) {
        // [ b  [a]]
        a_it++;
      } else {
        // [b [_]_a_]
        *a_it = {b_end + 1, a_end - b_end};
        b_it++;
      }
    }
  }

  return result;
}

} // namespace replikon

#endif // REPLIKON_UTILS_H