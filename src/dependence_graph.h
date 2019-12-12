#ifndef KOPT_COMMON_DEPENDENCE_GRAPH_H
#define KOPT_COMMON_DEPENDENCE_GRAPH_H

#include <iostream>
#include <tuple>
#include <vector>

#include <matching.h>

namespace kopt {

class DependenceGraph {
 public:
  DependenceGraph() = default;
  explicit DependenceGraph(const Matching &matching);

  friend bool operator<(const DependenceGraph &, const DependenceGraph &);
  friend bool operator==(const DependenceGraph &, const DependenceGraph &);

//  void DimacsFormat(std::ostream *) const;
  friend std::istream& operator>>(std::istream &, DependenceGraph &);
//  friend std::ostream& operator<<(std::ostream &, const DependenceGraph &);

 private:
  using Edge = std::tuple<int, int>;

  int node_count_{};
  int edge_count_{};
  std::vector<Edge> edges_{};

  auto Tie() const;
};

// Implementation.
// =====================================================================================================================

inline auto DependenceGraph::Tie() const { return std::tie(node_count_, edge_count_, edges_); }

inline bool operator<(const DependenceGraph &l, const DependenceGraph &r) { return l.Tie() < r.Tie(); }
inline bool operator==(const DependenceGraph &l, const DependenceGraph &r) { return l.Tie() == r.Tie(); }

}  // namespace kopt

#endif  // KOPT_COMMON_DEPENDENCE_GRAPH_H
