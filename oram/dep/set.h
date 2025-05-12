#ifndef SET_H
#define SET_H

#include "new_oram.h"

struct AVLMetadata {
  unsigned int l_child_leaf;
  unsigned int l_child_addr;
  unsigned int r_child_leaf;
  unsigned int r_child_addr;
  int height;
};

template <typename V>
class SetClient : public ORAMClient {
 public:
  SetClient(std::string server_addr, int port, unsigned int levels,
	    unsigned int bucket_size, unsigned int threshold);
  SetClient(std::string server_addr, int port) :
    SetClient(server_addr, port, L, BUCKET_SIZE, STASH_THRESHOLD) {};
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

