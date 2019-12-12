#include <dependence_graph.h>

namespace kopt {

static std::istream& operator>>(std::istream &stream, std::tuple<int, int> &edge) {
  return stream >> std::get<0>(edge) >> std::get<1>(edge);
}

//static std::ostream& operator<<(std::ostream &stream, std::tuple<int, int> &edge) {
//  return stream << std::get<0>(edge) << ' ' << std::get<1>(edge);
//}

DependenceGraph::DependenceGraph(const Matching &matching) {
  for (SigNode node : matching.Domain()) {
    // The nodes of the dependence graph are the modified edges.
    int x = node.Edge().id + 1;
    int y = matching(node).Edge().id + 1;
    // Only push edges which do not duplicate the already added ones.
    // All edges (i, i+1) for i = 1, ..., n-1 are implicitly included in the graph, thus require y - x >= 2.
    if (y - x >= 2 && (edges_.empty() || edges_.back() != Edge(x, y)))
      edges_.emplace_back(x, y);
  }
  node_count_ = matching.Domain().Size() / 2;
  edge_count_ = static_cast<int>(edges_.size());
}

//void DependenceGraph::DimacsFormat(std::ostream *stream) const {
//  *stream << "p tw " << node_count_ << ' ' << node_count_ - 1 + edges_.size() << '\n';
//  for (int i = 1; i < node_count_; ++i)
//    *stream << i << ' ' << i + 1 << '\n';
//  for (auto e : edges_)
//    *stream << e << '\n';
//}

std::istream& operator>>(std::istream &stream, DependenceGraph &graph) {
  stream >> graph.node_count_ >> graph.edge_count_;
  assert(graph.node_count_ >= 0 && graph.edge_count_ >= 0);
  graph.edges_.resize(static_cast<unsigned>(graph.edge_count_));
  for (int i = 0; i < graph.edge_count_; ++i)
    stream >> graph.edges_[i];
  return stream;
}

//std::ostream& operator<<(std::ostream &stream, const DependenceGraph &graph) {
//  stream << graph.node_count_ << ' ' << graph.edges_.size();
//  for (auto e : graph.edges_)
//    stream << ' ' << e;
//  return stream;
//}

}  // namespace kopt
