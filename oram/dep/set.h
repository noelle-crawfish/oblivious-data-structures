#ifndef SET_H
#define SET_H

#include "oram.h"
#include "new_oram.h"

struct AVLMetadata {
  unsigned int l_child_leaf;
  unsigned int l_child_addr;
  unsigned int r_child_leaf;
  unsigned int r_child_addr;
  int height;
};

struct BlockPtr {
  unsigned int addr;
  unsigned int leaf_idx;
};

template <typename V>
class SetClient : public ORAMClient {
 public:
  SetClient(std::string server_addr, int port);
  void insert(V v);
  void remove(V v);
  bool contains(V v);
  int size(); 
  void prefix_print();
 protected:
  BlockPtr insert(V v, BlockPtr b_ptr);
  BlockPtr remove(V v, BlockPtr b_ptr);
  BlockPtr find(V v, BlockPtr b_ptr);

  BlockPtr right_rotate(BlockPtr b_ptr); 
  BlockPtr left_rotate(BlockPtr b_ptr);

  int get_balance(BlockPtr b_ptr);
  BlockPtr min_node(BlockPtr b_ptr);
  virtual int compare_value(V v1, V v2);

  Block* get_block(BlockPtr b_ptr);
  Block* get_block(unsigned int addr, unsigned int leaf_idx);

  AVLMetadata parse_metadata(char *buf);
  void serialize_metadata(char *buf, AVLMetadata m);

  void prefix_print(BlockPtr b_ptr);

  unsigned int root_addr, root_leaf;
  int ctr;
  int entries;
};


template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& p) {
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}


#endif // SET_H

