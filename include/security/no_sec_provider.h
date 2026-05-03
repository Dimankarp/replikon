#ifndef REPLIKON_SECURITY_NO_SEC_PROVIDER_H
#define REPLIKON_SECURITY_NO_SEC_PROVIDER_H

#include "types.h"
#include <variant>
namespace replikon::sec {
class NoSecurityProvider {
public:
  using Signature = std::monostate;
  
  NoSecurityProvider() = default;

  template <typename T> //
  Signature sign(const T &value) const {
    return {};
  }

  template <typename T> //
  bool isValid(const Signature &s, const std::string &author,
               const T &value) const {
    return true;
  }
};
} // namespace replikon::sec

#endif // REPLIKON_SECURITY_NO_SEC_PROVIDER_H