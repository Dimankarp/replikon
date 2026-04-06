#ifndef REPLIKON_CRDT_H
#define REPLIKON_CRDT_H

namespace replikon {

class ICRDT {
  virtual void Merge() {};
  virtual ~ICRDT() = delete;
};


} // namespace replikon

#endif // REPLIKON_CRDT_H