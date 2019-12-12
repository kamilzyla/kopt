#include <retrieve_solution.h>

namespace kopt {

std::vector<CycleNode> RetrieveSolution(int graph_size, const Matching &matching, const EmbeddingInterface &embedding) {
  std::vector<CycleNode> result;
  result.reserve(graph_size);
  SigNode mod_pos(0);
  do {
    // Step along a new edge.
    result.emplace_back(embedding(mod_pos));
    mod_pos = matching(mod_pos);
    // Step along the old edges.
    int step = mod_pos.IsLeft() ? -1 : 1;
    CycleNode cycle_pos = embedding(mod_pos).Step(0, graph_size);
    mod_pos = mod_pos.Step(step, matching.Domain().Size());
    CycleNode cycle_target = embedding(mod_pos);
    while (cycle_pos != cycle_target) {
      result.emplace_back(cycle_pos);
      cycle_pos = cycle_pos.Step(step, graph_size);
    }
  } while (mod_pos != SigNode(0));
  return result;
}

void print_canonical(std::ostream &stream, const std::vector<kopt::CycleNode> &solution) {
  int n = solution.size();
  auto get = [&](int idx) {
    return solution[idx < 0 ? idx + n : (idx < n ? idx : idx - n)].id;
  };
  int at = 0;
  while (get(at)) ++at;
  int step = get(at + 1) < get(at - 1) ? 1 : -1;
  for (int i = 0; i < n; ++i, at += step) {
    stream << get(at) << (i + 1 < n ? ' ' : '\n');
  }
}

}  // namespace kopt
