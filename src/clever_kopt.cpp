#include <clever_kopt.h>

#include <iostream>

#include <dynamic.h>
#include <matching.h>
#include <retrieve_solution.h>
#include <naive_kopt.h>
#include <algorithm>

namespace kopt {

struct Sig {
  long cost;
  MatchingId id;
  const Decomposition *decomposition;

  bool operator<(const Sig &right) const { return cost < right.cost; }
};

static std::vector<Sig> Signatures(int n, const DecompositionLibrary &library, int min_k, int max_k, int max_tw = 0) {
  if (!max_tw)
    max_tw = max_k - 1;
  std::vector<Sig> result;
  for (int k = min_k; k <= max_k; ++k) {
    Matching matching(k);
    while (matching.NextIrreducible()) {
      auto &decomposition = library[DependenceGraph(matching)];
      long cost = decomposition.Dfs(ComplexityVisitor{n}).sum;
      result.emplace_back(Sig{cost, matching.Id(), &decomposition});
    }
  }
  std::sort(result.begin(), result.end());
  return result;
}

static std::vector<CycleNode> Kopt(
    const Graph &graph, const std::vector<Sig> &signatures, bool dynamic, bool first_better, clock_t deadline) {
  int64_t best_gain = 0;
  Matching best_matching;
  std::unique_ptr<EmbeddingInterface> best_embedding;
  for (auto &sig : signatures) {
    if (deadline && clock() >= deadline) break;
    Matching matching(sig.id);
    GainFunc gain_func(graph, matching);
    if (dynamic) {
      auto result = sig.decomposition->Dfs(Dynamic(graph.N(), gain_func));
      if (result->table[0] > best_gain) {
        best_gain = result->table[0];
        best_matching = matching;
        best_embedding = std::make_unique<SlowEmbedding>(RetrieveEmbedding(result, graph.N()));
        if (first_better) break;
      }
    } else {
      Embedding embedding(Len(sig.id) + 1, graph.N());
      do {
        // Hacky, but fast.
        int64_t gain = 0;
        int k = embedding.Domain().Size();
        for (int i = 0; i < k; ++i) gain += graph.GetWeight(embedding(SigEdge(i)));
        for (int i = 0; i < 2*k; ++i) {
          SigNode x(i), y = matching(x);
          if (x.id < y.id) gain -= graph.GetWeight(embedding.FastMapNode(x.id), embedding.FastMapNode(y.id));
        }
        // End hack.
        if (gain > best_gain) {
          best_gain = gain;
          best_matching = matching;
          best_embedding = std::make_unique<Embedding>(embedding);
          if (first_better) break;
        }
      } while (embedding.Next());
    }
  }
  if (best_gain > 0)
    return RetrieveSolution(graph.N(), best_matching, *best_embedding);
  else
    return IdentityCycle(graph.N());
}

std::vector<CycleNode> LocalClever(int k, const Graph &graph, const DecompositionLibrary &library) {
  return Kopt(graph, Signatures(graph.N(), library, k, k), true, false, 0);
}

std::vector<CycleNode> LocalNaive(int k, const Graph &graph, const DecompositionLibrary &library) {
  return Kopt(graph, Signatures(graph.N(), library, k, k), false, false, 0);
}

static void PrintWeight(int64_t weight) {
  std::cout << clock() << ' ' << weight << '\n';
}

std::vector<CycleNode> Global(Graph *graph, const DecompositionLibrary &library) {
  auto signatures = Signatures(graph->N(), library, 4, 7, 2);
  int64_t weight = graph->CycleWeight(IdentityCycle(graph->N()));
  PrintWeight(weight);

  clock_t deadline = clock() + 30 * CLOCKS_PER_SEC;
  int k = 2;
  while (k <= 7) {
    std::vector<CycleNode> solution;
    if (k == 2) solution = Naive2opt(*graph);
    else if (k == 3) solution = Naive3opt(*graph);
    else solution = Kopt(*graph, signatures, true, true, deadline);
    auto new_weight = graph->CycleWeight(solution);
    if (new_weight < weight) {
      PrintWeight(weight = new_weight);
      graph->ApplyPermutation(solution);
      deadline = clock() + 30 * CLOCKS_PER_SEC;
      k = 2;
    } else {
      ++k;
    }
  }
  auto result = graph->GetPermutationIds();
  graph->ResetPermutation();
  return result;
}

}  // namespace kopt
