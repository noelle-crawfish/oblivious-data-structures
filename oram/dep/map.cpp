#include <cstring>

#include "map.h"

template<typename K, typename V>
ObliviousMap<K, V>::ObliviousMap() {
  // ctr = 0;
  root_addr = 0;
  root_leaf = N_LEAVES; // invalid value to represent "empty"
}

template<typename K, typename V>
void ObliviousMap<K, V>::insert(K k, V v) {


  // unsigned int leaf_idx = std::rand() % N_LEAVES;
  unsigned int leaf_idx = 0;


  if(root_leaf == N_LEAVES) {
    root_leaf = leaf_idx;
  }
}

template<typename K, typename V>
MapMetadata ObliviousMap<K, V>::parse_metadata(char *buf) {
  MapMetadata m;

  // m.l_child_leaf = *((unsigned int*)buf);
  // m.r_child_leaf = *((unsigned int*)buf + 1);
  // m.parent_leaf = *((unsigned int*)buf + 2);

  return m;
}

template<typename K, typename V>
void ObliviousMap<K, V>::serialize_metadata(char *buf, MapMetadata m) {
  // memcpy(buf, &m.l_child_leaf, sizeof(unsigned int));
  // memcpy(buf + sizeof(unsigned int), &m.r_child_leaf, sizeof(unsigned int));
  // memcpy(buf + 2*sizeof(unsigned int), &m.parent_leaf, sizeof(unsigned int));
}

// ------------------------------------------------------------------------------------------

template<typename K, typename V>
MapClient<K, V>::MapClient(std::string server_addr, int port) : ORAMClient(server_addr, port) {
  root_addr = 0, root_leaf = 0;
}

template<typename K, typename V>
void MapClient<K, V>::insert(K k, V v) {

}

template<typename K, typename V>
Block* MapClient<K, V>::right_rotate(Block *b) {
  MapMetadata b_meta = parse_metadata(b->metadata);
  Block *new_root = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);

  MapMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.l_child_addr = new_root_meta.r_child_addr; // replace new root w/ it's right child in old root
  b_meta.l_child_leaf = new_root_meta.r_child_leaf;

  new_root_meta.r_child_addr = b->addr; // new root's right child is the old root
  new_root_meta.r_child_leaf = b->leaf_idx;

  // update heights
  MapMetadata b_left_meta = parse_metadata(get_block(b_meta.l_child_addr, b_meta.l_child_leaf)->metadata);
  MapMetadata b_right_meta = parse_metadata(get_block(b_meta.r_child_addr, b_meta.r_child_leaf)->metadata);
  b_meta.height = 1 + std::max(b_left_meta.height, b_right_meta.height);

  MapMetadata new_root_left_meta = parse_metadata(get_block(new_root_meta.l_child_addr,
							    new_root_meta.l_child_leaf)->metadata);
  MapMetadata new_root_right_meta = parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf)->metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, new_root_right_meta.height);

  // copy metadata back in
  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  return new_root;
}

template<typename K, typename V>
Block* MapClient<K, V>::left_rotate(Block *b) {
  MapMetadata b_meta = parse_metadata(b->metadata);
  Block *new_root = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  MapMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.r_child_addr = new_root_meta.l_child_addr; // replace new root w/ it's left child in old root
  b_meta.r_child_leaf = new_root_meta.l_child_leaf;

  new_root_meta.l_child_addr = b->addr; // new root's left child is the old root
  new_root_meta.l_child_leaf = b->leaf_idx;

  // update heights
  MapMetadata b_left_meta = parse_metadata(get_block(b_meta.l_child_addr, b_meta.l_child_leaf)->metadata);
  MapMetadata b_right_meta = parse_metadata(get_block(b_meta.r_child_addr, b_meta.r_child_leaf)->metadata);
  b_meta.height = 1 + std::max(b_left_meta.height, b_right_meta.height);

  MapMetadata new_root_left_meta = parse_metadata(get_block(new_root_meta.l_child_addr,
							    new_root_meta.l_child_leaf)->metadata);
  MapMetadata new_root_right_meta = parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf)->metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, new_root_right_meta.height);

  // copy metadata back in
  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  return new_root;
}

template<typename K, typename V>
int MapClient<K, V>::get_balance(Block *b) {
  if(b == NULL) return 0;

  MapMetadata m = parse_metadata(b->metadata);

  MapMetadata m_left = parse_metadata(get_block(m.l_child_addr, m.l_child_leaf)->metadata);
  MapMetadata m_right = parse_metadata(get_block(m.l_child_addr, m.l_child_leaf)->metadata);

  return m_left.height - m_right.height;
}

template<typename K, typename V>
Block MapClient<K, V>::get_block(unsigned int addr, unsigned int leaf_idx) {
  Block *b = NULL;
  // check stash
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    b = (Block*)it;
    if(b->addr == addr && b->leaf_idx == leaf_idx) return *b;
  }

  // check server
  get_blocks(leaf_idx);
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    b = (Block*)it;
    if(b->addr == addr && b->leaf_idx == leaf_idx) break;
  }
  dump_stash(leaf_idx);

  return *b;
}

template<typename K, typename V>
int MapClient<K, V>::height(Block *b) {
  if(b == NULL) return 0;

  MapMetadata m = parse_metadata(b->metadata);

  Block left = get_block(m.l_child_addr, m.l_child_leaf);
  Block right = get_block(m.r_child_addr, m.r_child_leaf);

  return max(height(&left), height(&right));
}

template<typename K, typename V>
MapMetadata MapClient<K, V>::parse_metadata(char *buf) {
  MapMetadata m;
  memcpy((char*)&m, buf, sizeof(MapMetadata));
  return m;
}

template<typename K, typename V>
void MapClient<K, V>::serialize_metadata(char *buf, MapMetadata m) {
  memcpy(buf, (char*)&m, sizeof(MapMetadata));
}
