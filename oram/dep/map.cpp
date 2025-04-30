#include <cstring>

#include "map.h"

template class MapClient<int, int>;

template<typename K, typename V>
MapClient<K, V>::MapClient(std::string server_addr, int port) : ORAMClient(server_addr, port) {
  root_addr = 0, root_leaf = 0;
}

template<typename K, typename V>
void MapClient<K, V>::insert(K k, V v) {
  BlockPtr b_ptr = insert(k, v, BlockPtr{.addr = root_addr, .leaf_idx = root_leaf});

  root_addr = b_ptr.addr;
  root_leaf = b_ptr.leaf_idx;
}

template<typename K, typename V>
bool MapClient<K, V>::remove(K k) {

  return 0;
}

template<typename K, typename V>
V MapClient<K, V>::at(K k) {
  std::cout << "at(" << k << ");\n";
  std::cout << "stash size: " << stash.size() << "\n";

  BlockPtr b_ptr = find_key(k, BlockPtr(root_addr, root_leaf));
  Block b = get_block(b_ptr.addr, b_ptr.leaf_idx);
  return ((std::pair<K, V>*)b.data)->second;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::insert(K k, V v, BlockPtr root) {
  std::cout << "Inserting (" << k << ", " << v << ") - @ addr " << root.addr << "\n";

  Block b;

  if(root.addr == 0) {
    std::cout << "Doing insert...\n";
    b.addr = ++ctr;
    b.leaf_idx = random_leaf_idx();
    memset(b.metadata, 0, sizeof(MapMetadata));

    std::pair<K, V> data = std::pair<K, V>(k, v);
    memcpy(b.data, (char*)(&data), sizeof(std::pair<K, V>));

    get_blocks(b.leaf_idx);
    stash.push_back(b);

    dump_stash(b.leaf_idx);

    return BlockPtr {.addr = b.addr, .leaf_idx = b.leaf_idx};
  }

  b = get_block(root.addr, root.leaf_idx);
  std::pair<K, V> *b_entry = (std::pair<K, V>*)b.data;

  MapMetadata b_meta = parse_metadata(b.metadata);
  if(k < b_entry->first) {
    std::cout << "going left\n";
    BlockPtr b_ptr = insert(k, v, BlockPtr{.addr = b_meta.l_child_addr, .leaf_idx = b_meta.l_child_leaf});
    b_meta.l_child_addr = b_ptr.addr;
    b_meta.l_child_leaf = b_ptr.leaf_idx;
    serialize_metadata((char*)b.metadata, b_meta);
  } else if(k > b_entry->first) {
    std::cout << "going right\n";
    BlockPtr b_ptr = insert(k, v, BlockPtr{.addr = b_meta.r_child_addr, .leaf_idx = b_meta.r_child_leaf});
    b_meta.r_child_addr = b_ptr.addr;
    b_meta.r_child_leaf = b_ptr.leaf_idx;
    serialize_metadata((char*)(&b.metadata), b_meta);
  } else {
    assert(0); // TODO need to decide what behavior in multiple insertion case
  }

  std::cout << "Post-Processing...\n";

  Block b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  // update height
  std::cout << "Updating height\n";
  MapMetadata b_left_meta = parse_metadata(b_left.metadata);
  MapMetadata b_right_meta = parse_metadata(b_right.metadata);
  ((MapMetadata*)(b.metadata))->height = 1 + std::max(b_left_meta.height, b_right_meta.height);

  // get balance
  std::cout << "Getting balance: ";
  int balance = get_balance(&b);
  std::cout << balance << "\n";

  if(balance > 1 && k < ((std::pair<K, V>*)(b_left.data))->first) {
    // left-left
    return right_rotate(root);
  } else if(balance < -1 && k > ((std::pair<K, V>*)(b_right.data))->first) {
    // right-right
    return left_rotate(root);
  } else if(balance > 1 && k > ((std::pair<K, V>*)(b_left.data))->first) {
    // left-right
    return right_rotate(root);
  } else if(balance < -1 && k < ((std::pair<K, V>*)(b_right.data))->first) {
    // right-left
    return left_rotate(root);
  } else {
    return root;
  }

  BlockPtr nop_ptr;
  return nop_ptr;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::find_key(K k, BlockPtr root) {
  std::cout << "find_key " << k << " @ block addr " << root.addr << "\n";
  Block b = get_block(root.addr, root.leaf_idx);
  MapMetadata b_meta = *(MapMetadata*)b.metadata;
  if(((std::pair<K, V>*)b.data)->first == k) {
    std::cout << "FOUND!\n";
    return root;
  } else if(((std::pair<K, V>*)b.data)->first < k) {
    std::cout << "Going right\n";
    return find_key(k, BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
  } else if(((std::pair<K, V>*)b.data)->first > k) {
    std::cout << "Going left\n";
    return find_key(k, BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf));
  } else return BlockPtr(0, 0); // impossible case
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::right_rotate(BlockPtr b_ptr) {
  Block b_tmp = get_block(b_ptr.addr, b_ptr.leaf_idx);
  Block *b = &b_tmp;

  MapMetadata b_meta = parse_metadata(b->metadata);

  Block new_root_tmp = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *new_root = &new_root_tmp;

  MapMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.l_child_addr = new_root_meta.r_child_addr; // replace new root w/ it's right child in old root
  b_meta.l_child_leaf = new_root_meta.r_child_leaf;

  new_root_meta.r_child_addr = b->addr; // new root's right child is the old root
  new_root_meta.r_child_leaf = b->leaf_idx;

  // update heights
  MapMetadata b_left_meta = parse_metadata(get_block(b_meta.l_child_addr, b_meta.l_child_leaf).metadata);
  MapMetadata b_right_meta = parse_metadata(get_block(b_meta.r_child_addr, b_meta.r_child_leaf).metadata);
  b_meta.height = 1 + std::max(b_left_meta.height, b_right_meta.height);

  MapMetadata new_root_left_meta = parse_metadata(get_block(new_root_meta.l_child_addr,
							    new_root_meta.l_child_leaf).metadata);
  MapMetadata new_root_right_meta = parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf).metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, new_root_right_meta.height);

  // copy metadata back in
  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  BlockPtr new_root_ptr {.addr = new_root->addr, .leaf_idx = new_root->leaf_idx};
  return new_root_ptr;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::left_rotate(BlockPtr b_ptr) {
  Block b_tmp = get_block(b_ptr.addr, b_ptr.leaf_idx);
  Block *b = &b_tmp;

  MapMetadata b_meta = parse_metadata(b->metadata);

  Block new_root_tmp = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);
  Block *new_root = &new_root_tmp;

  MapMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.r_child_addr = new_root_meta.l_child_addr; // replace new root w/ it's left child in old root
  b_meta.r_child_leaf = new_root_meta.l_child_leaf;

  new_root_meta.l_child_addr = b->addr; // new root's left child is the old root
  new_root_meta.l_child_leaf = b->leaf_idx;

  // update heights
  MapMetadata b_left_meta = parse_metadata(get_block(b_meta.l_child_addr, b_meta.l_child_leaf).metadata);
  MapMetadata b_right_meta = parse_metadata(get_block(b_meta.r_child_addr, b_meta.r_child_leaf).metadata);
  b_meta.height = 1 + std::max(b_left_meta.height, b_right_meta.height);

  MapMetadata new_root_left_meta = parse_metadata(get_block(new_root_meta.l_child_addr,
							    new_root_meta.l_child_leaf).metadata);
  MapMetadata new_root_right_meta = parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf).metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, new_root_right_meta.height);

  // copy metadata back in
  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  BlockPtr new_root_ptr {.addr = new_root->addr, .leaf_idx = new_root->leaf_idx};
  return new_root_ptr;
}

template<typename K, typename V>
int MapClient<K, V>::get_balance(Block *b) {
  if(b == NULL) return 0;

  MapMetadata m = parse_metadata(b->metadata);

  MapMetadata m_left = parse_metadata(get_block(m.l_child_addr, m.l_child_leaf).metadata);
  MapMetadata m_right = parse_metadata(get_block(m.l_child_addr, m.l_child_leaf).metadata);

  return m_left.height - m_right.height;
}

template<typename K, typename V>
Block MapClient<K, V>::get_block(unsigned int addr, unsigned int leaf_idx) {
  Block *b = NULL;
  // check stash
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    b = &(*it);
    if(b->addr == addr && b->leaf_idx == leaf_idx) return *b;
  }

  // check server
  get_blocks(leaf_idx);
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    b = &(*it);
    if(b->addr == addr && b->leaf_idx == leaf_idx) break;
  }
  dump_stash(leaf_idx);

  if(b == NULL) return Block();
  else return *b;
}

template<typename K, typename V>
int MapClient<K, V>::height(Block *b) {
  if(b == NULL) return 0;

  MapMetadata m = parse_metadata(b->metadata);

  Block left = get_block(m.l_child_addr, m.l_child_leaf);
  Block right = get_block(m.r_child_addr, m.r_child_leaf);

  return std::max(height(&left), height(&right));
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
