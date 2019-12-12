#ifndef KOPT_COMMON_NAIVE_KOPT_H_
#define KOPT_COMMON_NAIVE_KOPT_H_

#include <vector>

#include <graph.h>
#include <identifier.h>
#include "slow_embedding.h"

namespace kopt {

std::vector<CycleNode> NaiveKopt(int k, const Graph &);
std::vector<CycleNode> Naive2opt(const Graph &);
std::vector<CycleNode> Naive3opt(const Graph &);
std::vector<CycleNode> Experimental3opt(const Graph &g);

Kmove Naive2optBase(const Graph &);
Kmove Naive3optBase(const Graph &);

}  // namespace kopt

#endif  // KOPT_COMMON_NAIVE_KOPT_H_
