#include "common.h"

#include <gflags/gflags.h>

DEFINE_int64(seed, 0, "seed for the random engine");

namespace kopt {

std::vector<int> Sequence(int n) {
  assert(n >= 0);
  std::vector<int> seq(n);
  for (int i = 0; i < n; ++i) seq[i] = i;
  return seq;
}

std::mt19937 &Rng() {
  static std::mt19937 engine(FLAGS_seed);
  return engine;
}

std::vector<CycleNode> IdentityCycle(int n) {
  std::vector<CycleNode> result(n);
  for (int i = 0; i < n; ++i)
    result[i] = CycleNode(i);
  return result;
}

std::vector<int> ToInts(const std::vector<CycleNode> &vec) {
  int n = Size(vec);
  std::vector<int> res(n);
  for (int i = 0; i < n; ++i)
    res[i] = vec[i].id;
  return res;
}
std::vector<CycleNode> ToIds(const std::vector<int> &vec) {
  int n = Size(vec);
  std::vector<CycleNode> res(n);
  for (int i = 0; i < n; ++i)
    res[i] = CycleNode(vec[i]);
  return res;
}

}  // namespace kopt
