#ifndef REPLIKON_CHAT_TREE_H
#define REPLIKON_CHAT_TREE_H

#include <cstddef>
#include <vector>
namespace replikon {

namespace internal {

struct Interval {
  size_t start;
  size_t len;
}

using DeltaTree

} // namespace internal

template <typename Value> class ChatTree {

  using Advertisement = std::vector<internal::Interval>;
  using Package = ;

public:
  Advertisement Advertisement();
  Package Package();
  void Merge(Package);
};

} // namespace replikon

#endif // REPLIKON_CHAT_TREE_H