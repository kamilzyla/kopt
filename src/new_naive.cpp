#include "new_naive.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <random>

#include "common.h"

namespace kopt {
namespace {

std::vector<int> Sequence(int n) {
  assert(n >= 0);
  std::vector<int> seq(n);
  for (int i = 0; i < n; ++i) seq[i] = i;
  return seq;
}

constexpr int kMaxK = 7;

struct Subset {
  explicit Subset(int k = 0, int n = -1) : k(k) {
    assert(k >= 0);
    assert(n == -1 || n >= k);
    for (int i = 0; i < k; ++i) v[i] = i;
    v[k] = n;
  }

  int K() const { return k; }
  int N() const { return v[k]; }

  bool Next() {
    int i = 0;
    for (i = 0; i < k && v[i] + 1 == v[i + 1]; ++i)
      v[i] = i;
    if (i < k) {
      ++v[i];
      return true;
    } else {
      return false;
    }
  }

  int MapEdge(int x) const { return v[x]; }
  int MapNode(int x) const { return v[x / 2] + x % 2; }

  int k;
  std::array<int, kMaxK + 1> v{};
};

struct Frag {
  int idx;
  bool rev;

  int Front() const { return 2 * idx + 1 + rev; }
  int Back() const { return 2 * idx + 2 - rev; }
};

struct SignatureId {
  int K() const { return static_cast<int>(std::strlen(str.data()) + 1); }

  Frag operator()(int i) const {
    bool rev = str[i] >= 'a';
    int idx = str[i] - (rev ? 'a' : 'A');
    return Frag{idx, rev};
  }

  // A string of k-1 characters + terminating null.
  std::array<char, kMaxK> str{};
};

struct Signature {
  struct Edge {
    int x, y;
  };

  explicit Signature(SignatureId id) {
    for (k = 0; id.str[k]; ++k) {
      auto frag = id(k);
      per[k] = frag.idx;
      rev[k] = frag.rev;
    }
    UpdateEdges();
  }

  explicit Signature(int k = 0) : k(k) {
    assert(k >= 0);
    if (k == 0)
      return;
    for (int i = 0; i < k - 1; ++i)
      per[i] = i;
    edge[0].x = 0;
    edge[k - 1].y = 2 * k - 1;
    UpdateEdges();
  }

  int K() const { return k; }

  SignatureId Id() const {
    SignatureId id;
    for (int i = 0; i < k; ++i)
      id.str[i] = (rev[i] ? 'a' : 'A') + per[i];
    id.str[k] = 0;
    return id;
  }

  bool Reducible() const {
    for (int i = 0; i < k; ++i)
      if (std::abs(edge[i].x - edge[i].y) == 1) return true;
    return false;
  }

  bool NextIrreducible() {
    while (Next())
      if (!Reducible()) return true;
    return false;
  }

  bool Next() {
    int i;
    for (i = k - 2; i >= 0; --i) {
      rev[i] = !rev[i];
      if (rev[i]) break;
      else if (i < k - 2 && per[i] < per[i + 1]) {
        int low = k - 2;
        while (per[i] >= per[low]) --low;
        std::swap(per[i], per[low]);
        break;
      }
    }
    std::reverse(per.begin() + i + 1, per.begin() + k - 1);
    UpdateEdges();
    return i >= 0;
  }

  void UpdateEdges() {
    // The i-th edge connects the back of fragment i-1 to the front of fragment i.
    // We always have e[0].x == 0 and e[k-1].y = 2*k-1.
    for (int i = 0; i < k; ++i) {
      if (i > 0) edge[i].x = Frag{per[i - 1], rev[i - 1]}.Back();
      if (i < k - 1) edge[i].y = Frag{per[i], rev[i]}.Front();
    }
  }

  int k;
  std::array<Edge, kMaxK> edge{};
  // Permutation and orientation (reversed or not) of cycle fragments.
  std::array<int, kMaxK - 1> per{};
  std::array<bool, kMaxK - 1> rev{};
};

using Weight = std::int64_t;

struct Kmove {
  Weight gain{};
  SignatureId sig_id;
  Subset subset;

  std::vector<int> Cycle() const {
    assert(sig_id.K() == subset.K() && subset.K() <= subset.N());
    int k = subset.K(), n = subset.N();
    if (k <= 1)
      return Sequence(n);

    std::vector<int> cycle;
    cycle.reserve(subset.N());
    auto AppendFragment = [&](int first, int last, int dir) {
      for (int v = first; v != last + dir; v += dir)
        cycle.emplace_back(v);
    };

    // The (implicit) last fragment crosses the n-1 / 0 boundary.
    AppendFragment(0, subset.MapNode(0), 1);
    for (int i = 0; i < k - 1; ++i) {
      auto frag = sig_id(i);
      int first = subset.MapNode(frag.Front());
      int last = subset.MapNode(frag.Back());
      int dir = frag.rev ? -1 : 1;
      AppendFragment(first, last, dir);
    }
    AppendFragment(subset.MapNode(2 * k - 1), n - 1, 1);
    assert(Size(cycle) == n);

    // Ensure the lexicographically smallest cycle is returned.
    if (cycle[1] > cycle[n - 1])
      std::reverse(cycle.begin() + 1, cycle.end());

    return cycle;
  }

  static bool GainCmp(const Kmove &l, const Kmove &r) { return l.gain > r.gain; }
};

struct Graph {
  struct Point {
    double x, y;
  };

  static Graph Random(int n) {
    Graph g(n);
    std::uniform_int_distribution<> dist(1, 100 * n);
    auto rand = std::bind(dist, Rng());
    for (int i = 0; i < n; ++i) {
      g.p[i].x = rand();
      g.p[i].y = rand();
    }
    g.p[n] = g.p[0];
    return g;
  }

  explicit Graph(int n = 0) : n(n) {
    assert(n >= 0);
    p.resize(n + 1);
  }

  Graph(const Graph &) = default;
  Graph(Graph &&) = default;

  Graph &operator=(const Graph &) = default;
  Graph &operator=(Graph &&) = default;

  int N() const { return n; }

  Weight operator()(int x, int y) const {
    assert(0 <= x && x <= n && 0 <= y && y <= n && x != y);
    return Dist(p[x], p[y]);
  }

  int n;
  std::vector<Point> p;  // Invariant: p.size() == n + 1 && p[0] == p[n].

 private:
  static Weight Dist(Point a, Point b) {
    // TSPLIB documentation dictates the use of cast instead of round().
    // NOLINTNEXTLINE(bugprone-incorrect-roundings)
    return Weight(std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)) + 0.5);
  }
};

Weight RemovedWeight(const Graph &graph, const Subset &subset) {
  Weight weight = 0;
  for (int i = 0; i < subset.K(); ++i) {
    int edge = subset.MapEdge(i);
    weight += graph(edge, edge + 1);
  }
  return weight;
}

Weight AddedWeight(const Graph &graph, const Subset &subset, const Signature &sig) {
  Weight weight = 0;
  for (int i = 0; i < sig.K(); ++i) {
    int x = subset.MapNode(sig.edge[i].x);
    int y = subset.MapNode(sig.edge[i].y);
    weight += graph(x, y);
  }
  return weight;
}

Kmove Kopt(int k, const Graph &graph) {
  Kmove best;
  Subset subset(k, graph.N());
  Signature signature(k);
  do {
    auto removed_weight = RemovedWeight(graph, subset);
    while (signature.NextIrreducible()) {
      auto gain = removed_weight - AddedWeight(graph, subset, signature);
      if (gain > best.gain)
        best = Kmove{gain, signature.Id(), subset};
    }
  } while (subset.Next());
  return best;
}

void TestNewNaive_(int k, int n) {
  auto g = Graph::Random(n);
  Kopt(k, g);
}

}  // namespace

void TestNewNaive(int k, int n) { return TestNewNaive_(k, n); }

}  // namespace kopt
