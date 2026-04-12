#ifndef REPLIKON_CRDT_LOG_H
#define REPLIKON_CRDT_LOG_H

#include "dao/message.h"
#include "traits/crdt.h"
#include "utils.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace replikon {

template <typename Value, typename Time> class Log {
public:
  using Header = std::map<std::string, std::vector<Interval>>;
  using Request = Header;
  using Update = Value;

  Header GetHeader() const { return _messages_dao->GetHeaders(); }
  Request GetRequest(Header header) const {}
  Update GetUpdate(Request) const { return {}; }
  MergeStatus Merge(Update);

private:
  std::shared_ptr<dao::Messages> _messages_dao;
};

static_assert(traits::IsCRDT<Log<int>>::value, "Log must fulfill CRDT trait");

} // namespace replikon

#endif // REPLIKON_CRDT_LOG_H