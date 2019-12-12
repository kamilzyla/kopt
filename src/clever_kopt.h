#ifndef KOPT_CLEVER_CLEVER_KOPT_H_
#define KOPT_CLEVER_CLEVER_KOPT_H_

#include <vector>

#include <decomposition_library.h>
#include <graph.h>
#include <identifier.h>

namespace kopt {

std::vector<CycleNode> LocalClever(int k, const Graph &, const DecompositionLibrary &);
std::vector<CycleNode> LocalNaive(int k, const Graph &, const DecompositionLibrary &);
std::vector<CycleNode> Global(Graph *, const DecompositionLibrary &);

}  // namespace kopt

#endif  // KOPT_CLEVER_CLEVER_KOPT_H_
