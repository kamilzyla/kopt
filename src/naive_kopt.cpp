#include <naive_kopt.h>

#include <gain_func.h>
#include <fast_embedding.h>
#include <matching.h>
#include <retrieve_solution.h>

namespace kopt {

Kmove Naive2optBase(const Graph &g) {
  int64_t best_gain = std::numeric_limits<int64_t>::min();
  CycleEdge best_i, best_j;
  for (CycleEdge i(0); i.id < g.N(); ++i.id) {
    for (CycleEdge j(i.id + 1); j.id < g.N(); ++j.id) {
      int64_t gain = g.GetWeight(i) + g.GetWeight(j) - g.GetWeight(i.Left(), j.Left()) - g.GetWeight(i.Right(), j.Right());
      if (gain > best_gain) {
        best_gain = gain;
        best_i = i;
        best_j = j;
      }
    }
  }
  SlowEmbedding e(g.N());
  e.SetVal(SigEdge(0), best_i);
  e.SetVal(SigEdge(1), best_j);
  return Kmove{best_gain, {'a'}, e};
}

std::vector<CycleNode> Naive2opt(const Graph &g) {
  auto result = Naive2optBase(g);
  return RetrieveSolution(g.N(), Matching(result.matching_id), result.embedding);
}

static std::vector<CycleNode> retrieve(int n, int type, int i, int j, int k) {
  std::vector<CycleNode> sol(n);

  struct {
    int from, to, step;
  } r[4][2] = {
      {{j+1, k+1, 1}, {i+1, j+1, 1}},
      {{k, j, -1}, {i+1, j+1, 1}},
      {{j, i, -1}, {k, j, -1}},
      {{j+1, k+1, 1}, {j, i, -1}},
  };

  int l = 0;
  for (int at = 0; at <= i; ++at) sol[l++] = CycleNode(at + 1);
  for (int at = r[type][0].from; at != r[type][0].to; at += r[type][0].step) sol[l++] = CycleNode(at + 1);
  for (int at = r[type][1].from; at != r[type][1].to; at += r[type][1].step) sol[l++] = CycleNode(at + 1);
  for (int at = k+1; at < n; ++at) sol[l++] = CycleNode(at + 1);
  return sol;
}

Kmove Naive3optBase(const Graph &g) {
  int n = g.N();

  struct {
    int64_t gain, type, i, j, k;
  } best{};
  best.gain = 0;
  for (int i = 0; i < n; ++i) for (int j = i+1; j < n; ++j) for (int k = j+1; k < n; ++k) {
    int64_t cost[4] = {
        g(i, j+1) + g(k, i+1) + g(j, k+1),
        g(i, k) + g(j+1, i+1) + g(j, k+1),
        g(j, i) + g(k+1, j+1) + g(k, i+1),
        g(k, j) + g(i+1, k+1) + g(i, j+1),
    };
    int type = 0;
    for (int l = 1; l < 4; ++l)
      if (cost[l] < cost[type]) type = l;
    int gain = g(i, i+1) + g(j, j+1) + g(k, k+1) - cost[type];
    if (gain > best.gain)
      best = {gain, type, i, j, k};
  }

  MatchingId id;
  switch (best.type) {
    case 0: id = MatchingId{'B', 'A'}; break;
    case 1: id = MatchingId{'b', 'A'}; break;
    case 2: id = MatchingId{'a', 'b'}; break;
    case 3: id = MatchingId{'B', 'a'}; break;
  }
  SlowEmbedding e;
  e.SetVal(SigEdge(0), CycleEdge(best.i));
  e.SetVal(SigEdge(1), CycleEdge(best.j));
  e.SetVal(SigEdge(2), CycleEdge(best.k));
  return Kmove{best.gain, id, e};
}

std::vector<CycleNode> Naive3opt(const Graph &g) {
  auto result = Naive3optBase(g);
  return RetrieveSolution(g.N(), Matching(result.matching_id), result.embedding);
}

// === Experimental 3-opt ===

template<int k>
struct TemplateSubset {
  explicit TemplateSubset(int n) {
    for (int i = 0; i < k; ++i) v[i] = i;
    v[k] = n;
  }

  bool Next() {
    int i = 0;
    for (i = 0; i < k && v[i] + 1 == v[i + 1]; ++i)
      v[i] = i;
    if (i < k) {
      ++id;
      ++v[i];
      return true;
    } else {
      id = 0;
      return false;
    }
  }

  int Id() const { return id; }
  int MapEndpoint(int x) const { return v[x / 2] + x % 2; }
  int operator[](int idx) const { return v[idx]; }

  int v[k + 1], id{};
};

struct Edge {
  int x, y;
};

template<int k>
using SignatureEdges = std::array<Edge, k>;

template<int k>
int64_t RemovedWeight(const Graph &graph, const TemplateSubset<k> &subset) {
  int64_t weight = 0;
  for (int i = 0; i < k; ++i)
    weight += graph(subset[i], subset[i] + 1);
  return weight;
}

template<int k>
int64_t AddedWeight(const Graph &graph, const SignatureEdges<k> &sig, const TemplateSubset<k> &subset) {
  int64_t weight = 0;
  for (auto &edge : sig)
    weight += graph(subset.MapEndpoint(edge.x), subset.MapEndpoint(edge.y));
  return weight;
}

struct KoptResult {
  int64_t gain = 0;
  int i = -1, j = -1, k = -1;
  int signature = -1;

  bool operator<(const KoptResult &right) const { return gain < right.gain; }
};

template<int k>
KoptResult kopt(const std::vector<SignatureEdges<k>> &signatures, const Graph &graph) {
  TemplateSubset<k> subset(graph.N());
  KoptResult result;
  do {
    int64_t removed = RemovedWeight<k>(graph, subset);
    for (int i = 0; i < Size(signatures); ++i) {
      int64_t added = AddedWeight<k>(graph, signatures[i], subset);
      result = std::max(result, KoptResult{removed - added, subset[0], subset[1], subset[2], i});
    }
  } while (subset.Next());
  return result;
}

// TODO: Fix for n <= 3 (results in SEGFAULT).
std::vector<CycleNode> Experimental3opt(const Graph &g) {
  std::vector<SignatureEdges<3>> signatures = {
      {Edge{0, 3}, {1, 4}, {2, 5}},
      {Edge{0, 4}, {1, 3}, {2, 5}},
      {Edge{0, 2}, {1, 4}, {3, 5}},
      {Edge{0, 3}, {1, 5}, {2, 4}},
  };
  auto result = kopt<3>(signatures, g);
  return retrieve(g.N(), result.signature, result.i, result.j, result.k);
}

}  // namespace kopt
