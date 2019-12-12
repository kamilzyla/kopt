#include <decomposition_library.h>

#include <algorithm>

#include <matching.h>

namespace kopt {

DecompositionLibrary::DecompositionLibrary(int kmove_size, Decomposer decomposer) {
  Matching matching(kmove_size);
  while (matching.Next())
    argument_.emplace_back(matching);
  std::sort(argument_.begin(), argument_.end());
  argument_.erase(std::unique(argument_.begin(), argument_.end()), argument_.end());
  value_.reserve(argument_.size());
  for (auto &graph : argument_)
    value_.emplace_back(decomposer(graph));
}

const Decomposition& DecompositionLibrary::operator[](const DependenceGraph &graph) const {
  return *value_[std::lower_bound(argument_.begin(), argument_.end(), graph) - argument_.begin()];
}

std::istream& operator>>(std::istream &stream, DecompositionLibrary &library) {
  int size;
  stream >> size;
  for (int i = 0; i < size; ++i) {
    library.argument_.emplace_back();
    library.value_.emplace_back();
    stream >> library.argument_.back() >> library.value_.back();
  }
  return stream;
}

//std::ostream& operator<<(std::ostream &stream, const DecompositionLibrary &library) {
//  stream << library.argument_.size() << '\n';
//  for (unsigned i = 0; i < library.argument_.size(); ++i)
//    stream << library.argument_[i] << ' ' << library.value_[i] << '\n';
//  return stream;
//}

}  // namespace kopt
