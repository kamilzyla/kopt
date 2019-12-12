#ifndef KOPT_CLEVER_MONOTONIC_SEQUENCE_H_
#define KOPT_CLEVER_MONOTONIC_SEQUENCE_H_

#include <vector>

namespace kopt {

class Subset {
 public:
  // Creates an empty sequence.
  Subset() = default;
  // Creates a lexicographically smallest, strictly monotonic sequence with values in range [0, max_value).
  Subset(int length, int max_value);

  Subset(const Subset &) = default;
  Subset(Subset &&) = default;
  Subset& operator=(const Subset &) = default;
  Subset& operator=(Subset &&) = default;

  std::vector<int> ToVector() const;

  int Length() const;
  int MaxValue() const;

  int operator[](int pos) const { return x_[pos]; }

  // Changes the sequence into the next in lexicographic order and returns true or changes the sequence into the
  // lexicographically smallest one and returns false if it was already the largest.
  bool Next();

  int Index() const;
  int IndexWithout(int pos) const;

 private:
  int n_, k_;
  std::vector<int> a_, b_, x_;
};

}  // namespace kopt

#endif  // KOPT_CLEVER_MONOTONIC_SEQUENCE_H_
