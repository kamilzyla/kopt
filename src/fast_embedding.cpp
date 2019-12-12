#include <fast_embedding.h>

namespace kopt {

Embedding::Embedding(int domain, int codomain) : Embedding(Set<SigEdge>::Full(domain), codomain) {}

Embedding::Embedding(Set<SigEdge> domain, int codomain)
    : domain_(domain), values_(domain.Size(), codomain) {}

unsigned Embedding::Id() const {
  return values_.Index();
}

Embedding::Restricted Embedding::operator-(SigEdge edge) const {
  return Restricted(values_.IndexWithout(domain_.Index(edge)));
}

unsigned Embedding::IdSize() const {
  return IdSize(domain_, values_.MaxValue());
}

unsigned Embedding::IdSize(Set<SigEdge> domain, int codomain) {
  return Binom(codomain, domain.Size());
}

bool Embedding::Next() {
  return values_.Next();
}

}  // namespace kopt
