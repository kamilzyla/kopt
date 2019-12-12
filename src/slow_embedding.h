#ifndef KOPT_SLOW_EMBEDDING_H_
#define KOPT_SLOW_EMBEDDING_H_

#include <vector>

#include <embedding.h>
#include <monotonic_sequence.h>
#include "matching.h"
#include "fast_embedding.h"

namespace kopt {

class SlowEmbedding : public EmbeddingInterface {
 public:
  explicit SlowEmbedding(int codomain = 0);
  explicit SlowEmbedding(const Embedding &);

  Set<SigEdge> Domain() const override;
  int Codomain() const;

  int Index() const;
  void SetVal(SigEdge arg, CycleEdge val);
  void Remove(SigEdge);

 private:
  Set<SigEdge> domain_;
  int codomain_;
  std::vector<CycleEdge> values_;

  CycleEdge MapEdge(SigEdge edge) const override;
};

struct Kmove {
  Kmove() = default;

  int64_t gain{};
  MatchingId matching_id{};
  SlowEmbedding embedding{};
};


}  // namespace kopt

#endif  // KOPT_SLOW_EMBEDDING_H_
