#ifndef KOPT_DE_BERG_H_
#define KOPT_DE_BERG_H_

#include <vector>

#include <graph.h>
#include <identifier.h>
#include "slow_embedding.h"

namespace kopt {

int DeBergExponent(MatchingId);
Kmove SingleDeBerg(MatchingId, const Graph &);

std::vector<CycleNode> LocalDeBerg(int k, const Graph &);
std::vector<CycleNode> GlobalDeBerg(int k, Graph *);

}  // namespace kopt

#endif  // KOPT_DE_BERG_H_
