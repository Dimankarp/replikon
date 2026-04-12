#ifndef REPLIKON_CRDT_LOG_H
#define REPLIKON_CRDT_LOG_H

#include "dao/message.h"
#include "traits/crdt.h"
#include "utils.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace replikon {

namespace internal {

struct Interval {
  uint64_t start;
  uint64_t len;
};

} // namespace internal

template <typename Value, typename Time> class Log {
public:
  using Header = std::vector<internal::Interval>;
  using Request = Header;
  using Update = Value;

  Header GetHeader() const { return {}; }
  Request GetRequest(Header) const { return {}; }
  Update GetUpdate(Request) const { return {}; }
  MergeStatus Merge(Update);

private:
  std::shared_ptr<dao::Messages> _messages_dao;
};

static_assert(traits::IsCRDT<Log<int>>::value, "Log must fulfill CRDT trait");

} // namespace replikon

#endif // REPLIKON_CRDT_LOG_H