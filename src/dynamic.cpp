#include <dynamic.h>

#include <fast_embedding.h>

namespace kopt {
namespace {

Dynamic::Result DynamicResult(Dynamic::Bag &bag, Dynamic::Table &table, Dynamic::Result &left, Dynamic::Result &right) {
  return std::make_unique<Dynamic::ResultStruct>(bag, std::move(table), std::move(left), std::move(right));
}

Dynamic::Result DynamicResult(Dynamic::Bag &bag, Dynamic::Table &table, Dynamic::Result &left) {
  return std::make_unique<Dynamic::ResultStruct>(bag, std::move(table), std::move(left), nullptr);
}

Dynamic::Result DynamicResult(Dynamic::Bag &bag, Dynamic::Table &table) {
  return std::make_unique<Dynamic::ResultStruct>(bag, std::move(table), nullptr, nullptr);
}

}  // namespace

static const int64_t kNone = std::numeric_limits<int64_t>::min();

Dynamic::Table::Table(Set<SigEdge> bag, int graph_size)
    : table_(Embedding::IdSize(bag, graph_size), kNone),
      bag_(bag), graph_size_(graph_size) {}

template<class Index>
int64_t& Dynamic::Table::operator[](const Index &idx) {
  return table_[idx.Id()];
}

template<class Index>
const int64_t& Dynamic::Table::operator[](const Index &idx) const {
  return table_[idx.Id()];
}

struct Clock {
  Clock(char id, int arg): id(id), arg(arg), start(clock()) {}
  ~Clock() {
    clock_t end = clock();
    std::cerr << id << ' ' << arg << ' ' << end - start << '\n';
  }

  char id;
  int arg;
  clock_t start;
};

Dynamic::Dynamic(int graph_size, GainFunc gain) : graph_size_(graph_size), gain_(gain) {}

Dynamic::Result Dynamic::Leaf() const {
  auto bag = Set<SigEdge>();
  auto table = Table(bag, graph_size_);
  auto embedding = Embedding(bag, graph_size_);
  table[embedding] = 0;
  return DynamicResult(bag, table);
}

Dynamic::Result Dynamic::Introduce(SigEdge introduced, Result child) const {
  auto parent_bag = child->bag + Bag(introduced);
  auto parent_table = Table(parent_bag, graph_size_);
  auto parent_embedding = Embedding(parent_bag, graph_size_);
  do {
    auto &parent_gain = parent_table[parent_embedding];
    auto &child_gain = child->table[parent_embedding - introduced];
    if (child_gain != kNone) {
      parent_gain = child_gain + gain_.Introduce(parent_embedding, introduced);
    }
  } while (parent_embedding.Next());
  return DynamicResult(parent_bag, parent_table, child);
}

Dynamic::Result Dynamic::Forget(SigEdge forgotten, Result child) const {
  auto parent_bag = child->bag - Bag(forgotten);
  auto parent_table = Table(parent_bag, graph_size_);
  auto child_embedding = Embedding(child->bag, graph_size_);
  do {
    auto &parent_gain = parent_table[child_embedding - forgotten];
    auto &child_gain = child->table[child_embedding];
    parent_gain = std::max(parent_gain, child_gain);
  } while (child_embedding.Next());
  return DynamicResult(parent_bag, parent_table, child);
}

Dynamic::Result Dynamic::Join(Result left, Result right) const {
  auto parent_bag = left->bag;
  auto parent_table = Table(left->bag, graph_size_);
  auto parent_embedding = Embedding(left->bag, graph_size_);
  do {
    auto &parent_gain = parent_table[parent_embedding];
    auto &left_gain = left->table[parent_embedding];
    auto &right_gain = right->table[parent_embedding];
    if (left_gain != kNone && right_gain != kNone) {
      parent_gain = left_gain + right_gain - gain_.Join(parent_embedding);
    }
  } while (parent_embedding.Next());
  return DynamicResult(parent_bag, parent_table, left, right);
}

void RetrieveEmbeddingDfs(const Dynamic::Result &subtree, SlowEmbedding *full, SlowEmbedding *bag) {
  if (!subtree->left) {
    // Leaf
  } else if (subtree->right) {
    // Join
    SlowEmbedding bag_copy = *bag;
    RetrieveEmbeddingDfs(subtree->left, full, bag);
    *bag = bag_copy;
    RetrieveEmbeddingDfs(subtree->right, full, bag);
  } else if (subtree->bag.Size() > subtree->left->bag.Size()) {
    // Introduce
    SigEdge introduced = *(subtree->bag - subtree->left->bag).begin();
    bag->Remove(introduced);
    RetrieveEmbeddingDfs(subtree->left, full, bag);
  } else {
    // Forget
    SigEdge forgotten = *(subtree->left->bag - subtree->bag).begin();
    int idx = bag->Domain().Index(forgotten);
    int lowest = idx > 0 ? (*bag)(bag->Domain().Nth(idx)).id + 1 : 0;
    int highest = idx < bag->Domain().Size() ? (*bag)(bag->Domain().Nth(idx + 1)).id - 1 : bag->Codomain() - 1;

    int64_t best = std::numeric_limits<int64_t>::min();
    int best_i = -1;
    for (int i = lowest; i <= highest; ++i) {
      bag->SetVal(forgotten, CycleEdge(i));
      int64_t now = subtree->left->table[bag->Index()];
      if (now > best) {
        best = now;
        best_i = i;
      }
    }
    full->SetVal(forgotten, CycleEdge(best_i));
    bag->SetVal(forgotten, CycleEdge(best_i));

    RetrieveEmbeddingDfs(subtree->left, full, bag);
  }
}

SlowEmbedding RetrieveEmbedding(const Dynamic::Result &root, int graph_size) {
  SlowEmbedding full(graph_size), bag(graph_size);
  RetrieveEmbeddingDfs(root, &full, &bag);
  return full;
}

std::ostream& operator<<(std::ostream &stream, const Dynamic::Result &result) {
  return stream << "bag: " << result->bag << "\ntable: " << result->table;
}

std::ostream& operator<<(std::ostream &stream, const Dynamic::Table &table) {
  stream << "{\n";
  Embedding embedding(table.bag_, table.graph_size_);
  do {
    stream << embedding << ": " << table[embedding] << '\n';
  } while (embedding.Next());
  stream << "}\n";
  return stream;
}

}  // namespace kopt
