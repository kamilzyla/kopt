#ifndef KOPT_COMMON_COMMON_H_
#define KOPT_COMMON_COMMON_H_

#include <cassert>
#include <random>

#include "identifier.h"

namespace kopt {

template<class T>
int Size(const T &container) { return static_cast<int>(container.size()); }

template<class T>
std::ostream& operator<<(std::ostream &stream, const std::vector<T> &v) {
  stream << "{";
  for (int i = 0; i < Size(v); ++i)
    stream << (i ? ", " : "") << v[i];
  return stream << "}";
}

std::vector<int> Sequence(int n);

std::mt19937 &Rng();

[[deprecated]] std::vector<CycleNode> IdentityCycle(int n);
[[deprecated]] std::vector<int> ToInts(const std::vector<CycleNode> &vec);
[[deprecated]] std::vector<CycleNode> ToIds(const std::vector<int> &vec);

}

#endif  // KOPT_COMMON_COMMON_H_
