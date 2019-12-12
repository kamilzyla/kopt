#include <embedding.h>

namespace kopt {

std::ostream& operator<<(std::ostream &stream, const EmbeddingInterface &embedding) {
  auto domain = embedding.Domain();
  bool first = true;
  stream << '{';
  for (auto arg : domain) {
    stream << (first ? "" : ", ") << arg;
    first = false;
  }
  stream << "} -> {";
  first = true;
  for (auto arg : domain) {
    stream << (first ? "" : ", ") << embedding(arg);
    first = false;
  }
  stream << '}';
  return stream;
}

}
