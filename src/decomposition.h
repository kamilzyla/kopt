#ifndef KOPT_COMMON_DECOMPOSITION_H_
#define KOPT_COMMON_DECOMPOSITION_H_

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include <common.h>
#include <identifier.h>
#include <set.h>
#include <cmath>

namespace kopt {

class Decomposition {
 public:
  using Ptr = std::unique_ptr<Decomposition>;

  int TreeWidth() const;
  int Constant(int n) const;
  std::string BagSizes() const;

  static Ptr Leaf();
  static Ptr Introduce(SigEdge introduced, Ptr child);
  static Ptr Forget(SigEdge forgotten, Ptr child);
  static Ptr Join(Ptr left, Ptr right);

  template<class Visitor>
  typename Visitor::Result Dfs(const Visitor &visitor) const;

  friend std::istream& operator>>(std::istream &, Ptr &);
//  friend std::ostream& operator<<(std::ostream &, const Ptr &);

 private:
  enum class Type { kLeaf, kIntroduce, kForget, kJoin };

  Type type_;
  SigEdge edge_;
  Ptr left_, right_;

  explicit Decomposition(Type);
  Decomposition(Type, SigEdge, Ptr, Ptr);
};

// Implementation
// =====================================================================================================================

template<class Visitor>
typename Visitor::Result Decomposition::Dfs(const Visitor &visitor) const {
  switch (type_) {
    case Type::kLeaf:
      return visitor.Leaf();
    case Type::kIntroduce:
      return visitor.Introduce(edge_, left_->Dfs(visitor));
    case Type::kForget:
      return visitor.Forget(edge_, left_->Dfs(visitor));
    case Type::kJoin:
      return visitor.Join(left_->Dfs(visitor), right_->Dfs(visitor));
    default:
      abort();
  }
}

struct TreeWidthVisitor {
  struct Result {
    int now, max;
  };

  Result Leaf() const { return {0, 0}; }
  Result Introduce(SigEdge, Result child) const { return {child.now + 1, std::max(child.max, child.now + 1)}; }
  Result Forget(SigEdge, Result child) const { return {child.now - 1, child.max}; }
  Result Join(Result left, Result) const { return left; }
};


struct BagVisitor {
  struct Result {
    int now;
    std::vector<int> bags;
  };

  Result Leaf() const { return {0, {1}}; }
  Result Introduce(SigEdge, Result child) const {
    if (++child.now < Size(child.bags)) ++child.bags[child.now];
    else child.bags.emplace_back(1);
    return child;
  }
  Result Forget(SigEdge, Result child) const {
    ++child.bags[--child.now];
    return child;
  }
  Result Join(Result left, Result right) const {
    if (Size(left.bags) < Size(right.bags)) left.bags.swap(right.bags);
    for (int i = 0; i < Size(right.bags); ++i) left.bags[i] += right.bags[i];
    return left;
  }
};

static constexpr double kIntroduce[5] = {4.36e-02, 1.17e-02, 7.06e-03, 1.70e-03, 5.35e-04};
static constexpr double kForget[5] = {2.46e-02, 5.54e-03, 1.64e-03, 3.88e-04, 9.67e-05};
static constexpr double kJoin[5] = {3.41e-02, 3.41e-02, 3.41e-02, 3.41e-02, 3.41e-02};
struct ComplexityVisitor {
  struct Result {
    long now, sum;
  };

  explicit ComplexityVisitor(int n) : n(n) {}

  Result Leaf() const { return {0, 0}; }
  Result Introduce(SigEdge, Result result) const {
    ++result.now;
    result.sum += lround(kIntroduce[result.now - 1] * pow(double(n), double(result.now)));
    return result;
  }
  Result Forget(SigEdge, Result result) const {
    --result.now;
    result.sum += lround(kForget[result.now] * pow(double(n), double(result.now + 1)));
    return result;
  }
  Result Join(Result left, Result right) const {
    left.sum += lround(kJoin[left.now - 1] * pow(double(n), double(left.now))) + right.sum;
    return left;
  }

  int n;
};

inline int Decomposition::TreeWidth() const {
  return Dfs(TreeWidthVisitor()).max - 1;
}

inline int Decomposition::Constant(int n) const {
  return Dfs(ComplexityVisitor(n)).sum;
}

inline std::string Decomposition::BagSizes() const {
  std::vector<int> bags = Dfs(BagVisitor()).bags;
  std::ostringstream result;
  for (int i = 0; i < Size(bags); ++i)
    result << (i ? " " : "") << bags[i];
  return result.str();
}

}  // namespace kopt

#endif  // KOPT_COMMON_DECOMPOSITION_H_
