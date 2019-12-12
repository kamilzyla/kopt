#include <monotonic_sequence.h>

#include <algorithm>
#include <cassert>
#include <limits>

namespace kopt {

int Binom(int n, int k) {
  static int k_size = 1, n_size = 1;
  static std::vector<int> cache{1};
  static auto binom = [&](int n, int k) -> int& { return cache[k*n_size + n]; };

  if (k >= k_size || n >= n_size) {
    while (k >= k_size) k_size *= 2;
    while (n >= n_size) n_size *= 2;
    cache.clear();
    cache.resize(k_size * n_size);
    std::fill(cache.begin(), cache.begin() + n_size, 1);
    for (int i = 1; i < k_size; ++i) for (int j = 1; j < n_size; ++j) {
      binom(j, i) = binom(j-1, i-1) + binom(j-1, i);
    }
  }

  return binom(n, k);
}

Subset::Subset(int length, int max_value)
    : n_(max_value), k_(length), a_(k_+1), b_(k_+1), x_(k_+1) {
  assert(0 <= k_ && k_ <= n_);
  for (int i = 0; i < k_; ++i) {
    b_[i] = k_ - i;
    x_[i] = i;
  }
  x_[k_] = n_;
}

std::vector<int> Subset::ToVector() const {
  return std::vector<int>(x_.begin(), x_.begin() + k_);
}

int Subset::Length() const {
  return k_;
}

int Subset::MaxValue() const {
  return n_;
}

bool Subset::Next() {
  int i = 0;
  while (i < k_ && x_[i]+1 >= x_[i+1]) ++i;
  if (i < k_) {
    a_[i] += Binom(x_[i], i);
    b_[i] += i > 0 ? Binom(x_[i], i-1) : 0;
    x_[i] += 1;
  }
  while (--i >= 0) {
    a_[i] = a_[i+1];
    b_[i] = b_[i+1] + 1;
    x_[i] = i;
  }
  return x_[k_-1] >= k_;
}

int Subset::Index() const {
  return a_[0];
}

int Subset::IndexWithout(int pos) const {
  return a_[0] - a_[pos] + b_[pos + 1];
}

}  // namespace kopt
