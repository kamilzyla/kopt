#include <gflags/gflags.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "common.h"
#include "graph.h"

DEFINE_string(name, "", "name of the test");
DEFINE_string(tsp_file, "", "path for the .tsp file (output to stdout if empty)");
DEFINE_string(tour_file, "", "path for the .tour file (not generated if empty)");
DEFINE_int32(n, 0, "number of vertices");

namespace {

class FileOrDefault {
 public:
  explicit FileOrDefault(const std::string &path, std::ostream *def = nullptr) : ptr(def) {
    if (!path.empty()) {
      file.open(path);
      ptr = &file;
    }
  }

  std::ostream *Ptr() { return ptr; }

 private:
  std::ofstream file;
  std::ostream *ptr;
};

void Ensure(bool what, const char *str = nullptr) {
  if (!what) {
    if (str) std::cerr << str << '\n';
    std::exit(1);
  }
}

}  // namespace

namespace kopt {
namespace {

using Tours = std::vector<Permutation>;

Tours BestTours(const Graph &graph) {
  int n = graph.N();
  Tours best_tour(n - 1);
  std::vector<Weight> best_weight(n - 1, kMaxWeight);

  Permutation tour(n);
  // Skip the first (identity) tour.
  while (tour.Next(1)) {
    // Only consider canonical tours.
    if (tour[1] > tour[n - 1])
      continue;
    int idx = Changes(tour) - 2;
    Weight weight = CycleWeight(graph, tour);
    if (weight < best_weight[idx]) {
      best_tour[idx] = tour;
      best_weight[idx] = weight;
    }
  }
  return best_tour;
}

void Work(std::ostream *tsp_out, std::ostream *tour_out) {
  auto graph = Graph::Random(FLAGS_n);
  WriteGraph(tsp_out, graph, FLAGS_name);
  if (tour_out) {
    auto tours = BestTours(graph);
    WriteTours(tour_out, tours, FLAGS_name, graph.N());
  }
}

}  // namespace
}  // namespace kopt

int main(int argc, char **argv) {
  gflags::SetUsageMessage("Generate a validation test");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  Ensure(FLAGS_n > 0, "Parameter n must be positive.");
  if (!FLAGS_tour_file.empty())
    Ensure( 2 <= FLAGS_n && FLAGS_n <= 13, "Tours can only be generated for n in [2, 13].");
  FileOrDefault tsp_out(FLAGS_tsp_file, &std::cout);
  FileOrDefault tour_out(FLAGS_tour_file);
  kopt::Work(tsp_out.Ptr(), tour_out.Ptr());
  return 0;
}
