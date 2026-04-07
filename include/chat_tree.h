#ifndef REPLIKON_CHAT_TREE_H
#define REPLIKON_CHAT_TREE_H

#include "traits/crdt.h"
#include "utils.h"
#include <cstdint>
#include <vector>
namespace replikon {

namespace internal {

struct Interval {
  int64_t start;
  int64_t len;
};

} // namespace internal

template <typename Value> class ChatTree {
public:
  using Header = std::vector<internal::Interval>;
  using Request = Header;
  using Update = Value;

  Header GetHeader() const { return {}; }
  Request GetRequest(Header) const { return {}; }
  Update GetUpdate(Request) const { return {}; }
  MergeStatus Merge(Update);
};

static_assert(traits::IsCRDT<ChatTree<int>>::value,
              "ChatTree must fulfill CRDT trait");

} // namespace replikon

#endif // REPLIKON_CHAT_TREE_H