#ifndef REPLIKON_CRDT_CHAT_META_H
#define REPLIKON_CRDT_CHAT_META_H

#include "constants.h"
#include "dao/key_value.h"
#include "dao/security.h"
#include "logging.h"
#include "security/provider.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "traits/crdt.h"
#include "types.h"
#include "utils.h"
#include <algorithm>
#include <map>
#include <memory>
#include <optional>
namespace replikon::crdt {

template <typename SecurityProvider, typename Meta> class ChatMetaCrdt {

public:
  using Version = uint16_t;
  using Signature = typename SecurityProvider::Signature;

  using Header = Version;
  using Request = Version;
  using Update = std::tuple<Version, Meta, Signature>;

  ChatMetaCrdt(std::shared_ptr<dao::KeyValueDao> kv_dao,
               std::shared_ptr<SecurityProvider> provider, Author self,
               Author admin)
      : _kv_dao{kv_dao}, _provider{provider}, _self{std::move(self)},
        _admin{std::move(admin)} {}

public:
  Header getHeader() const { return getPersistedVersion(); }

  std::optional<Request> getRequest(Header header) const {
    auto local_version = getHeader();
    if (local_version >= header) {
      return std::nullopt;
    }
    return header;
  }

  Update getUpdate(Request request) const {
    auto version = getPersistedVersion();
    auto meta = getPersistedMeta();
    auto signature = getPersistedSignature();
    Update update = {version, std::move(meta), std::move(signature)};
    return update;
  }

  MergeStatus merge(Update update) {
    auto local_version = getHeader();
    auto &&[version, meta, sign] = update;
    if (version <= local_version) {
      return MergeStatus::SKIPPED;
    }

    auto to_check = std::tie(version, meta);
    if (!_provider->isValid(sign, _admin, to_check)) {
      return MergeStatus::SKIPPED;
    }

    // TODO: this better be a tx
    _kv_dao->insertIntegerValue(CHAT_META_VERSION_KEY, version);
    _kv_dao->insertBlobValue<Meta>(CHAT_META_KEY, meta);
    _kv_dao->insertBlobValue<Signature>(CHAT_META_SIGN_KEY, sign);

    return MergeStatus::MERGED;
  }

  void setMeta(const Meta &meta) {
    REPLIKON_ASSERT(_self == _admin);
    auto version = getPersistedVersion();
    version++;
    auto res = _kv_dao->insertIntegerValue(CHAT_META_VERSION_KEY, version);
    REPLIKON_ASSERT(res == db::SqliteResult::OK);

    auto to_sign = std::tie(version, meta);
    LOGD("To sign _self %s", _self.c_str());
    auto sign = _provider->sign(to_sign);

    res = _kv_dao->insertBlobValue<Meta>(CHAT_META_KEY, meta);
    REPLIKON_ASSERT(res == db::SqliteResult::OK);
    LOGD("Inserting signature");
    _kv_dao->insertBlobValue<Signature>(CHAT_META_SIGN_KEY, sign);
  }

private:
  Version getPersistedVersion() const {
    auto value = _kv_dao->getIntegerValue(CHAT_META_VERSION_KEY);
    REPLIKON_ASSERT(value.hasValue());
    auto opt = value.value();
    if (opt.has_value()) {
      return opt.value();
    }
    auto res = _kv_dao->insertIntegerValue(CHAT_META_VERSION_KEY, 0);
    REPLIKON_ASSERT(res == db::SqliteResult::OK);
    return 0;
  }

  Signature getPersistedSignature() const {
    auto value = _kv_dao->getBlobValue<Signature>(CHAT_META_SIGN_KEY);
    REPLIKON_ASSERT(value.hasValue() && value.value().has_value());
    Signature sign = value.value().value();
    return sign;
  }

  Meta getPersistedMeta() const {
    auto value = _kv_dao->getBlobValue<Meta>(CHAT_META_KEY);
    REPLIKON_ASSERT(value.hasValue() && value.value().has_value());
    Meta meta = value.value().value();
    return meta;
  }

private:
  std::shared_ptr<dao::KeyValueDao> _kv_dao;
  std::shared_ptr<SecurityProvider> _provider;
  Author _self;
  Author _admin;
}; // namespace replikon::crdt

static_assert(traits::IsCRDT<ChatMetaCrdt<sec::ED25519SecurityProvider,
                                          std::string>>::value,
              "ChatMetaCrdt must fulfill CRDT trait");

} // namespace replikon::crdt
#endif // REPLIKON_CRDT_CHAT_META_H