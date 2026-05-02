#ifndef REPLIKON_CRDT_KEYS_H
#define REPLIKON_CRDT_KEYS_H

#include "constants.h"
#include "dao/key_value.h"
#include "dao/security.h"
#include "security/provider.h"
#include "serial/serde.h"
#include "sqlite.h"
#include "traits/crdt.h"
#include "types.h"
#include "utils.h"
#include <map>
#include <memory>
#include <optional>
namespace replikon::crdt {

template <typename SecurityProvider> class KeysCrdt {

public:
  using Version = uint16_t;
  using PubKeyMap = std::map<Author, PubKey>;
  using Signature = typename SecurityProvider::Signature;

  using Header = std::tuple<Author, Version>;
  using Request = std::tuple<Author, Version>;
  using Update = std::tuple<Author, Version, PubKeyMap, Signature>;

  KeysCrdt(std::shared_ptr<dao::KeyValueDao> kv_dao,
           std::shared_ptr<dao::SecurityDao> sec_dao,
           std::shared_ptr<SecurityProvider> provider)
      : _kv_dao{kv_dao}, _sec_dao{sec_dao}, _provider{provider} {}

public:
  Header getHeader() const {
    return std::make_tuple(getAdmin(), getPersistedVersion());
  }

  std::optional<Request> getRequest(Header header) const {
    auto &&[local_author, local_version] = getHeader();
    auto &[author, version] = header;
    REPLIKON_ASSERT(local_author == author);

    if (local_version >= version) {
      return std::nullopt;
    }

    return header;
  }

  Update getUpdate(Request request) const {
    auto res = _sec_dao->getAllPublicKeys();
    REPLIKON_ASSERT(res.hasValue());
    auto pairs = res.value();

    auto admin = getAdmin();
    auto version = getPersistedVersion();
    REPLIKON_ASSERT(std::get<Author>(request) == admin);

    auto sign = getPersistedSignature();

    PubKeyMap pub_keys{};
    for (auto &&[author, key] : pairs) {
      pub_keys[author] = key;
    }

    Update update = {admin, version, std::move(pub_keys), sign};
    return update;
  }

  MergeStatus merge(Update update) {
    auto &&[local_author, local_version] = getHeader();
    auto &&[author, version, pub_keys, sign] = update;

    REPLIKON_ASSERT(local_author == author);
    if (local_version >= version) {
      return MergeStatus::SKIPPED;
    }

    auto to_check = std::tie(author, version, pub_keys);
    if (!_provider->isValid(sign, to_check)) {
      return MergeStatus::SKIPPED;
    }

    _kv_dao->insertIntegerValue(KEYS_CRDT_VERSION_KEY, version);
    _kv_dao->insertBlobValue(KEYS_CRDT_SIGN_KEY, serde::BufferView{sign});

    for (auto &&[author, pub_key] : pub_keys) {
      _sec_dao->insertPublicKey(author, pub_key);
    }

    return MergeStatus::MERGED;
  }

private:
  Version getPersistedVersion() const {
    auto value = _kv_dao->getIntegerValue(KEYS_CRDT_VERSION_KEY);
    REPLIKON_ASSERT(value.hasValue());
    auto opt = value.value();
    if (opt.has_value()) {
      return opt.value();
    }
    auto res = _kv_dao->insertIntegerValue(KEYS_CRDT_VERSION_KEY, 0);
    REPLIKON_ASSERT(res == db::SqliteResult::OK);
    return 0;
  }

  Author getAdmin() const {
    auto value = _sec_dao->getAdminsUserInfo();
    REPLIKON_ASSERT(value.hasValue());
    REPLIKON_ASSERT(value.value().size() == 1);
    return value.value().back().author;
  }

  Signature getPersistedSignature() const {
    auto value = _kv_dao->getBlobValue(KEYS_CRDT_SIGN_KEY);
    REPLIKON_ASSERT(value.hasValue() && value.value().has_value());
    auto blob = value.value().value();
    Signature sign;
    REPLIKON_ASSERT(blob.size() == sign.size());
    sign.assign(blob.begin(), blob.end());
    return sign;
  }

private:
  std::shared_ptr<dao::KeyValueDao> _kv_dao;
  std::shared_ptr<dao::SecurityDao> _sec_dao;
  std::shared_ptr<SecurityProvider> _provider;
}; // namespace replikon::crdt

static_assert(traits::IsCRDT<KeysCrdt<sec::ED25519SecurityProvider>>::value,
              "KeysCrdt must fulfill CRDT trait");

} // namespace replikon::crdt
#endif // REPLIKON_CRDT_KEYS_H