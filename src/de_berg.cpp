#include <de_berg.h>

#include <algorithm>

#include <common.h>
#include <matching.h>
#include "slow_embedding.h"
#include "retrieve_solution.h"

namespace kopt {

struct FastSubset {
  FastSubset(int k = 0, int n = -1) : k(k), v(k + 1) {
    for (int i = 0; i < k; ++i) v[i] = i;
    v[k] = n;
  }

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

  const int &operator[](int idx) const { return v[idx]; }
  int &operator[](int idx) { return v[idx]; }

  int MapNode(int x) const { return x < 0 ? 0 : x < 2 * k ? v[x / 2] + x % 2 : v[k]; }

  friend std::ostream &operator<<(std::ostream &s, const FastSubset &v) {
    return s << v.v;
  }

  int k;
  std::vector<int> v;
};

struct FasterSubset {
  struct Edge {
    int64_t weight;
    int idx;
    bool operator<(const Edge &r) const { return weight > r.weight; }
  };

  FastSubset subset;
  std::vector<int> permutation, now;

  FasterSubset(int k, const Graph &graph) : subset(k, graph.N()), permutation(graph.N()), now(k) {
    std::vector<Edge> edges(graph.N());
    for (int i = 0; i < graph.N(); ++i)
      edges[i] = Edge{graph(i, i + 1), i};
    std::sort(edges.begin(), edges.end());
    for (int i = 0; i < graph.N(); ++i)
      permutation[i] = edges[i].idx;
    UpdateNow();
  }

  int operator[](int idx) const { return now[idx]; }
  int MapNode(int x) const { return x < 0 ? 0 : x < 2 * subset.k ? now[x / 2] + x % 2 : subset.v[subset.k]; }

  bool Next() {
    bool ret = subset.Next();
    UpdateNow();
    return ret;
  }

  void UpdateNow() {
    for (int i = 0; i < subset.k; ++i)
      now[i] = permutation[subset[i]];
    std::sort(now.begin(), now.end());
  }
};

using Signature = std::vector<int>;

struct DynamicTable {
  std::vector<int> best_idx;
  std::vector<int64_t> gain;
  int ydim;

  DynamicTable(int xdim, int ydim) : ydim(ydim) {
    best_idx.resize(xdim * ydim);
    gain.resize(ydim);
  }

  int64_t &Gain(int y) { return gain[y]; }
  int &BestIdx(int x, int y) { return best_idx[x * ydim + y]; }
};

struct Dynamic {
  struct Edge {
    int i, x, y;

    int64_t Gain(const Graph &graph, int i = -1) const {
      if (i == -1) i = this->i;
      return graph(i, i + 1) - graph(i, x) - graph(i + 1, y);
    }

    friend std::ostream &operator<<(std::ostream &stream, const Dynamic::Edge edge) {
      return stream << '(' << edge.i << ", " << edge.x << ", " << edge.y << ")";
    }
  };

  int begin, end;
  std::vector<Edge> edges;

  bool Feasible() {
    return Size(edges) <= end - begin;
  }

  void Run(const Graph &graph, DynamicTable *t) {
    int n = end - begin, m = Size(edges);
    for (int i = 0; i < n; ++i) {
      for (int j = std::min(i, m - 1); j >= 0; --j) {
        if (i > j)
          t->BestIdx(i, j) = t->BestIdx(i - 1, j);
        int64_t gain = (j ? t->Gain(j - 1) : 0) + edges[j].Gain(graph, begin + i);
        if (i == j || gain > t->Gain(j)) {
          t->Gain(j) = gain;
          t->BestIdx(i, j) = i;
        }
      }
    }
    for (int i = n, j = m - 1; j >= 0; --j) {
      i = t->BestIdx(i - 1, j);
      edges[j].i = begin + i;
    }
  }

  friend std::ostream &operator<<(std::ostream &stream, const Dynamic &dynamic) {
    return stream << "begin = " << dynamic.begin << ", end = " << dynamic.end << ", edges = " << dynamic.edges;
  }
};

struct DynamicData {
  Dynamic unmapped, mapped;

  void Map(const FastSubset &subset) {
    mapped.begin = subset.MapNode(unmapped.begin);
    mapped.end = subset.MapNode(unmapped.end);
    mapped.edges.resize(unmapped.edges.size());
    for (int i = 0; i < Size(unmapped.edges); ++i) {
      mapped.edges[i].x = subset.MapNode(unmapped.edges[i].x);
      mapped.edges[i].y = subset.MapNode(unmapped.edges[i].y);
    }
  }
};

struct ReducedIndex {
  int k, l;
  std::vector<int> idx;

  explicit ReducedIndex(const Signature &sig) : k(Size(sig) / 2), l(), idx(k) {
    std::vector<bool> visited(k), reduce(k);
    for (int i = 0; i < k; ++i) {
      for (int ep = 2 * i, step = 0; !visited[ep / 2]; ep = sig[ep ^ 1], ++step) {
        visited[ep / 2] = true;
        reduce[ep / 2] = step % 2;
      }
    }
    for (int i = 0; i < k; ++i)
      idx[i] = reduce[i] ? -1 : l++;
  }

  bool Dep(int edge) { return edge >= k || idx[edge] >= 0; }
  int operator()(int ep) {
    return ep < 0 ? -1 : ep < 2 * k ? 2 * idx[ep / 2] + ep % 2 : 2 * l;
  }
};

struct DeBergSignature {
  struct AddEdge {
    int x, y;
  };

  Matching matching;
  int k;  // Size of the signature before reduction.
  std::vector<int> del;  // Original indices of edges remaining after reduction.
  std::vector<AddEdge> add;
  std::vector<DynamicData> dyn;

  explicit DeBergSignature(const Matching &matching) : matching(matching), k(matching.Domain().Size() / 2) {
    Signature sig(2 * k);
    for (int i = 0; i < 2 * k; ++i)
      sig[i] = matching(SigNode(i)).id;
    Init(sig);
  }

  void Init(const Signature &sig) {
    ReducedIndex idx(sig);
    for (int i = 0; i < k; ++i)
      if (idx.Dep(i)) del.emplace_back(i);
    for (int i = 0; i < 2 * k; ++i) {
      int j = sig[i];
      if (i < j && idx.Dep(i / 2) && idx.Dep(j / 2))
        add.emplace_back(AddEdge{idx(i), idx(j)});
    }
    for (int i = 0; i < k; ++i)
      if (!idx.Dep(i)) {
        dyn.emplace_back();
        auto &d = dyn.back().unmapped;
        d.begin = idx(2 * i - 1);
        do d.edges.emplace_back(Dynamic::Edge{i, idx(sig[2 * i]), idx(sig[2 * i + 1])});
        while (!idx.Dep(++i));
        d.end = idx(2 * i);
      }
  }

  int64_t Embed(const Graph &graph, FastSubset *result) {
    int64_t best_gain = 0;
    DynamicTable table(graph.N(), DynSize());
    FastSubset subset(Size(del), graph.N());
    do {
      bool feasible = true;
      for (auto &dynamic : dyn) {
        dynamic.Map(subset);
        feasible &= dynamic.mapped.Feasible();
      }
      if (!feasible) continue;
      for (auto &dynamic : dyn)
        dynamic.mapped.Run(graph, &table);
      int64_t gain = Gain(graph, subset);
      if (gain > best_gain) {
        best_gain = gain;
        *result = RetrieveResult(subset);
      }
    } while (subset.Next());
    return best_gain;
  }

  int DynSize() const {
    int exp = 0;
    for (auto &d : dyn)
      exp = std::max(exp, Size(d.unmapped.edges));
    return exp;
  }

  int64_t Gain(const Graph &graph, const FastSubset &subset) const {
    int64_t gain = 0;
    for (int i = 0; i < Size(del); ++i)
      gain += graph(subset[i], subset[i] + 1);
    for (auto &edge : add)
      gain -= graph(subset.MapNode(edge.x), subset.MapNode(edge.y));
    for (auto &dynamic : dyn) {
      for (auto &edge : dynamic.mapped.edges)
        gain += edge.Gain(graph);
    }
    return gain;
  }

  FastSubset RetrieveResult(const FastSubset &subset) const {
    FastSubset result(k);
    for (int i = 0; i < Size(del); ++i)
      result[del[i]] = subset[i];
    for (auto &dynamic : dyn) {
      for (int i = 0; i < Size(dynamic.unmapped.edges); ++i)
        result[dynamic.unmapped.edges[i].i] = dynamic.mapped.edges[i].i;
    }
    return result;
  }
};

std::ostream &operator<<(std::ostream &stream, const DynamicData &data) {
  return stream << data.unmapped;
}

std::ostream &operator<<(std::ostream &stream, const DeBergSignature::AddEdge &edge) {
  return stream << '(' << edge.x << ", " << edge.y << ')';
}

std::ostream &operator<<(std::ostream &stream, const DeBergSignature &sig) {
  stream << "k = " << sig.k << '\n';
  stream << "del = " << sig.del << '\n';
  stream << "add = " << sig.add << '\n';
  stream << "dyn = " << sig.dyn << '\n';
  return stream;
}

std::vector<CycleNode> GenericDeBerg(std::vector<DeBergSignature> *signatures,
                                  const Graph &graph,
                                  bool first_better = false,
                                  clock_t deadline = 0) {
  int64_t best_gain = 0;
  FastSubset best_subset;
  Matching best_matching;

  FastSubset subset;
  for (auto &sig : *signatures) {
    if (deadline && clock() > deadline) break;
    int64_t gain = sig.Embed(graph, &subset);
    if (gain > best_gain) {
      best_gain = gain;
      best_subset = subset;
      best_matching = sig.matching;
      if (first_better)
        break;
    }
  }

  if (best_gain > 0) {
    SlowEmbedding best_embedding;
    for (int i = 0; i < best_subset.k; ++i)
      best_embedding.SetVal(SigEdge(i), CycleEdge(best_subset[i]));
    return RetrieveSolution(graph.N(), best_matching, best_embedding);
  } else {
    return IdentityCycle(graph.N());
  }
}

std::vector<DeBergSignature> GenerateDeBergSignatures(int min_k, int max_k) {
  std::vector<DeBergSignature> result;
  for (int k = min_k; k <= max_k; ++k) {
    Matching matching(k);
    while (matching.NextIrreducible()) {
      FastSubset subset;
      result.emplace_back(matching);
    }
  }
  constexpr auto cmp = [](const DeBergSignature &l, const DeBergSignature &r) {
    return l.del.size() < r.del.size();
  };
  std::sort(result.begin(), result.end(), cmp);
  return result;
}

std::vector<CycleNode> LocalDeBerg(int k, const Graph &graph) {
  auto signatures = GenerateDeBergSignatures(k, k);
  return GenericDeBerg(&signatures, graph);
}

static void PrintWeight(int64_t weight) {
  std::cout << clock() << ' ' << weight << '\n';
}

std::vector<CycleNode> GlobalDeBerg(int k, Graph *graph) {
  auto signatures = GenerateDeBergSignatures(2, k);
  int64_t weight = graph->CycleWeight(IdentityCycle(graph->N()));
  PrintWeight(weight);
  while (true) {
    std::vector<CycleNode> solution = GenericDeBerg(&signatures, *graph, true, clock() + 30 * CLOCKS_PER_SEC);
    auto new_weight = graph->CycleWeight(solution);
    if (new_weight < weight) {
      PrintWeight(weight = new_weight);
      graph->ApplyPermutation(solution);
    } else {
      break;
    }
  }
  auto result = graph->GetPermutationIds();
  graph->ResetPermutation();
  return result;
}

int DeBergExponent(MatchingId m) {
  return DeBergSignature(Matching(m)).del.size() + 1;
}

Kmove SingleDeBerg(MatchingId m, const Graph &g) {
  DeBergSignature sig((Matching(m)));
  FastSubset result;
  int64_t gain = sig.Embed(g, &result);
  if (gain > 0) {
    SlowEmbedding e;
    for (int i = 0; i < result.k; ++i)
      e.SetVal(SigEdge(i), CycleEdge(result[i]));
    return Kmove{gain, m, e};
  } else {
    return Kmove{};
  }
}

}  // namespace kopt

