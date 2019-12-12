#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <random>

#include <gflags/gflags.h>
#include "clever_kopt.h"
#include "de_berg.h"
#include "retrieve_solution.h"
#include "naive_kopt.h"
#include "gain_func.h"
#include "slow_embedding.h"
#include "dynamic.h"
#include "new_naive.h"

DEFINE_bool(iterate, false, "iterate k-opt");
DEFINE_int32(k, 0, "the k in k-opt (number of edges in signature)");
DEFINE_int32(min_k, 0, "the minimum value of k");
DEFINE_int32(max_k, 0, "the maximum value of k");
DEFINE_string(input, "", "input file to read (read from stdin if empty)");

DEFINE_string(library, "data/decomposition", "path to decomposition library");

DEFINE_string(algorithm, "", "the algorithm to use");
DEFINE_string(initial_cycle, "", "the initial cycle to use (identity, shuffle or walk");
DEFINE_bool(shuffle_signatures, false, "shuffle signatures with equal cost");
DEFINE_int64(deadline, 0, "maximum running time in seconds for global");
DEFINE_int64(deadline_step, 0, "deadline extension in seconds after each improvement");

enum class Algorithm {
  kClever, kDeberg, kNaive, kHardcoded, kCombined, kExperimental,
};

enum class InitialCycle {
  kIdentity, kShuffle, kWalk,
};

Algorithm GetAlgorithm() {
  if (FLAGS_algorithm == "clever")
    return Algorithm::kClever;
  else if (FLAGS_algorithm == "deberg")
    return Algorithm::kDeberg;
  else if (FLAGS_algorithm == "naive")
    return Algorithm::kNaive;
  else if (FLAGS_algorithm == "hardcoded")
    return Algorithm::kHardcoded;
  else if (FLAGS_algorithm == "combined")
    return Algorithm::kCombined;
  else if (FLAGS_algorithm == "experimental")
    return Algorithm::kExperimental;
  std::cerr << "Invalid flag --algorithm='" << FLAGS_algorithm << "'\n";
  exit(1);
}

InitialCycle GetInitialCycle() {
  if (FLAGS_initial_cycle == "identity")
    return InitialCycle::kIdentity;
  else if (FLAGS_initial_cycle == "shuffle")
    return InitialCycle::kShuffle;
  else if (FLAGS_initial_cycle == "walk")
    return InitialCycle::kWalk;
  std::cerr << "Invalid flag --initial-cycle='" << FLAGS_initial_cycle << "'\n";
  exit(1);
}

std::vector<kopt::CycleNode> Local(int k, const kopt::Graph &graph, const kopt::DecompositionLibrary &library) {
  auto algo = GetAlgorithm();
  if (algo == Algorithm::kClever) {
    return kopt::LocalClever(k, graph, library);
  } else if (algo == Algorithm::kDeberg) {
    return kopt::LocalDeBerg(k, graph);
  } else if (algo == Algorithm::kNaive) {
    return kopt::LocalNaive(k, graph, library);
  } else if (algo == Algorithm::kHardcoded) {
    if (k == 2) {
      return kopt::Naive2opt(graph);
    } else if (k == 3) {
      return kopt::Naive3opt(graph);
    } else {
      std::cerr << "No hardcoded algorithm for k = " << k << '\n';
      std::exit(1);
    }
  } else if (algo == Algorithm::kExperimental) {
    if (k == 3) {
      return kopt::Experimental3opt(graph);
    } else {
      std::cerr << "No experimental algorithm for k = " << k << '\n';
      std::exit(1);
    }
  } else abort();
}

namespace kopt {
namespace {

struct Algo {
  virtual ~Algo() = default;
  virtual Kmove Run(const Graph &) const = 0;
  virtual int K() const { return Len(Sig()) + 1; }
  virtual std::tuple<int, int, int> Cost() const = 0;
  virtual std::string Type() const = 0;
  virtual MatchingId Sig() const = 0;
  bool Improve(Graph *g) {
    Kmove move = Run(*g);
    if (move.gain > 0) {
      g->ApplyPermutation(RetrieveSolution(g->N(), Matching(move.matching_id), move.embedding));
      return true;
    }
    return false;
  }
};

struct FuncAlgo : public Algo {
  using Func = Kmove (*)(const Graph &);

  explicit FuncAlgo(Func f, const std::string &type, int cost, MatchingId sig)
      : f(f), type(type), cost(cost), sig(sig) {}
  int K() const override { return cost; }
  std::tuple<int, int, int> Cost() const override { return {cost, 0, 0}; }
  std::string Type() const override { return type; }
  MatchingId Sig() const override { return sig; }
  Kmove Run(const Graph &g) const override { return f(g); }

  Func f;
  std::string type;
  int cost;
  MatchingId sig;
};

struct NaiveAlgo : public Algo {
  NaiveAlgo(MatchingId id) : k(Len(id) + 1), matching_id(id) {}
  std::string Type() const override { return "naive"; }
  std::tuple<int, int, int> Cost() const override { return {k, 3, 0}; }
  MatchingId Sig() const override { return matching_id; }
  Kmove Run(const Graph &g) const override {
    Matching matching(matching_id);
    GainFunc gain_func(g, matching);
    Embedding embedding(Len(matching_id) + 1, g.N());
    do {
      int64_t gain = gain_func.Join(embedding);
      if (gain > 0)
        return Kmove{gain, matching_id, SlowEmbedding(embedding)};
    } while (embedding.Next());
    return {};
  }

  int k;
  MatchingId matching_id;
};

struct CleverAlgo : public Algo {
  CleverAlgo(MatchingId id, const Decomposition *d, int n)
      : matching_id(id), decomposition(d), tw(decomposition->TreeWidth()), constant(decomposition->Constant(n)) {}
  std::string Type() const override { return "clever"; }
  std::tuple<int, int, int> Cost() const override { return {tw + 1, 2, constant}; }
  MatchingId Sig() const override { return matching_id; }
  Kmove Run(const Graph &g) const override {
    Matching matching(matching_id);
    auto result = decomposition->Dfs(Dynamic(g.N(), GainFunc(g, matching)));
    if (result->table[0] > 0)
      return Kmove{result->table[0], matching_id, RetrieveEmbedding(result, g.N())};
    else
      return Kmove{};
  }

  MatchingId matching_id;
  const Decomposition *decomposition;
  int tw;
  int constant;
};

struct DeBergAlgo : public Algo {
  DeBergAlgo(MatchingId id) : matching_id(id), exp(DeBergExponent(id)) {}
  std::string Type() const override { return "deberg"; }
  std::tuple<int, int, int> Cost() const override { return {exp, 1, 0}; }
  MatchingId Sig() const override { return matching_id; }
  Kmove Run(const Graph &g) const override {
    return SingleDeBerg(matching_id, g);
  }

  MatchingId matching_id;
  int exp;
};

std::unique_ptr<Algo> ChooseAlgo(int n, const Matching &m, const DecompositionLibrary &lib) {
  if (FLAGS_algorithm == "naive") {
    return std::make_unique<NaiveAlgo>(m.Id());
  } else if (FLAGS_algorithm == "clever") {
    return std::make_unique<CleverAlgo>(m.Id(), &lib[DependenceGraph(m)], n);
  } else if (FLAGS_algorithm == "deberg") {
    return std::make_unique<DeBergAlgo>(m.Id());
  } else if (FLAGS_algorithm == "combined") {
    auto clever = std::make_unique<CleverAlgo>(m.Id(), &lib[DependenceGraph(m)], n);
    auto deberg = std::make_unique<DeBergAlgo>(m.Id());
    if (clever->Cost() < deberg->Cost())
      return clever;
    else
      return deberg;
  } else abort();
}

void PrintCost(std::ostream &stream, const std::tuple<int, int, int> &t) {
  stream << '(' << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << ')';
}

std::vector<std::unique_ptr<Algo>> PrepareSignatures(int n, const DecompositionLibrary &library) {
  using Ptr = std::unique_ptr<Algo>;
  std::vector<Ptr> sig;
  sig.emplace_back(new FuncAlgo(&Naive2optBase, "hardcoded", 2, MatchingId{'#', '2'}));
  sig.emplace_back(new FuncAlgo(&Naive3optBase, "hardcoded", 3, MatchingId{'#', '3'}));
  for (int k = 4; k <= 7; ++k) {
    Matching matching(k);
    while (matching.NextIrreducible())
      sig.emplace_back(ChooseAlgo(n, matching, library));
  }
  constexpr auto cmp = [](const Ptr &l, const Ptr &r) {
    return l->Cost() < r->Cost();
  };
  std::sort(sig.begin() + 2, sig.end(), cmp);
  // Shuffle signatures with equal cost.
  if (FLAGS_shuffle_signatures) {
    auto &engine = Rng();
    auto begin = sig.begin(), end = sig.begin() + 1;
    while (begin < sig.end()) {
      while (end < sig.end() && (*begin)->Cost() == (*end)->Cost()) ++end;
      std::shuffle(begin, end, engine);
      begin = end;
      end = begin + 1;
    }
  }
  return sig;
}

void GenerateWalk(Graph *graph) {
  auto &engine = Rng();
  auto rand = [&](int a) { return std::uniform_int_distribution<>(0, a - 1)(engine); };

  int n = graph->N();
  CycleNode at(rand(n) + 1);
  std::vector<bool> visited(n);
  std::vector<CycleNode> cycle{at};
  while (Size(cycle) < n) {
    visited[at.id] = true;
    struct Candidate { int64_t dist; CycleNode v; };
    std::vector<Candidate> candidates;
    for (int i = 0; i < n; ++i) {
      auto dist = graph->GetWeight(at, CycleNode(i));
      if (!visited[i]) candidates.emplace_back(Candidate{dist, CycleNode(i)});
    }
    constexpr auto cmp = [](const Candidate &l, const Candidate &r) { return l.dist < r.dist; };
    std::sort(candidates.begin(), candidates.end(), cmp);
    at = candidates[rand(std::min(Size(candidates), 5))].v;
    cycle.emplace_back(at);
  }

  graph->ApplyPermutation(cycle);
}

void SetInitialCycle(Graph *graph) {
  auto cycle = GetInitialCycle();
  if (cycle == InitialCycle::kIdentity) {
    return;
  } else if (cycle == InitialCycle::kShuffle) {
    graph->RandomShuffle();
  } else if (cycle == InitialCycle::kWalk) {
    GenerateWalk(graph);
  } else abort();
}

void PrintHeader() {
  std::cout << "time,weight,k,method,exponent,signature\n";
}

void PrintStep(int64_t weight, const Algo &algo) {
  std::cout << clock() << ',' << weight;
  std::cout << ',' << algo.K() << ',' << algo.Type() << ',' << std::get<0>(algo.Cost()) << ',' << algo.Sig() << '\n';
}

std::vector<CycleNode> GenericGlobal(Graph *graph, const DecompositionLibrary &library) {
  SetInitialCycle(graph);
  auto signatures = PrepareSignatures(graph->N(), library);
  auto it = signatures.begin();
  PrintHeader();
  clock_t deadline = (FLAGS_deadline ? FLAGS_deadline : FLAGS_deadline_step) * CLOCKS_PER_SEC;
  while (it < signatures.end() && clock() < deadline) {
    if ((*it)->Improve(graph)) {
      PrintStep(graph->CycleWeight(), **it);
      it = signatures.begin();
      deadline = std::max(deadline, clock() + FLAGS_deadline_step * CLOCKS_PER_SEC);
    } else {
      ++it;
    }
  }
  auto result = graph->GetPermutationIds();
  graph->ResetPermutation();
  return result;
}

}  // namespace
}  // namespace kopt

int main(int argc, char **argv) {
  using namespace kopt;
  gflags::SetUsageMessage("k-opt heuristic for TSP");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_k) {
    FLAGS_min_k = FLAGS_k;
    FLAGS_max_k = FLAGS_k;
  }
  if (!FLAGS_iterate && !(2 <= FLAGS_min_k && FLAGS_min_k <= FLAGS_max_k && FLAGS_max_k <= 7)) {
    std::cerr << "The value of k must be in range [2, 7]\n";
    return 1;
  }

  Graph graph;
  if (FLAGS_input.empty()) {
    std::cin >> graph;
  } else {
    std::ifstream input(FLAGS_input);
    if (input) {
      input >> graph;
    } else {
      std::cerr << "Failed to open '" << FLAGS_input << "'\n";
      return 1;
    }
  }

  DecompositionLibrary library;
  for (int i = 2; i <= 7; ++i)
    std::ifstream(FLAGS_library + '/' + std::to_string(i)) >> library;

  std::vector<CycleNode> solution;
  if (FLAGS_iterate) {
    solution = GenericGlobal(&graph, library);
  } else {
    std::vector<Permutation> tours;
    for (int k = FLAGS_min_k; k <= FLAGS_max_k; ++k)
      tours.emplace_back(Permutation(ToInts(Local(k, graph, library))));
    WriteTours(&std::cout, tours);
  }
  return 0;
}
