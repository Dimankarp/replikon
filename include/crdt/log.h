#ifndef REPLIKON_CRDT_LOG_H
#define REPLIKON_CRDT_LOG_H

#include "dao/message.h"
#include "logging.h"
#include "traits/crdt.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace replikon {

template <typename Value> class Log {
public:
  using Header = std::map<std::string, std::vector<Interval>>;
  using Request = Header;
  using Update = std::map<std::string, std::vector<Value>>;

  Log(std::shared_ptr<dao::MessagesDao> messages_dao)
      : _messages_dao(std::move(messages_dao)) {}

  Header getHeader() const {
    LOGI("Request header");
    return _messages_dao->getHeaders().value();
  }
  Request getRequest(Header header) const {
    auto local_header = _messages_dao->getHeaders().value();
    Request result;
    for (auto &&[k, v] : header) {
      result[k] = intervalsDifference(std::move(v), local_header[k]);
    }
    return result;
  }
  Update getUpdate(Request request) const {
    Update update;
    for (auto &&[k, v] : request) {
      update[k] = _messages_dao->getAllMessages(k, v).value();
    }
    return update;
  }

  MergeStatus merge(Update update) {
    for (auto &&[k, v] : update) {
      for (auto &m : v) {
        auto res = _messages_dao->insertMessage(m);
      }
    }
    return MergeStatus::MERGED;
  }

private:
  std::shared_ptr<dao::MessagesDao> _messages_dao;
};

static_assert(traits::IsCRDT<Log<ChatMessage>>::value,
              "Log must fulfill CRDT trait");

} // namespace replikon

#endif // REPLIKON_CRDT_LOG_H