#ifndef KOPT_CLEVER_GRAPH_H_
#define KOPT_CLEVER_GRAPH_H_

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

#include "permutation.h"

namespace kopt {

using Weight = std::int64_t;
constexpr Weight kMinWeight = std::numeric_limits<Weight>::min();
constexpr Weight kMaxWeight = std::numeric_limits<Weight>::max();

struct Point {
  double x, y;
};

class Graph {
 public:
  static Graph Random(int n);
  explicit Graph(int n = 0);

  Graph(const Graph &) = default;
  Graph(Graph &&) = default;

  Graph &operator=(const Graph &) = default;
  Graph &operator=(Graph &&) = default;

  int N() const { return n; }
  const Point &operator[](int v) const { return points[CheckIdx(v)]; }
  Weight operator()(int u, int v) const { return Dist(CheckIdx(u), CheckIdx(v)); }

  void Permutate(const Permutation &);
  const Permutation &GetPermutation() const;

  // Read Graph in TSPLIB format.
  friend std::istream &operator>>(std::istream &, Graph &);

  [[deprecated]] Weight GetWeight(CycleEdge e) const { return Dist(e.id, e.id + 1); }
  [[deprecated]] Weight GetWeight(CycleNode u, CycleNode v) const { return Dist(u.id, v.id); }
  [[deprecated]] Weight CycleWeight() const {
    Weight weight = 0;
    for (int i = 0; i < n; ++i)
      weight += Dist(i, i + 1);
    return weight;
  }
  [[deprecated]] Weight CycleWeight(const std::vector<CycleNode> &cycle) const {
    Weight weight = GetWeight(cycle[n - 1], cycle[0]);
    for (int i = 1; i < n; ++i)
      weight += GetWeight(cycle[i - 1], cycle[i]);
    return weight;
  }
  [[deprecated]] void ApplyPermutation(const std::vector<CycleNode> &cycle) {
    Permutate(Permutation(ToInts(cycle)));
  }
  [[deprecated]] void ResetPermutation() {
    Permutate(Inverse(GetPermutation()));
  }
  [[deprecated]] std::vector<CycleNode> GetPermutationIds() const {
    return ToIds(GetPermutation().Vec());
  }
  [[deprecated]] void RandomShuffle() {
    Permutate(Permutation::Random(n));
  }

 private:
  Weight Dist(int u, int v) const {
    Point a = points[u], b = points[v];
    // TSPLIB documentation dictates the use of cast instead of round().
    // NOLINTNEXTLINE(bugprone-incorrect-roundings)
    return Weight(std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)) + 0.5);
  }

  int CheckIdx(int idx) const {
    assert(n > 0);
    assert(idx >= 0);
    assert(idx <= n);
    return idx;
  }

  void CheckInvariant() const;

  int n;
  std::vector<Point> points;
  Permutation perm;
};

Weight CycleWeight(const Graph &, const Permutation &cycle);

void WriteGraph(std::ostream *out, const Graph &graph, const std::string &name = "");
void WriteTours(std::ostream *out,
                const std::vector<Permutation> &tours,
                const std::string &name = "",
                int dimension = -1);

}  // namespace kopt

#endif  // KOPT_CLEVER_GRAPH_H_
