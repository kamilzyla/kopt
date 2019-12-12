#include "permutation.h"

#include <algorithm>
#include <random>
#include <utility>

namespace kopt {

bool IsPermutation(const std::vector<int> &vec) {
  int n = Size(vec);
  std::vector<bool> found(n);
  for (int i = 0; i < n; ++i) {
    if (vec[i] < 0 || vec[i] >= n || found[vec[i]])
      return false;
    found[vec[i]] = true;
  }
  return true;
}

Permutation Permutation::Random(int n) {
  assert(n >= 0);
  auto seq = Sequence(n);
  std::shuffle(seq.begin(), seq.end(), Rng());
  return Permutation(std::move(seq));
}

Permutation::Permutation(int n) {
  assert(n >= 0);
  perm = Sequence(n);
}

Permutation::Permutation(const std::vector<int> &v) {
  assert(IsPermutation(v));
  perm = v;
}

Permutation::Permutation(std::vector<int> &&v) {
  assert(IsPermutation(v));
  perm = std::move(v);
}

bool Permutation::Next(int begin) {
  assert(0 <= begin && begin <= N());
  return std::next_permutation(perm.begin() + begin, perm.end());
}

Permutation Inverse(const Permutation &p) {
  std::vector<int> inv(p.N());
  for (int i = 0; i < p.N(); ++i)
    inv[p[i]] = i;
  return Permutation(std::move(inv));
}

Permutation Compose(const Permutation &outer, const Permutation &inner) {
  assert(outer.N() == inner.N());
  int n = outer.N();
  std::vector<int> composed(n);
  for (int i = 0; i < n; ++i)
    composed[i] = outer[inner[i]];
  return Permutation(std::move(composed));
}

int Changes(const Permutation &perm) {
  int n = perm.N();
  auto AreNeighbors = [&](int i, int j) {
    int dif = std::abs(i - j);
    return dif == 1 || dif == n - 1;
  };
  int neighbors = AreNeighbors(perm[n - 1], perm[0]);
  for (int i = 1; i < n; ++i)
    neighbors += AreNeighbors(perm[i - 1], perm[i]);
  return n - neighbors;
}

}  // namespace kopt
