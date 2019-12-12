#ifndef KOPT_SRC_PERMUTATION_H__
#define KOPT_SRC_PERMUTATION_H__

#include <vector>

#include "common.h"

namespace kopt {

bool IsPermutation(const std::vector<int> &vec);

class Permutation {
 public:
  static Permutation Random(int n);
  explicit Permutation(int n = 0);
  explicit Permutation(const std::vector<int> &);
  explicit Permutation(std::vector<int> &&);

  Permutation(const Permutation &) = default;
  Permutation(Permutation &&) = default;

  Permutation &operator=(const Permutation &) = default;
  Permutation &operator=(Permutation &&) = default;

  int N() const { return Size(perm); }
  int operator[](int idx) const {
    assert(0 <= idx && idx < N());
    return perm[idx];
  }

  bool Next(int begin = 0);

  [[deprecated]] const std::vector<int> &Vec() const { return perm; }

 private:
  std::vector<int> perm;
};

Permutation Inverse(const Permutation &);
Permutation Compose(const Permutation &outer, const Permutation &inner);

int Changes(const Permutation &);

}  // namespace kopt

#endif  // KOPT_SRC_PERMUTATION_H__
