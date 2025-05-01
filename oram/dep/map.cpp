#include <cstring>

#include "map.h"

template class MapClient<int, int>;

template<typename K, typename V>
MapClient<K, V>::MapClient(std::string server_addr, int port) : ORAMClient(server_addr, port) {
  root_addr = 0, root_leaf = 0;
  ctr = 0;
  entries = 0;
}

template<typename K, typename V>
void MapClient<K, V>::insert(K k, V v) {
  ++entries;
  std::cout << "ROOT: " << root_addr << " " << root_leaf << "\n";
  BlockPtr b_ptr = insert(k, v, BlockPtr{.addr = root_addr, .leaf_idx = root_leaf});

  root_addr = b_ptr.addr;
  root_leaf = b_ptr.leaf_idx;

  // std::cout << "New root after insert: " << b_ptr.addr << " " << b_ptr.leaf_idx << "\n";
  // std::cout << "New metadata: " << b_meta.l_child_addr << " " << b_meta.r_child_addr << "\n";

  unsigned int random_dump_leaf_idx = random_leaf_idx();
  get_blocks(random_dump_leaf_idx);
  // get_blocks(root_leaf);
  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
  // dump_stash(root_leaf);
  dump_stash(random_dump_leaf_idx);
  std::cout << "ROOT: " << root_addr << " " << root_leaf << "\n";
}

template<typename K, typename V>
bool MapClient<K, V>::remove(K k) {
  --entries;
  std::cout << "Starting removal... key: " << k << "\n";
  std::cout << "ROOT: " << root_addr << " " << root_leaf << "\n";
  remove(k, BlockPtr{.addr = root_addr, .leaf_idx = root_leaf});
  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
  return 0;
}

template<typename K, typename V>
V MapClient<K, V>::at(K k) {
  // std::cout << "at(" << k << ");\n";
  // std::cout << "stash size: " << stash.size() << "\n";

  BlockPtr b_ptr = find_key(k, BlockPtr(root_addr, root_leaf));
  Block *b = get_block(b_ptr.addr, b_ptr.leaf_idx);

  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
  return ((std::pair<K, V>*)b->data)->second;
}

template<typename K, typename V>
bool MapClient<K, V>::contains(K k) {
  BlockPtr b_ptr = find_key(k, BlockPtr(root_addr, root_leaf));

  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
  return b_ptr.addr != 0;
}

template<typename K, typename V>
int MapClient<K, V>::size() {
  return entries;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::insert(K k, V v, BlockPtr root) {
  // std::cout << "Inserting (" << k << ", " << v << ") - @ addr " << root.addr << "\n";

  if(root.addr == 0) {
    Block new_block;
    // std::cout << "Doing insert...\n";
    new_block.addr = ++ctr;
    new_block.leaf_idx = random_leaf_idx();
    new_block.in_use = true;
    memset(new_block.metadata, 0, sizeof(MapMetadata));

    std::pair<K, V> data = std::pair<K, V>(k, v);
    memcpy(new_block.data, (char*)(&data), sizeof(std::pair<K, V>));

    stash.push_back(new_block);
    return BlockPtr {.addr = new_block.addr, .leaf_idx = new_block.leaf_idx};
  }

  Block *b = get_block(root.addr, root.leaf_idx);
  std::pair<K, V> *b_entry = (std::pair<K, V>*)b->data;

  MapMetadata b_meta = parse_metadata(b->metadata);
  if(k < b_entry->first) {
    // std::cout << "going left\n";
    BlockPtr b_ptr = insert(k, v, BlockPtr{.addr = b_meta.l_child_addr, .leaf_idx = b_meta.l_child_leaf});
    b_meta.l_child_addr = b_ptr.addr;
    b_meta.l_child_leaf = b_ptr.leaf_idx;
    serialize_metadata(get_block(root.addr, root.leaf_idx)->metadata, b_meta);
  } else if(k > b_entry->first) {
    // std::cout << "going right\n";
    BlockPtr b_ptr = insert(k, v, BlockPtr{.addr = b_meta.r_child_addr, .leaf_idx = b_meta.r_child_leaf});
    b_meta.r_child_addr = b_ptr.addr;
    b_meta.r_child_leaf = b_ptr.leaf_idx;
    serialize_metadata(get_block(root.addr, root.leaf_idx)->metadata, b_meta);
  } else {
    assert(0); // TODO need to decide what behavior in multiple insertion case
  }

  // std::cout << "Post-Processing...\n";

  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  // update height
  // std::cout << "Updating height\n";
  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  ((MapMetadata*)(b->metadata))->height = 1 + std::max(b_left_height, b_right_height);

  // get balance
  // std::cout << "Getting balance: ";
  // int balance = get_balance(b);
  int balance = b_left_height - b_right_height;
  // std::cout << balance << "\n";

  if(balance > 1 && k < ((std::pair<K, V>*)(b_left->data))->first) {
    // left-left
    return right_rotate(root);
  } else if(balance < -1 && k > ((std::pair<K, V>*)(b_right->data))->first) {
    // right-right
    return left_rotate(root);
  } else if(balance > 1 && k > ((std::pair<K, V>*)(b_left->data))->first) {
    // left-right
    return right_rotate(root);
  } else if(balance < -1 && k < ((std::pair<K, V>*)(b_right->data))->first) {
    // right-left
    return left_rotate(root);
  } else {
    std::cout << "No rotation\n";
    return root;
  }
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::remove(K k, BlockPtr root) {
  if(root.addr == 0) return root;

  std::cout << "Current root..." << root.addr << " " << root.leaf_idx << "\n";
  Block *b = get_block(root.addr, root.leaf_idx);
  MapMetadata b_meta = parse_metadata(b->metadata);
  std::cout << "Looking for node to remove, at key " << k << "\n";

  if(k < ((std::pair<K, V>*)(b->data))->first) {
    BlockPtr left = remove(k, BlockPtr{.addr = b_meta.l_child_addr, .leaf_idx = b_meta.l_child_leaf});
    b_meta.l_child_addr = left.addr;
    b_meta.l_child_leaf = left.leaf_idx;
  } else if(k > ((std::pair<K, V>*)(b->data))->first) {
    BlockPtr right = remove(k, BlockPtr{.addr = b_meta.r_child_addr, .leaf_idx = b_meta.r_child_leaf});
    b_meta.r_child_addr = right.addr;
    b_meta.r_child_leaf = right.leaf_idx;
  } else {
    std::cout << "Need to remove the root!\n";
    // remove root
    if(b_meta.l_child_addr == 0) {
      root.addr = b_meta.r_child_addr;
      root.leaf_idx = b_meta.r_child_leaf;
    } else if(b_meta.r_child_addr == 0) {
      root.addr = b_meta.l_child_addr;
      root.leaf_idx = b_meta.l_child_leaf;
    } else {
      BlockPtr min_ptr = min_node(BlockPtr{.addr = b_meta.r_child_addr, .leaf_idx = b_meta.r_child_leaf});
      Block *min_node = get_block(min_ptr.addr, min_ptr.leaf_idx);

      root.addr = min_ptr.addr;
      root.leaf_idx = min_ptr.leaf_idx;

      BlockPtr right = remove(((std::pair<K, V>*)(min_node->data))->first,
			      BlockPtr{.addr = b_meta.r_child_addr, .leaf_idx = b_meta.r_child_leaf});

      b_meta.r_child_addr = right.addr;
      b_meta.r_child_leaf = right.leaf_idx;
    }
  }
  serialize_metadata(b->metadata, b_meta);
  std::cout << "Post-processing... ";

  if(root.addr == 0) return root; // this means there was only one node, now empty
  std::cout << "non-null node\n";

  // update height
  std::cout << "Update height...";
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);
  serialize_metadata(b->metadata, b_meta);

  std::cout << "DONE!\n";

  int balance = b_left_height - b_right_height;

  if(balance > 1 && get_balance(BlockPtr{.addr = b_left->addr, .leaf_idx = b_left->leaf_idx}) >= 0) {
    std::cout << "A\n";
    return right_rotate(root);
  } else if(balance > 1 && get_balance(BlockPtr{.addr = b_left->addr, .leaf_idx = b_left->leaf_idx}) < 0) {
    std::cout << "B\n";
    BlockPtr new_left = left_rotate(BlockPtr{.addr = b_left->addr, .leaf_idx = b_left->leaf_idx});
    b_meta.l_child_addr = new_left.addr;
    b_meta.l_child_leaf = new_left.leaf_idx;
    serialize_metadata(b->metadata, b_meta);
    return right_rotate(root);
  } else if(balance < -1 && get_balance(BlockPtr{.addr = b_right->addr, .leaf_idx = b_right->leaf_idx}) <= 0) {
    std::cout << "C\n";
    BlockPtr b = left_rotate(root);
    std::cout << "Rotated\n";
    return b;
  } else if(balance < -1 && get_balance(BlockPtr{.addr = b_right->addr, .leaf_idx = b_right->leaf_idx}) > 0) {
    std::cout << "D\n";
    BlockPtr new_right = right_rotate(BlockPtr{.addr = b_right->addr, .leaf_idx = b_right->leaf_idx});
    b_meta.r_child_addr = new_right.addr;
    b_meta.r_child_leaf = new_right.leaf_idx;
    return left_rotate(root);
  }

  std::cout << "No rebalancing needed\n";

  return root;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::find_key(K k, BlockPtr root) {
  if(root.addr == 0) return root;
  // std::cout << "find_key " << k << " with curr root @ block addr " << root.addr << "\n";
  Block *b = get_block(root.addr, root.leaf_idx);
  // if(b == NULL) std::cout << "ROOT IS NULL?\n";
  // else std::cout << "Got a block ...\n";

  MapMetadata b_meta = *(MapMetadata*)(b->metadata);
  // std::cout << ((std::pair<K, V>*)b->data)->first << " " << ((std::pair<K, V>*)b->data)->second << "\n";

  // std::cout << "Left addr: " << b_meta.l_child_addr << " | right addr: " << b_meta.r_child_addr << "\n";


  if(((std::pair<K, V>*)b->data)->first == k) {
    // std::cout << "FOUND KEY: " << k << "!\n";
    return root;
  } else if(((std::pair<K, V>*)b->data)->first < k) {
    // std::cout << "Going right to addr " << b_meta.r_child_addr << "\n"; 
    return find_key(k, BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
  } else if(((std::pair<K, V>*)b->data)->first > k) {
    // std::cout << "Going left\n";
    return find_key(k, BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf));
  } else return BlockPtr(0, 0); // impossible case
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::right_rotate(BlockPtr b_ptr) {
  Block *b = get_block(b_ptr.addr, b_ptr.leaf_idx);
  MapMetadata b_meta = parse_metadata(b->metadata);

  Block *new_root = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  MapMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.l_child_addr = new_root_meta.r_child_addr; // replace new root w/ it's right child in old root
  b_meta.l_child_leaf = new_root_meta.r_child_leaf;

  b = get_block(b_ptr.addr, b_ptr.leaf_idx);
  new_root_meta.r_child_addr = b->addr; // new root's right child is the old root
  new_root_meta.r_child_leaf = b->leaf_idx;

  // update heights
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);

  MapMetadata new_root_left_meta = parse_metadata(get_block(new_root_meta.l_child_addr,
							    new_root_meta.l_child_leaf)->metadata);
  MapMetadata new_root_right_meta = parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf)->metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, new_root_right_meta.height);

  // copy metadata back in
  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  BlockPtr new_root_ptr {.addr = new_root->addr, .leaf_idx = new_root->leaf_idx};
  return new_root_ptr;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::left_rotate(BlockPtr b_ptr) {
  // TODO why are we getting illegal leaf indexes?
  std::cout << "LEFT ROTATE " << b_ptr.addr << ", " << b_ptr.leaf_idx << "\n";
  Block *b = get_block(b_ptr.addr, b_ptr.leaf_idx);
  MapMetadata b_meta = parse_metadata(b->metadata);

  Block *new_root = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);
  MapMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.r_child_addr = new_root_meta.l_child_addr; // replace new root w/ it's left child in old root
  b_meta.r_child_leaf = new_root_meta.l_child_leaf;

  b = get_block(b_ptr.addr, b_ptr.leaf_idx);
  new_root_meta.l_child_addr = b->addr; // new root's left child is the old root
  new_root_meta.l_child_leaf = b->leaf_idx;

  // update heights
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);

  MapMetadata new_root_left_meta = parse_metadata(get_block(new_root_meta.l_child_addr,
							    new_root_meta.l_child_leaf)->metadata);
  MapMetadata new_root_right_meta = parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf)->metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, new_root_right_meta.height);

  // copy metadata back in
  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  BlockPtr new_root_ptr {.addr = new_root->addr, .leaf_idx = new_root->leaf_idx};
  return new_root_ptr;
}

template<typename K, typename V>
BlockPtr MapClient<K, V>::min_node(BlockPtr root) {
  Block *curr = get_block(root);
  MapMetadata root_meta = parse_metadata(curr->metadata);

  while(root_meta.r_child_addr != 0) {
    curr = get_block(root_meta.r_child_addr, root_meta.r_child_leaf);
    root_meta = parse_metadata(curr->metadata);
  }
  return BlockPtr{.addr = curr->addr, .leaf_idx = curr->leaf_idx};
}

// template<typename K, typename V>
// int MapClient<K, V>::get_balance(Block *b) {
//   if(b == NULL) return 0;

//   MapMetadata m = parse_metadata(b->metadata);

//   MapMetadata m_left = parse_metadata(get_block(m.l_child_addr, m.l_child_leaf)->metadata);
//   MapMetadata m_right = parse_metadata(get_block(m.l_child_addr, m.l_child_leaf)->metadata);

//   int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
//   int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;

//   return m_left.height - m_right.height;
// }

template<typename K, typename V>
Block* MapClient<K, V>::get_block(BlockPtr b) {
  return get_block(b.addr, b.leaf_idx);
}

template<typename K, typename V>
Block* MapClient<K, V>::get_block(unsigned int addr, unsigned int leaf_idx) {
  // std::cout << "get_block(" << addr << ", " << leaf_idx << ");\n";
  Block b;
  b.addr = 0;

  // check stash
  if(stash.size() > 0) {
    // std::cout << "---------- CURRENT STASH ----------\n";
  }
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    // std::cout << (*it).addr << " " << (*it).leaf_idx << ": " << (*it).in_use << "\n";
    if((*it).addr == addr && (*it).leaf_idx == leaf_idx) {
      (*it).in_use = true;
      return &(*it);
    }
  }
  if(stash.size() > 0) {
    // std::cout << "-----------------------------------\n";
  }

  // std::cout << "Not found in stash\n";

  // check server
  get_blocks(leaf_idx);
  // std::cout << "did get blocks\n";
  if(stash.size() > 0) {
    // std::cout << "---------- CURRENT STASH ----------\n";
  }
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    // std::cout << (*it).addr << " " << (*it).leaf_idx << ": " << (*it).in_use << "\n";
    if((*it).addr == addr && (*it).leaf_idx == leaf_idx) {
      b = (*it);
      stash.erase(it, it+1);
      break;
    }
  }
  if(stash.size() > 0 || b.addr != 0) {
    // std::cout << "-----------------------------------\n";
  }
  dump_stash(leaf_idx);

  if(b.addr == 0) return NULL;

  b.in_use = true;

  stash.push_back(b);
  return &stash[stash.size()-1];
}

template<typename K, typename V>
int MapClient<K, V>::height(Block *b) {
  if(b == NULL) return 0;

  MapMetadata m = parse_metadata(b->metadata);

  Block *left = get_block(m.l_child_addr, m.l_child_leaf);
  Block *right = get_block(m.r_child_addr, m.r_child_leaf);

  return std::max(height(left), height(right));
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

template<typename K, typename V>
int MapClient<K, V>::get_balance(BlockPtr b_ptr) {
  MapMetadata b_meta = parse_metadata(get_block(b_ptr)->metadata);
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;

  int balance = b_left_height - b_right_height;
  return balance;
}
