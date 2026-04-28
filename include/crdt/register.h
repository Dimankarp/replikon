#ifndef REPLIKON_CRDTS_REGISTER_H
#define REPLIKON_CRDTS_REGISTER_H

#include "traits/crdt.h"
#include "utils.h"
#include <variant>
namespace replikon::crdt {

/*
 * Single-value Register
 */

template <typename Value> //
class Register {
public:
  using Header = std::monostate;
  using Request = std::monostate;
  using Update = Value;

  Header getHeader() const { return {}; }
  Request getRequest(Header) const { return {}; }
  Update getUpdate(Request) const { return _value; }
  MergeStatus merge(Update u) {
    _value = u;
    return MergeStatus::MERGED;
  }

  void localUpdate(Update u) { merge(u); }

private:
  Value _value;
};

static_assert(traits::IsCRDT<Register<int>>::value,
              "Register must fulfill CRDT trait");

} // namespace replikon::crdt

#endif // REPLIKON_CRDTS_REGISTER_H