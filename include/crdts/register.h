#ifndef REPLIKON_CRDTS_REGISTER_H
#define REPLIKON_CRDTS_REGISTER_H

#include "utils.h"
#include <variant>
namespace replikon::crdt {

/*
 * Single-value Register
 */

template <typename Value> class Register {
public:
  using Header = std::monostate;
  using Request = std::monostate;
  using Update = Value;

  Header GetHeader() const { return {}; }
  Request GetRequest(Header) const { return {}; }
  Update GetUpdate(Request) const { return value; }
  MergeStatus Merge(Update u) {
    value = u;
    return MergeStatus::MERGED;
  }

private:
  Value value;
};

static_assert(traits::IsCRDT<Register<int>>::value,
              "Register must fulfill CRDT trait");

} // namespace replikon::crdt

#endif // REPLIKON_CRDTS_REGISTER_H