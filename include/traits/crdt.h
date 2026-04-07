#ifndef REPLIKON_TRAITS_CRDT_H
#define REPLIKON_TRAITS_CRDT_H

#include "utils.h"
#include <type_traits>
namespace replikon::traits {

template <typename T> struct CRDT {
  using Header = typename T::Header;
  using Request = typename T::Request;
  using Update = typename T::Update;
  static_assert(
      std::is_same_v<decltype(std::declval<T>().Merge(std::declval<Update>())),
                     replikon::MergeStatus>,
      "CRDT::Merge(Update) must return MergeStatus");
};

template <typename T, typename = std::void_t<>>
struct IsCRDT : std::false_type {};

template <typename T>
struct IsCRDT<T,
              std::void_t<typename CRDT<T>::Header,  //
                          typename CRDT<T>::Request, //
                          typename CRDT<T>::Update>> : std::true_type {};

} // namespace replikon::traits

#endif // REPLIKON_TRAITS_CRDT_H
