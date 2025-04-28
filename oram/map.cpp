#include <cstring>

#include "map.h"

template<typename K, typename V>
ObliviousMap<K, V>::ObliviousMap() {
  root_leaf = N_LEAVES; // invalid value to represent "empty"
}

template<typename K, typename V>
void ObliviousMap<K, V>::insert(K k, V v) {


  unsigned int leaf_idx = std::rand() % N_LEAVES;


  if(root_leaf == N_LEAVES) {
    root_leaf = leaf_idx;
  }
}

template<typename K, typename V>
MapMetadata ObliviousMap<K, V>::parse_metadata(char *buf) {
  MapMetadata m;

  m.l_child_leaf = *((unsigned int*)buf);
  m.r_child_leaf = *((unsigned int*)buf + 1);
  m.parent_leaf = *((unsigned int*)buf + 2);

  return m;
}

template<typename K, typename V>
void ObliviousMap<K, V>::serialize_metadata(char *buf, MapMetadata m) {
  memcpy(buf, &m.l_child_leaf, sizeof(unsigned int));
  memcpy(buf + sizeof(unsigned int), &m.r_child_leaf, sizeof(unsigned int));
  memcpy(buf + 2*sizeof(unsigned int), &m.parent_leaf, sizeof(unsigned int));
}
