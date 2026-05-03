#ifndef REPLIKON_SECURITY_PROVIDER_H
#define REPLIKON_SECURITY_PROVIDER_H

#include "dao/security.h"
#include "logging.h"
#include "serial/serde.h"
#include "sodium.h"
#include "types.h"
#include "utils.h"
#include <memory>
#include <sodium/crypto_sign.h>
#include <sodium/crypto_sign_ed25519.h>
#include <string>
namespace replikon::sec {
class ED25519SecurityProvider {
public:
  using Signature = replikon::Signature;

  ED25519SecurityProvider(std::shared_ptr<dao::SecurityDao> dao, Author self,
                          PubKey self_pub_key, PrivKey self_priv_key)
      : _dao{dao}, _self{self}, _self_pub_key{self_pub_key},
        _self_priv_key{self_priv_key} {}

public:
  static std::pair<PubKey, PrivKey> generateKeys() {
    PubKey pub_k;
    PrivKey priv_k;
    auto res =
        crypto_sign_keypair(reinterpret_cast<unsigned char *>(pub_k.data()),
                            reinterpret_cast<unsigned char *>(priv_k.data()));
    REPLIKON_ASSERT(res == 0);
    return {std::move(pub_k), std::move(priv_k)};
  }

  template <typename T> //
  Signature sign(const T &value) const {
    Signature s;
    serde::Buffer buf;
    replikon::serde::serialize(buf, value);
    serde::BufferView view{buf};
    auto res = crypto_sign_detached(
        reinterpret_cast<unsigned char *>(s.data()), nullptr,
        reinterpret_cast<const unsigned char *>(view.data()), view.size(),
        reinterpret_cast<const unsigned char *>(_self_priv_key.data()));
    REPLIKON_ASSERT(res == 0);
    LOGD("Signing: toSign: %s, signature %s", hexStr(buf).c_str(),
         hexStr(s).c_str());
    return s;
  }

  template <typename T>
  bool isValid(const Signature &s, const std::string &author,
               const T &value) const {
    auto user_info_res = _dao->getUserInfo(author);
    if (!user_info_res.hasValue() || !user_info_res.value().has_value()) {
      return false;
    }
    auto user_info = user_info_res.value().value();

    serde::Buffer buf;
    replikon::serde::serialize(buf, value);
    serde::BufferView view{buf};

    auto res = crypto_sign_verify_detached(
        reinterpret_cast<const unsigned char *>(s.data()),
        reinterpret_cast<const unsigned char *>(view.data()), view.size(),
        reinterpret_cast<const unsigned char *>(user_info.pub_key.data()));
    LOGD("IsValid: toCheck: %s, signature %s, verdict %d", hexStr(buf).c_str(),
         hexStr(s).c_str(), res);
    return res == 0;
  }

private:
  std::shared_ptr<dao::SecurityDao> _dao;
  Author _self;
  PubKey _self_pub_key;
  PrivKey _self_priv_key;
};
} // namespace replikon::sec
#endif // REPLIKON_SECURITY_PROVIDER_H