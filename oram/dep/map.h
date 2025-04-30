#ifndef MAP_H
#define MAP_H

#include "oram.h"
#include "new_oram.h"

struct MapMetadata {
  unsigned int l_child_leaf;
  unsigned int r_child_leaf;
  unsigned int parent_leaf;
};

template <typename K, typename V>
class ObliviousMap : public ORAM {
 public:
  ObliviousMap();
  void insert(K k, V v);
 private:
  MapMetadata parse_metadata(char *buf);
  void serialize_metadata(char *buf, MapMetadata m);

  unsigned int root_leaf;
  unsigned int root_addr;
};

template <typename K, typename V>
class MapClient : public ORAMClient {
public:
  MapClient(std::string server_addr, int port);
  void insert(K k, V v);
private:
  Block right_rotate(Block addr); // TODO should I use Block or addr as identifyer? 
  Block left_rotate(Block addr);
  int get_balance(unsigned int addr); // get balance of node @ addr
  MapMetadata parse_metadata(char *buf);
  void serialize_metadata(char *buf, MapMetadata m);

  unsigned int root_addr, root_leaf;
};


#endif
