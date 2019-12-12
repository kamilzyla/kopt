#ifndef KOPT_RETRIEVE_SOLUTION_H_
#define KOPT_RETRIEVE_SOLUTION_H_

#include <vector>

#include <embedding.h>
#include <identifier.h>
#include <matching.h>

namespace kopt {

std::vector<CycleNode> RetrieveSolution(int graph_size, const Matching &, const EmbeddingInterface &);
void print_canonical(std::ostream &stream, const std::vector<kopt::CycleNode> &solution);

}  // namespace kopt

#endif  // KOPT_RETRIEVE_SOLUTION_H_
