#ifndef SET_H
#define SET_H

#include "oram.h"
#include "new_oram.h"
#include "map.h"

template <typename V>
class SetClient : public ORAMClient {
 public:
  SetClient(std::string server_addr, int port);
  void insert(V v);
  void remove(V v);
  bool contains(V v);
  int size(); 
  void prefix_print();
 private:
  BlockPtr insert(V v, BlockPtr b_ptr);
  BlockPtr remove(V v, BlockPtr b_ptr);
  BlockPtr find(V v, BlockPtr b_ptr);

  BlockPtr right_rotate(BlockPtr b_ptr); 
  BlockPtr left_rotate(BlockPtr b_ptr);

  int get_balance(BlockPtr b_ptr);
  BlockPtr min_node(BlockPtr b_ptr);

  Block* get_block(BlockPtr b_ptr);
  Block* get_block(unsigned int addr, unsigned int leaf_idx);

  MapMetadata parse_metadata(char *buf);
  void serialize_metadata(char *buf, MapMetadata m);

  void prefix_print(BlockPtr b_ptr);

  unsigned int root_addr, root_leaf;
  int ctr;
  int entries;
};

#endif // SET_H
