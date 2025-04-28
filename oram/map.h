#ifndef MAP_H
#define MAP_H

#include "oram.h"

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
};

#endif
