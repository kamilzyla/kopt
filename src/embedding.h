#ifndef KOPT_COMMON_EMBEDDING_H_
#define KOPT_COMMON_EMBEDDING_H_

#include <ostream>

#include <identifier.h>
#include <set.h>

namespace kopt {

int Binom(int n, int k);

class EmbeddingInterface {
 public:
  virtual ~EmbeddingInterface() = default;

  virtual Set<SigEdge> Domain() const = 0;

  CycleEdge operator()(SigEdge) const;
  CycleNode operator()(SigNode) const;

 private:
  virtual CycleEdge MapEdge(SigEdge) const = 0;
};

std::ostream& operator<<(std::ostream &, const EmbeddingInterface &);

// Implementation
// =====================================================================================================================

inline CycleEdge EmbeddingInterface::operator()(SigEdge edge) const {
  return MapEdge(edge);
}

inline CycleNode EmbeddingInterface::operator()(SigNode node) const {
  return CycleNode(MapEdge(SigEdge(node.id / 2)).id + node.id % 2);
}

}  // namespace kopt

#endif  // KOPT_COMMON_EMBEDDING_H_
