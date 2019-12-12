#ifndef KOPT_CLEVER_FAST_EMBEDDING_H_
#define KOPT_CLEVER_FAST_EMBEDDING_H_

#include <cassert>
#include <limits>
#include <iostream>
#include <unordered_set>

#include <embedding.h>
#include <monotonic_sequence.h>
#include <set.h>

namespace kopt {

class Embedding : public EmbeddingInterface {
 public:
  class Restricted;

  // Creates an empty embedding.
  Embedding() = default;

  // Creates a lexicographically smallest embedding.
  Embedding(int domain, int codomain);
  Embedding(Set<SigEdge> domain, int codomain);

  Embedding(const Embedding &) = default;
  Embedding& operator=(const Embedding &) = default;

  Set<SigEdge> Domain() const override { return domain_; }
  int Codomain() const { return values_.MaxValue(); }

  // Returns a unique ID among embeddings with a fixed domain.
  unsigned Id() const;

  // Conceptually: creates an embedding with an edge removed from the domain.
  // Actually: the returned object is not a full-fledged embedding; it only has the Id() method.
  Restricted operator-(SigEdge edge) const;

  // The upper bound for values returned by Id().
  unsigned IdSize() const;
  static unsigned IdSize(Set<SigEdge> domain, int codomain);

  bool Next();

  CycleNode FastMapNode(int idx) const { return CycleNode(values_[idx / 2] + idx % 2); }

 private:
  Set<SigEdge> domain_{};
  Subset values_{};

  CycleEdge MapEdge(SigEdge edge) const override;
};

// Implementation
// =====================================================================================================================

inline CycleEdge Embedding::MapEdge(SigEdge edge) const {
  return CycleEdge(values_[domain_.Index(edge)]);
}

class Embedding::Restricted {
 public:
  unsigned Id() const { return index_; }

 private:
  const unsigned index_;
  explicit Restricted(unsigned index) : index_(index) {}
  friend class Embedding;
};



}  // namespace kopt

#endif  // KOPT_CLEVER_FAST_EMBEDDING_H_
