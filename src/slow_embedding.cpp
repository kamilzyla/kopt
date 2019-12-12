#include <slow_embedding.h>

namespace kopt {

SlowEmbedding::SlowEmbedding(int codomain) : codomain_(codomain) {}

SlowEmbedding::SlowEmbedding(const kopt::Embedding &e) : codomain_(e.Codomain()) {
  for (auto arg : e.Domain())
    SetVal(arg, e(arg));
}

Set<SigEdge> SlowEmbedding::Domain() const {
  return domain_;
}

int SlowEmbedding::Codomain() const {
  return codomain_;
}

int SlowEmbedding::Index() const {
  int result = 0;
  for (int i = domain_.Size(); i > 0; --i)
    result += Binom(values_[i-1].id, i);
  return result;
}

void SlowEmbedding::SetVal(SigEdge arg, CycleEdge val) {
  if (domain_.Contains(arg)) {
    values_[domain_.Index(arg)] = val;
  } else {
    domain_ += Set<SigEdge>(arg);
    values_.insert(values_.begin() + domain_.Index(arg), val);
  }
}

void SlowEmbedding::Remove(SigEdge arg) {
  values_.erase(values_.begin() + domain_.Index(arg));
  domain_ -= Set<SigEdge>(arg);
}

CycleEdge SlowEmbedding::MapEdge(SigEdge edge) const {
  return values_[domain_.Index(edge)];
}

}  // namespace kopt
