#include <decomposition.h>

#include <cassert>
#include <limits>
#include <sstream>

namespace kopt {

Decomposition::Decomposition(Type type) : type_(type) {}

Decomposition::Decomposition(Type type, SigEdge edge, Ptr left, Ptr right)
    : type_(type), edge_(edge), left_(std::move(left)), right_(std::move(right)) {}

Decomposition::Ptr Decomposition::Leaf() {
  return Ptr(new Decomposition(Type::kLeaf));
}

Decomposition::Ptr Decomposition::Introduce(SigEdge introduced, Ptr child) {
  return Ptr(new Decomposition(Type::kIntroduce, introduced, std::move(child), Ptr()));
}

Decomposition::Ptr Decomposition::Forget(SigEdge forgotten, Ptr child) {
  return Ptr(new Decomposition(Type::kForget, forgotten, std::move(child), Ptr()));
}

Decomposition::Ptr Decomposition::Join(Ptr left, Ptr right) {
  return Ptr(new Decomposition(Type::kJoin, SigEdge(), std::move(left), std::move(right)));
}

std::istream& operator>>(std::istream &stream, Decomposition::Ptr &ptr) {
  using Type = Decomposition::Type;
  std::string type;
  stream >> type;
  switch (type[0]) {
    case 'L':
      ptr.reset(new Decomposition(Type::kLeaf));
      return stream;
    case 'I':
      ptr.reset(new Decomposition(Type::kIntroduce));
      return stream >> ptr->edge_ >> ptr->left_;
    case 'F':
      ptr.reset(new Decomposition(Type::kForget));
      return stream >> ptr->edge_ >> ptr->left_;
    case 'J':
      ptr.reset(new Decomposition(Type::kJoin));
      return stream >> ptr->left_ >> ptr->right_;
    default:
      abort();
  }
}

//std::ostream& operator<<(std::ostream &stream, const Decomposition::Ptr &ptr) {
//  using Type = Decomposition::Type;
//  if (ptr) {
//    switch (ptr->type_) {
//      case Type::kLeaf:
//        return stream << 'L';
//      case Type::kIntroduce:
//        return stream << "I " << ptr->edge_.Id() << ' ' << ptr->left_;
//      case Type::kForget:
//        return stream << "F " << ptr->edge_.Id() << ' ' << ptr->left_;
//      case Type::kJoin:
//        return stream << "J " << ptr->left_ << ' ' << ptr->right_;
//      default:
//        assert(false);
//        return stream;
//    }
//  } else {
//    return stream << 'L';
//  }
//}

}  // namespace kopt
