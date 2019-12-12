#ifndef KOPT_COMMON_IDENTIFIER_H
#define KOPT_COMMON_IDENTIFIER_H

#include <initializer_list>
#include <iostream>
#include <vector>

namespace kopt {

// A CRTP base class to wrap a signed integer in a meaningful type for use as an ID.
template<class T>
class Identifier {
 public:
  int id;

  explicit Identifier(int id = -1) : id(id) {}
  explicit operator int() { return id; }

  T Step(int count, int n) const {
    int now = id + count;
    if (now < 0) now += n;
    else if (now >= n) now -= n;
    return T(now);
  }

  friend std::istream& operator>>(std::istream &stream, T &id) { stream >> id.id; --id.id; return stream; }
  friend std::ostream& operator<<(std::ostream &stream, T id) { return stream << id.id + 1; }

  friend bool operator!=(T x, T y) { return x.id != y.id; }
  friend bool operator==(T x, T y) { return x.id == y.id; }
};

class CycleNode;
class CycleEdge;
class SigNode;
class SigEdge;

// A number in [1, n] range - the index of a node on the Hamiltonian cycle.
// The actual int value can be n+1 and shall be interpreted as 1 in that case.
class CycleNode : public Identifier<CycleNode> {
 public:
  using Identifier::Identifier;
};

// A number in [1, n] range - the index of an edge on the Hamiltonian cycle.
class CycleEdge : public Identifier<CycleEdge> {
 public:
  using Identifier::Identifier;

  CycleNode Left() const;
  CycleNode Right() const;
};

// A number in [1, 2k] range - the index of an endpoint of a modified edge.
class SigNode : public Identifier<SigNode> {
 public:
  using Identifier::Identifier;

  bool IsLeft() const;
//  bool IsRight() const;
  SigEdge Edge() const;
};

// A number in [1, k] range - the index of a modified edge.
class SigEdge : public Identifier<SigEdge> {
 public:
  using Identifier::Identifier;

  SigNode Left() const;
  SigNode Right() const;
};

// An implicitly convertible convenience class for ID literals.
//class Id {
// public:
//  explicit Id(int id);
//  template<class I> operator I() const;
//
// private:
//  const int id;
//};

// An implicitly convertible convenience class for ID sequence literals.
//class Ids {
// public:
//  explicit Ids(std::initializer_list<int> ids);
//  template<class I> operator std::vector<I>() const;
//
// private:
//  const std::initializer_list<int> ids_;
//};

// Implementation
// =====================================================================================================================

inline CycleNode CycleEdge::Left() const { return CycleNode(id); }
inline CycleNode CycleEdge::Right() const { return CycleNode(id + 1); }
//
inline bool SigNode::IsLeft() const { return id % 2 == 0; }
//inline bool SigNode::IsRight() const { return Id() % 2 == 0; }
inline SigEdge SigNode::Edge() const { return SigEdge(id / 2); }
//
inline SigNode SigEdge::Left() const { return SigNode(2 * id); }
inline SigNode SigEdge::Right() const { return SigNode(2 * id + 1); }

//inline Id::Id(int id) : id(id) {}
//template<class I> Id::operator I() const {
//  static_assert(std::is_base_of<Identifier<I>, I>()());
//  return I(id);
//}

//inline Ids::Ids(std::initializer_list<int> ids) : ids_(ids) {}
//template<class I> Ids::operator std::vector<I>() const {
//  static_assert(std::is_base_of<Identifier<I>, I>()());
//  return std::vector<I>(ids_.begin(), ids_.end());
//}

}  // namespace kopt

#endif  // KOPT_COMMON_IDENTIFIER_H
