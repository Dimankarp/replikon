#ifndef REPLIKON_CRDT_LOG_H
#define REPLIKON_CRDT_LOG_H

#include "dao/message.h"
#include "traits/crdt.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace replikon {

template <typename Value, typename Time> class Log {
public:
  using Header = std::map<std::string, std::vector<Interval>>;
  using Request = Header;
  using Update = std::map<std::string, std::vector<Value>>;

  Header GetHeader() const { return _messages_dao->GetHeaders(); }
  Request GetRequest(Header header) const {
    auto local_header = _messages_dao->GetHeaders();
    Request result;
    for (auto &&[k, v] : header) {
      result[k] = IntervalsDifference(std::move(v), local_header[k]);
    }
    return result;
  }
  Update GetUpdate(Request request) const {
    Update update;
    for (auto &&[k, v] : request) {
      update[k] = _messages_dao->GetAllMessages(k, v);
    }
    return update;
  }

  MergeStatus Merge(Update update) { return MergeStatus::MERGED; }

private:
  std::shared_ptr<dao::Messages> _messages_dao;
};

static_assert(traits::IsCRDT<Log<ChatMessage>>::value,
              "Log must fulfill CRDT trait");

} // namespace replikon

#endif // REPLIKON_CRDT_LOG_H