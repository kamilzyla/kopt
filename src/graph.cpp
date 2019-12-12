#include "graph.h"

#include <functional>
#include <random>
#include <sstream>

#include "common.h"

namespace kopt {

Graph Graph::Random(int n) {
  assert(n >= 0);
  if (n == 0) return Graph();
  Graph g(n);
  std::uniform_int_distribution<> dist(1, n);
  auto rand = std::bind(dist, Rng());
  for (int i = 0; i < n; ++i) {
    g.points[i].x = rand();
    g.points[i].y = rand();
  }
  g.points[n] = g.points[0];
  g.CheckInvariant();
  return g;
}

Graph::Graph(int n) : n(n) {
  assert(n >= 0);
  if (n) {
    points.resize(n + 1);
    perm = Permutation(n);
  }
  CheckInvariant();
}

void Graph::Permutate(const Permutation &p) {
  assert(p.N() == n);
  if (n) {
    std::vector<Point> new_points(n + 1);
    for (int i = 0; i < n; ++i)
      new_points[i] = points[p[i]];
    new_points[n] = new_points[0];
    points.swap(new_points);
    perm = Compose(p, perm);
  }
  CheckInvariant();
}

const Permutation &Graph::GetPermutation() const {
  return perm;
}

std::istream& operator>>(std::istream &stream, Graph &graph) {
  std::vector<Point> points;
  std::string line;
  bool coord_section = false;
  while (std::getline(stream, line)) {
    if (line == "NODE_COORD_SECTION") {
      coord_section = true;
    } else if (line == "EOF") {
      coord_section = false;
    } else if (coord_section) {
      std::istringstream ss(line);
      int id;
      double x, y;
      ss >> id >> x >> y;
      points.emplace_back(Point{x, y});
    }
  }
  points.emplace_back(points[0]);
  graph.n = Size(points) - 1;
  graph.points.swap(points);
  graph.perm = Permutation(graph.n);
  graph.CheckInvariant();
  return stream;
}

void Graph::CheckInvariant() const {
  assert(n >= 0);
  if (n == 0) {
    assert(Size(points) == 0);
  } else {
    assert(Size(points) == n + 1);
    assert(points[0].x == points[n].x && points[0].y == points[n].y);
  }
  assert(perm.N() == n);
}

Weight CycleWeight(const Graph &graph, const Permutation &cycle) {
  assert(graph.N() == cycle.N());
  int n = cycle.N();
  Weight weight = graph(cycle[n - 1], cycle[0]);
  for (int i = 1; i < n; ++i)
    weight += graph(cycle[i - 1], cycle[i]);
  return weight;
}

void WriteGraph(std::ostream *out, const Graph &graph, const std::string &name) {
  if (!name.empty())
    *out << "NAME : " << name << '\n';
  *out << "TYPE : TSP\n";
  *out << "DIMENSION : " << graph.N() << '\n';
  *out << "EDGE_WEIGHT_TYPE : EUC_2D\n";
  *out << "NODE_COORD_SECTION\n";
  for (int i = 0; i < graph.N(); ++i)
    *out << i + 1 << ' ' << graph[i].x << ' ' << graph[i].y << '\n';
  *out << "EOF\n";
}

void WriteTours(std::ostream *out, const std::vector<Permutation> &tours, const std::string &name, int dimension) {
  if (!name.empty())
    *out << "NAME : " << name << '\n';
  *out << "TYPE : TOUR\n";
  if (dimension >= 0)
    *out << "DIMENSION : " << dimension << '\n';
  *out << "TOUR_SECTION\n";
  for (auto &tour : tours) {
    for (int i = 0; i < tour.N(); ++i)
      *out << tour[i] + 1 << ' ';
    *out << "-1\n";
  }
  *out << "EOF\n";
}

}  // namespace kopt
