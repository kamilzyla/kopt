#ifndef KOPT_COMMON_DECOMPOSITION_LIBRARY_H_
#define KOPT_COMMON_DECOMPOSITION_LIBRARY_H_

#include <functional>
#include <iostream>
#include <vector>

#include <decomposition.h>
#include <dependence_graph.h>

namespace kopt {

class DecompositionLibrary {
 public:
  using Decomposer = std::function<Decomposition::Ptr(const DependenceGraph &)>;

  DecompositionLibrary() = default;

  explicit DecompositionLibrary(int kmove_size, Decomposer decomposer);

  const Decomposition& operator[](const DependenceGraph &) const;

  friend std::istream& operator>>(std::istream &, DecompositionLibrary &);
//  friend std::ostream& operator<<(std::ostream &, const DecompositionLibrary &);

 private:
  std::vector<DependenceGraph> argument_;
  std::vector<Decomposition::Ptr> value_;
};

}  // namespace kopt

#endif  // KOPT_COMMON_DECOMPOSITION_LIBRARY_H_
