#ifndef MAP_H
#define MAP_H

#include "oram.h"
#include "new_oram.h"

struct MapMetadata {
  unsigned int l_child_leaf;
  unsigned int l_child_addr;
  unsigned int r_child_leaf;
  unsigned int r_child_addr;
  int height;
//   unsigned int parent_leaf;
//   unsigned int parent_addr;
};

struct BlockPtr {
  unsigned int addr;
  unsigned int leaf_idx;
};

template <typename K, typename V>
class MapClient : public ORAMClient {
public:
  MapClient(std::string server_addr, int port);
  void insert(K k, V v);
  bool remove(K k); // TODO
  V at(K k);
private:
  BlockPtr insert(K k, V v, BlockPtr root);
  BlockPtr find_key(K k, BlockPtr root);
  BlockPtr right_rotate(BlockPtr b_ptr); 
  BlockPtr left_rotate(BlockPtr b_ptr);
  int get_balance(Block *b); // get balance of node @ addr
  Block get_block(unsigned int addr, unsigned int leaf_idx);
  int height(Block *b);
  MapMetadata parse_metadata(char *buf);
  void serialize_metadata(char *buf, MapMetadata m);

  unsigned int root_addr, root_leaf;
  int ctr;
};


#endif
