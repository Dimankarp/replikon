#ifndef REPLIKON_CRDTS_MAP_CRDT_H
#define REPLIKON_CRDTS_MAP_CRDT_H

#include "traits/crdt.h"
#include "utils.h"
#include <map>
#include <utility>
#include <vector>
namespace replikon::crdt {

template <typename K, typename ValueCRDT> //
class MapCRDT {
  static_assert(traits::IsCRDT<ValueCRDT>::value,
                "Register must fulfill CRDT trait");

public:
  using Header = std::vector<std::pair<K, typename ValueCRDT::Header>>;
  using Request = std::vector<std::pair<K, typename ValueCRDT::Request>>;
  using Update = std::vector<std::pair<K, typename ValueCRDT::Update>>;
  using ValueUpdate = typename ValueCRDT::Update;

  Header GetHeader() const {
    Header header;
    for (auto &&[k, v] : values) {
      header.emplace_back(k, v.GetHeader());
    }
    return header;
  }

  Request GetRequest(Header &header) {
    Request request;
    for (auto &&[k, head] : header) {
      auto &v = values[k];
      request.emplace_back(k, v.GetRequest(head));
    }
    return request;
  }

  Update GetUpdate(Request &request) {
    Update update;
    for (auto &&[k, req] : request) {
      auto &v = values[k];
      update.emplace_back(k, v.GetUpdate(req));
    }
    return update;
  }

  MergeStatus Merge(Update &update) {

    MergeStatus status = MergeStatus::MERGED;
    for (auto &&[k, upd] : update) {
      auto &v = values[k];
      auto s = v.Merge(upd);
      if (s == MergeStatus::SKIPPED) {
        status = MergeStatus::SKIPPED;
      }
    }
    return status;
  }

  void LocalUpdate(const K &key, ValueUpdate &&update) {
    values[key].LocalUpdate(update);
  }

private:
  std::map<K, ValueCRDT> values;
};

} // namespace replikon::crdt

#endif // REPLIKON_CRDTS_MAP_CRDT_H
