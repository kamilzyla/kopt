#ifndef KOPT_COMMON_MATCHING_H_
#define KOPT_COMMON_MATCHING_H_

#include <cassert>
#include <cstddef>
#include <ostream>
#include <vector>

#include <set.h>
#include <identifier.h>
#include <array>
#include "common.h"

namespace kopt {

using MatchingId = std::array<char, 8>;

inline int Len(MatchingId id) {
  int len = 0;
  while (len < 8 && id[len]) ++len;
  return len;
}

inline std::ostream& operator<<(std::ostream &stream, MatchingId id) {
  char s[id.size() + 1] = {};
  for (size_t i = 0; i < id.size(); ++i)
    s[i] = id[i];
  return stream << s;
}

class Matching {
 public:
  // Creates an empty matching.
  Matching() = default;
  // Creates the lexicographically smallest matching on 2*pairs elements.
  explicit Matching(int pairs);
  // Creates a matching with the given id.
  explicit Matching(MatchingId);
  // Creates a matching based on the given permutation of elements 1, ..., 2*k for some k. The behavior is undefined
  // if the argument is invalid. For testing purposes.
  explicit Matching(std::vector<int> permutation);

  bool Reducible() const;  // True if the matching replaces an edge with itself.
  Set<SigNode> Domain() const;
  SigNode operator()(SigNode node) const;
  bool Next();
  bool NextIrreducible();

  MatchingId Id() const;
  friend std::ostream& operator<<(std::ostream &, const Matching &);

 private:
  std::vector<int> matching_;  // A permutation of numbers 0, 1, ..., 2*pairs-1.
  std::vector<int> p_;  // Permutation of the cycle pieces.
  std::vector<int> o_;  // Orientation of the cycle pieces.

  void UpdateMatching();
};

// Implementation
// =====================================================================================================================

inline Set<SigNode> Matching::Domain() const {
  return Set<SigNode>::Full(static_cast<int>(matching_.size()));
}

inline SigNode Matching::operator()(SigNode node) const {
  assert(0 <= node.id && node.id < Size(matching_));
  return SigNode(matching_[node.id]);
}

}  // namespace kopt

#endif  // KOPT_COMMON_MATCHING_H_
