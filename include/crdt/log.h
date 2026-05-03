#ifndef REPLIKON_CRDT_LOG_H
#define REPLIKON_CRDT_LOG_H

#include "dao/message.h"
#include "logging.h"
#include "security/provider.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "traits/crdt.h"
#include "types.h"
#include "utils.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace replikon::crdt {

template <typename Value, typename SecurityProvider> class Log {
public:
  using Header = std::map<std::string, std::vector<Interval>>;
  using Request = Header;
  using Signature = typename SecurityProvider::Signature;
  using SignedValue = std::pair<Signature, Value>;
  using Update = std::map<std::string, std::vector<SignedValue>>;

  Log(std::shared_ptr<dao::MessagesDao> messages_dao,
      std::shared_ptr<SecurityProvider> security_provider)
      : _messages_dao(std::move(messages_dao)),
        _security_provider(std::move(security_provider)) {}

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
      auto msgs_res = _messages_dao->getAllMessages(k, v);
      if (!msgs_res.hasValue()) continue;
      auto msgs = msgs_res.value();
      std::vector<SignedValue> converted;
      for (auto &&m : msgs) {
        Signature s;
        std::copy(m.sign.begin(), m.sign.end(), s.begin());
        converted.emplace_back(std::move(s), std::move(m.msg));
      }
      update[k] = std::move(converted);
    }
    return update;
  }

  MergeStatus merge(Update update) {
    bool merged = false;
    for (auto &&[k, v] : update) {
      for (auto &[sig, m] : v) {
        auto is_valid = _security_provider->isValid(sig, k, m);
        if (is_valid) {
          serde::Buffer sign_buf(sig.begin(), sig.end());
          auto res = _messages_dao->insertMessage(SignedChatMeesage{std::move(m), std::move(sign_buf)});
          merged |= res == db::SqliteResult::OK;
        }
      }
    }
    if (merged) {
      return MergeStatus::MERGED;
    } else {
      return MergeStatus::SKIPPED;
    }
  }

  void addNewMessage(const Author &author, Value v) {
    auto lamport_res = _messages_dao->getNextLamport(author);
    REPLIKON_ASSERT(lamport_res.hasValue());
    auto lamport = lamport_res.value();
    v.lamport = lamport;
    auto sign = _security_provider->sign(v);
    serde::Buffer sign_buf(sign.begin(), sign.end());
    _messages_dao->insertMessage(SignedChatMeesage{std::move(v), std::move(sign_buf)});
  }

private:
  std::shared_ptr<dao::MessagesDao> _messages_dao;
  std::shared_ptr<SecurityProvider> _security_provider;
};

static_assert(
    traits::IsCRDT<Log<ChatMessage, sec::ED25519SecurityProvider>>::value,
    "Log must fulfill CRDT trait");

} // namespace replikon::crdt

#endif // REPLIKON_CRDT_LOG_H