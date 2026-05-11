#ifndef REPLIKON_CRDT_USER_META_H
#define REPLIKON_CRDT_USER_META_H

#include "dao/message.h"
#include "dao/user_meta.h"
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

template <typename Meta, typename SecurityProvider> class UserMetaCrdt {
public:
  using Version = uint16_t;
  using Header = std::map<Author, Version>;
  using Request = Header;
  using Signature = typename SecurityProvider::Signature;
  using Update = std::map<Author, std::tuple<Version, Meta, Signature>>;

  UserMetaCrdt(std::shared_ptr<dao::UserMetaDao<Meta>> user_meta_dao,
               std::shared_ptr<SecurityProvider> security_provider, Author self)
      : _user_meta_dao(std::move(user_meta_dao)),
        _security_provider(std::move(security_provider)),
        _self{std::move(self)} {}

  Header getHeader() const {
    LOGI("Request header");
    return _user_meta_dao->getHeaders().value();
  }
  Request getRequest(Header header) const {
    auto local_header = _user_meta_dao->getHeaders().value();
    Request result;
    for (auto &&[k, v] : header) {
      if (!replikon::contains(local_header, k) || local_header[k] < v) {
        result[k] = v;
      }
    }
    return result;
  }
  Update getUpdate(Request request) const {
    Update update;
    for (auto &&[k, v] : request) {
      auto res = _user_meta_dao->getMeta(k);
      if (!res.hasValue() || !res.value().has_value()) {
        continue;
      }
      update[k] = std::move(res.value().value());
    }
    return update;
  }

  MergeStatus merge(Update update) {
    bool merged = false;
    for (auto &&[k, v] : update) {
      auto &[lamport, meta, sign] = v;
      auto is_valid =
          _security_provider->isValid(sign, k, std::tie(lamport, meta));
      if (is_valid) {
        auto res = _user_meta_dao->setMeta(k, lamport, meta, sign);
        if (res == db::SqliteResult::OK) {
           merged = true;
        }
      }
    }
    if (merged) {
      return MergeStatus::MERGED;
    } else {
      return MergeStatus::SKIPPED;
    }
  }

  void setMeta(const Meta &meta) {
    auto res = _user_meta_dao->getMeta(_self);
    REPLIKON_ASSERT(res.hasValue());

    Version lamport = 1;
    if (res.value().has_value()) {
      auto &&[cur_lamport, cur_meta, cur_sign] = res.value().value();
      lamport = cur_lamport + 1;
    }

    auto to_sign = std::tie(lamport, meta);
    auto sign = _security_provider->sign(to_sign);
    _user_meta_dao->setMeta(_self, lamport, meta, sign);
  }

private:
  std::shared_ptr<dao::UserMetaDao<Meta>> _user_meta_dao;
  std::shared_ptr<SecurityProvider> _security_provider;
  Author _self;
};

static_assert(traits::IsCRDT<UserMetaCrdt<std::string,
                                          sec::ED25519SecurityProvider>>::value,
              "UserMetaCrdt must fulfill CRDT trait");

} // namespace replikon::crdt

#endif // REPLIKON_CRDT_USER_META_H