#ifndef KOPT_COMMON_SET_H_
#define KOPT_COMMON_SET_H_

#include <cassert>
#include <ostream>
#include <vector>

#include <identifier.h>

namespace kopt {

// A fast bit-set fitting in a single machine word.
// The type T must be a zero-based identifier (as SigNode or SigEdge).
template<class T>
class Set {
 private:
  class IteratorEnd {
   private:
    IteratorEnd() = default;
    friend class Set;
  };

  class Iterator {
   public:
    T operator*() const;
    Iterator &operator++();
    bool operator!=(IteratorEnd);

   private:
    unsigned mask_;
    explicit Iterator(unsigned);
    friend class Set;
  };

 public:
  // Creates an empty set.
  Set() = default;
  // Creates a set with one element.
  explicit Set(T x);
  // Creates a set with elements from the given vector.
  explicit Set(const std::vector<int> &);
  // Creates a set with elements 1, 2, ..., size.
  static Set Full(int size);

  // Returns the bit-set representation.
  explicit operator unsigned() const;

  // Returns the number of elements.
  int Size() const;
  // Returns the number of elements smaller than x.
  int Index(T x) const;
  // Returns true iff the set contains x.
  bool Contains(T x) const;
  // Returns the nth smallest element (indexed starting with 1).
  T Nth(int n) const;

  Set& operator+=(Set);
  Set& operator-=(Set);

  // The class supports range-based for loop.
  Iterator begin() const;
  IteratorEnd end() const;

 private:
  unsigned mask_{};

  explicit Set(unsigned);
  static unsigned Mask(T);
};

template<class T> Set<T> operator+(Set<T>, Set<T>);
template<class T> Set<T> operator-(Set<T>, Set<T>);

template<class T> bool operator==(Set<T>, Set<T>);
template<class T> bool operator!=(Set<T>, Set<T>);

template<class T> std::ostream& operator<<(std::ostream &, const Set<T> &);

Set<SigNode> NodeSet(Set<SigEdge> edges);

// Implementation
// =====================================================================================================================

template<class T>
T Set<T>::Iterator::operator*() const {
  return T(__builtin_ffs(mask_) - 1);
}

template<class T>
typename Set<T>::Iterator& Set<T>::Iterator::operator++() {
  mask_ ^= mask_ & -mask_;
  return *this;
}

template<class T>
Set<T>::Iterator::Iterator(unsigned mask) : mask_(mask) {}

template<class T>
bool Set<T>::Iterator::operator!=(typename Set<T>::IteratorEnd) {
  return mask_ != 0;
}

template<class T>
Set<T>::Set(T x) : mask_(Mask(x)) {}

template<class T>
Set<T>::Set(const std::vector<int> &vec) {
  for (auto elem : vec)
    mask_ |= Mask(T(elem));
}

template<class T>
Set<T> Set<T>::Full(int size) {
  return Set((1u << size) - 1);
}

template<class T>
Set<T>::operator unsigned() const {
  return mask_;
}

template<class T>
int Set<T>::Size() const {
  return __builtin_popcount(mask_);
}

template<class T>
int Set<T>::Index(T x) const {
  return __builtin_popcount(mask_ & (Mask(x) - 1));
}

template<class T>
bool Set<T>::Contains(T x) const {
  return (mask_ & Mask(x)) != 0;
}

template<class T>
T Set<T>::Nth(int n) const {
  assert(1 <= n && n <= Size());
  auto it = begin();
  while (--n) ++it;
  return *it;
}

template<class T>
Set<T>& Set<T>::operator+=(Set set) {
  mask_ |= set.mask_; return *this;
}

template<class T>
Set<T>& Set<T>::operator-=(Set set) {
  mask_ &= ~set.mask_; return *this;
}

template<class T>
typename Set<T>::Iterator Set<T>::begin() const {
  return Iterator(mask_);
}

template<class T>
typename Set<T>::IteratorEnd Set<T>::end() const {
  return IteratorEnd();
}

template<class T>
Set<T>::Set(unsigned mask) : mask_(mask) {}

template<class T>
unsigned Set<T>::Mask(T x) {
  return 1 << x.id;
}

template<class T>
Set<T> operator+(Set<T> lhs, Set<T> rhs) {
  return Set<T>(lhs) += rhs;
}

template<class T>
Set<T> operator-(Set<T> lhs, Set<T> rhs) {
  return Set<T>(lhs) -= rhs;
}

template<class T>
bool operator==(Set<T> lhs, Set<T> rhs) {
  return static_cast<unsigned>(lhs) == static_cast<unsigned>(rhs);
}

template<class T>
bool operator!=(Set<T> lhs, Set<T> rhs) {
  return static_cast<unsigned>(lhs) != static_cast<unsigned>(rhs);
}

template<class T>
std::ostream& operator<<(std::ostream &stream, const Set<T> &set) {
  stream << '{';
  bool first = true;
  for (auto val : set) {
    stream << (first ? "" : ", ") << val;
    first = false;
  }
  stream << '}';
  return stream;
}

//inline Set<SigNode> NodeSet(Set<SigEdge> edges) {
//  Set<SigNode> result;
//  for (SigEdge edge : edges) {
//    result += Set<SigNode>(edge.Left());
//    result += Set<SigNode>(edge.Right());
//  }
//  return result;
//}

}  // namespace kopt

#endif  // KOPT_COMMON_SET_H_
