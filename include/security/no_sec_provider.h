#ifndef REPLIKON_SECURITY_NO_SEC_PROVIDER_H
#define REPLIKON_SECURITY_NO_SEC_PROVIDER_H

#include "types.h"
#include <variant>
namespace replikon::sec {
class NoSecurityProvider {
public:
  using Author = Author;
  using PubKey = std::monostate;
  using PrivKey = std::monostate;
  using Signature = std::monostate;

  Signature sign(serde::BufferView view) const { return {}; }

  bool isValid(const Signature &s, serde::BufferView view,
               const std::string &author) const {
    return true;
  }
};
} // namespace replikon::sec

#endif // REPLIKON_SECURITY_NO_SEC_PROVIDER_H