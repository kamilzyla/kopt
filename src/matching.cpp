#include <matching.h>

#include <algorithm>
#include <exception>
#include "common.h"

namespace kopt {

Matching::Matching(int pairs) {
  assert(pairs >= 0);
  matching_.resize(2 * pairs);
  p_.resize(pairs - 1);
  o_.resize(pairs - 1);
  for (int i = 0; i < pairs - 1; ++i)
    p_[i] = i;
  UpdateMatching();
}

Matching::Matching(MatchingId id) {
  int len = Len(id);
  matching_.resize(2*len + 2);
  p_.resize(len);
  o_.resize(len);
  for (int i = 0; i < len; ++i) {
    o_[i] = id[i] >= 'a';
    p_[i] = id[i] - (o_[i] ? 'a' : 'A');
  }
  UpdateMatching();
}

Matching::Matching(std::vector<int> permutation) : matching_(std::move(permutation)) {
  for (auto &x : matching_) --x;
}

bool Matching::Reducible() const {
  int k = Size(matching_) / 2;
  for (int i = 0; i < k; ++i) {
    if (matching_[2 * i] == 2 * i + 1) return true;
  }
  return false;
}

bool Matching::Next() {
  int sz = o_.size(), i;
  for (i = sz - 1; i >= 0; --i) {
    o_[i] = 1 - o_[i];
    if (o_[i]) break;
    else if (i + 1 < sz and p_[i] < p_[i + 1]) {
      int low = sz - 1;
      while (p_[i] >= p_[low]) --low;
      std::swap(p_[i], p_[low]);
      break;
    }
  }
  std::reverse(p_.begin() + i + 1, p_.end());
  UpdateMatching();
  return i >= 0;
}

bool Matching::NextIrreducible() {
  while (Next()) {
    if (!Reducible()) return true;
  }
  return false;
}

void Matching::UpdateMatching() {
  int k = Size(matching_) / 2;
  for (int i = 0; i < k; ++i) {
    int a = i == 0 ? 0 : 2*p_[i-1] + 2 - o_[i-1];
    int b = i == k-1 ? 2*k - 1 : 2*p_[i] + 1 + o_[i];
    matching_[a] = b;
    matching_[b] = a;
  }
}

std::ostream& operator<<(std::ostream &stream, const Matching &matching) {
  bool first = true;
  stream << '{';
  for (int x : matching.matching_) {
    if (x < matching.matching_[x]) {
      stream << (first ? "{" : ", {") << x + 1 << ", " << matching.matching_[x] + 1 << '}';
      first = false;
    }
  }
  stream << '}';
  return stream;
}

MatchingId Matching::Id() const {
  int len = Size(p_);
  MatchingId id{};
  // Breaks if len > 26 or len > id.size().
  for (int i = 0; i < len; ++i)
    id[i] = (o_[i] ? 'a' : 'A') + p_[i];
  return id;
}

}  // namespace kopt
