#ifndef KOPT_COMMON_GAIN_FUNC_H_
#define KOPT_COMMON_GAIN_FUNC_H_

#include <fast_embedding.h>
#include <graph.h>
#include <identifier.h>
#include <matching.h>
#include <set.h>

namespace kopt {

class GainFunc {
 public:
  // Saves the references to graph and matching - no copies are made.
  GainFunc(const Graph &graph, const Matching &matching) : graph_(graph), matching_(matching) {}

  int64_t Introduce(const EmbeddingInterface &embedding, SigEdge introduced) const;
  int64_t Join(const EmbeddingInterface &embedding) const;

 private:
  const Graph &graph_;
  const Matching &matching_;

  int64_t Check(const EmbeddingInterface &embedding, SigNode x, bool require_ordered = false) const;
};

// Implementation
// =====================================================================================================================

inline int64_t GainFunc::Introduce(const EmbeddingInterface &embedding, SigEdge introduced) const {
  int64_t gain = graph_.GetWeight(embedding(introduced));
  gain -= Check(embedding, introduced.Left());
  gain -= Check(embedding, introduced.Right());
  return gain;
}

inline int64_t GainFunc::Join(const EmbeddingInterface &embedding) const {
  int64_t gain = 0;
  for (auto edge : embedding.Domain()) {
    gain += graph_.GetWeight(embedding(edge));
    gain -= Check(embedding, edge.Left(), true);
    gain -= Check(embedding, edge.Right(), true);
  }
  return gain;
}

inline int64_t GainFunc::Check(const EmbeddingInterface &embedding, SigNode x, bool require_ordered) const {
  SigNode y = matching_(x);
  if ((!require_ordered || x.id < y.id) && embedding.Domain().Contains(y.Edge()))
    return graph_.GetWeight(embedding(x), embedding(y));
  else
    return 0;
}

}  // namespace kopt

#endif  // KOPT_COMMON_GAIN_FUNC_H_
