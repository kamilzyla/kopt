#ifndef KOPT_CLEVER_DYNAMIC_H_
#define KOPT_CLEVER_DYNAMIC_H_

#include <memory>
#include <ostream>

#include <slow_embedding.h>
#include <gain_func.h>
#include <identifier.h>
#include <set.h>

namespace kopt {

class Dynamic {
 public:
  using Bag = Set<SigEdge>;
  class Table;
  class ResultStruct;
  using Result = std::unique_ptr<ResultStruct>;

  Dynamic(int graph_size, GainFunc gain);

  Result Leaf() const;
  Result Introduce(SigEdge introduced, Result child) const;
  Result Forget(SigEdge forgotten, Result child) const;
  Result Join(Result left, Result right) const;

 private:
  const int graph_size_;
  const GainFunc gain_;
};

class Dynamic::Table {
 public:
  Table(Bag bag, int graph_size);

  template<class Index>
  int64_t& operator[](const Index &idx);
  int64_t& operator[](int idx) { return table_[idx]; }
  
  template<class Index>
  const int64_t& operator[](const Index &idx) const;
  const int64_t& operator[](int idx) const { return table_[idx]; }

 private:
  std::vector<int64_t> table_;
  
  // Used for printing
  Bag bag_;
  int graph_size_;
  
  friend std::ostream& operator<<(std::ostream &, const Dynamic::Table &);
};

class Dynamic::ResultStruct {
 public:
  ResultStruct(Bag bag, Table &&table, Result left, Result right)
      : bag(bag), table(std::move(table)), left(std::move(left)), right(std::move(right)) {}

  Bag bag;
  Table table;

  // Leaf nodes do not use these; Introduce and Forget nodes use left; Join uses left and right;
  Result left, right;
};

SlowEmbedding RetrieveEmbedding(const Dynamic::Result &root, int graph_size);

std::ostream& operator<<(std::ostream &, const Dynamic::Result &);
std::ostream& operator<<(std::ostream &, const Dynamic::Table &);

}  // namespace kopt



#endif  // KOPT_CLEVER_DYNAMIC_H_
