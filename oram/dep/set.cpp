#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "set.h"

template class SetClient<int>;
template class SetClient<std::pair<int, int>>;

template<typename V>
SetClient<V>::SetClient(std::string server_addr, int port) : ORAMClient(server_addr, port) {
  root_addr = 0, root_leaf = 0;
  ctr = 0;
  entries = 0;
}

template<typename V>
void SetClient<V>::insert(V v) {
  ++entries;

  BlockPtr b_ptr = insert(v, BlockPtr(root_addr, root_leaf));

  root_addr = b_ptr.addr;
  root_leaf = b_ptr.leaf_idx;

  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
}

template<typename V>
void SetClient<V>::remove(V v) {
  BlockPtr b_ptr = remove(v, BlockPtr(root_addr, root_leaf));
  root_addr = b_ptr.addr;
  root_leaf = b_ptr.leaf_idx;

  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
}

template<typename V>
bool SetClient<V>::contains(V v) {
  BlockPtr b_ptr = find(v, BlockPtr(root_addr, root_leaf));

  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
  return (b_ptr.addr != 0);
}

template<typename V>
int SetClient<V>::size() {
  return entries;
}

template<typename V>
BlockPtr SetClient<V>::insert(V v, BlockPtr b_ptr) {
  if(b_ptr.addr == 0) {
    Block new_block;
    
    new_block.addr = ++ctr;
    new_block.leaf_idx = random_leaf_idx();
    new_block.in_use = true;

    AVLMetadata metadata = {
	.l_child_leaf = 0,
	.l_child_addr = 0,
	.r_child_leaf = 0,
	.r_child_addr = 0,
	.height = 1, 
    };

    serialize_metadata(new_block.metadata, metadata);
    memcpy(new_block.data, (char*)(&v), sizeof(V));
    stash.push_back(new_block);
    return BlockPtr(new_block.addr, new_block.leaf_idx);
  }

  Block *b = get_block(b_ptr);
  V b_v = *(V*)(b->data);

  AVLMetadata b_meta = parse_metadata(b->metadata);
  if(compare_value(v, b_v) < 0) {
    BlockPtr new_left = insert(v, BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf));
    b_meta.l_child_addr = new_left.addr;
    b_meta.l_child_leaf = new_left.leaf_idx;
  } else if(compare_value(v, b_v) > 0) {
    BlockPtr new_right = insert(v, BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
    b_meta.r_child_addr = new_right.addr;
    b_meta.r_child_leaf = new_right.leaf_idx;
  } else {
    std::cerr << "Value already exists in tree\n";
    std::abort();
  }

  // post-processing (re-balance tree)
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);

  serialize_metadata(b->metadata, b_meta);

  int balance = b_left_height - b_right_height;

  if(balance > 1 && compare_value(v, b_v) < 0) {
    return right_rotate(b_ptr);
  } else if(balance < -1 && compare_value(v, b_v) > 0) {
    return left_rotate(b_ptr);
  } else if(balance > 1 && compare_value(v, b_v) > 0) {
    BlockPtr new_left = left_rotate(BlockPtr(b_left->addr, b_left->leaf_idx));
    b_meta.l_child_addr = new_left.addr;
    b_meta.l_child_leaf = new_left.leaf_idx;
    serialize_metadata(b->metadata, b_meta);
    return right_rotate(b_ptr);
  } else if(balance < -1 && compare_value(v, b_v) < 0) {
    BlockPtr new_right = right_rotate(BlockPtr(b_right->addr, b_right->leaf_idx));
    b_meta.r_child_addr = new_right.addr;
    b_meta.r_child_leaf = new_right.leaf_idx;
    serialize_metadata(b->metadata, b_meta);
    return left_rotate(b_ptr);
  } else {
    return b_ptr; // no rotation needed :)
  }

  std::cerr << "not implemented\n";
  std::abort();
  return BlockPtr(0, 0);
}

template<typename V>
BlockPtr SetClient<V>::remove(V v, BlockPtr b_ptr) {
  if(b_ptr.addr == 0) {
    std::cerr << "Could not find value to remove.\n";
    std::abort();
  }

  Block *b = get_block(b_ptr);
  AVLMetadata b_meta = parse_metadata(b->metadata);

  V b_v = *(V*)(b->data);
  if(compare_value(v, b_v) < 0) {
    // std::cout << "left_addr: " << b_meta.l_child_addr << "\n";
    BlockPtr new_left = remove(v, BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf));
    b_meta.l_child_addr = new_left.addr;
    b_meta.l_child_leaf = new_left.leaf_idx;
  } else if(compare_value(v, b_v) > 0) {
    BlockPtr new_right = remove(v, BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
    b_meta.r_child_addr = new_right.addr;
    b_meta.r_child_leaf = new_right.leaf_idx;
  } else {
    // need to remove the current node
    if(b_meta.l_child_addr == 0) {
      b_ptr.addr = b_meta.r_child_addr;
      b_ptr.leaf_idx = b_meta.r_child_leaf;
    } else if(b_meta.r_child_addr == 0) {
      b_ptr.addr = b_meta.l_child_addr;
      b_ptr.leaf_idx = b_meta.l_child_leaf;
    } else {
      BlockPtr min_ptr = min_node(BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
      Block *min_node = get_block(min_ptr);

      b_ptr.addr = min_ptr.addr;
      b_ptr.leaf_idx = min_ptr.leaf_idx;

      V v = *(V*)(min_node->data);
      std::cout << v << "\n";
      BlockPtr new_right = remove(v, BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
      b_meta.r_child_addr = new_right.addr;
      b_meta.r_child_leaf = new_right.leaf_idx;

      b = min_node;
    }
  }
  serialize_metadata(b->metadata, b_meta);

  if(b_ptr.addr == 0) return BlockPtr(0, 0); // this means there was only one node, now empty

  // update height
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height; // this should be ok?
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);
  serialize_metadata(b->metadata, b_meta);

  // check balance + do rotations
  int balance = b_left_height - b_right_height;
  if(balance > 1 && get_balance(BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf)) >= 0) {
    return right_rotate(b_ptr);
  } else if(balance > 1 && get_balance(BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf)) < 0) {
    BlockPtr new_left = left_rotate(BlockPtr(b_left->addr, b_left->leaf_idx));
    b_meta.l_child_addr = new_left.addr;
    b_meta.l_child_leaf = new_left.leaf_idx;
    serialize_metadata(b->metadata, b_meta);
    return right_rotate(b_ptr);
  } else if(balance < -1 && get_balance(BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf)) <= 0) {
    return left_rotate(b_ptr);
  } else if(balance < -1 && get_balance(BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf)) > 0) {
    BlockPtr new_right = right_rotate(BlockPtr(b_right->addr, b_right->leaf_idx));
    b_meta.r_child_addr = new_right.addr;
    b_meta.r_child_leaf = new_right.leaf_idx;
    serialize_metadata(b->metadata, b_meta);
    return left_rotate(b_ptr);
  }

  return b_ptr;
}

template<typename V>
BlockPtr SetClient<V>::find(V v, BlockPtr b_ptr) {
  if(b_ptr.addr == 0) {
    return BlockPtr(0, 0);
  }

  Block *b = get_block(b_ptr);
  AVLMetadata b_meta = parse_metadata(b->metadata);

  V b_v = *(V*)(b->data);

  if(compare_value(v, b_v) < 0) return find(v, BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf));
  else if(compare_value(v, b_v) > 0) return find(v, BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf));
  else return b_ptr; // found it!
}

template<typename V>
BlockPtr SetClient<V>::right_rotate(BlockPtr b_ptr) {
  Block *b = get_block(b_ptr);
  AVLMetadata b_meta = parse_metadata(b->metadata);

  Block *new_root = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  AVLMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.l_child_addr = new_root_meta.r_child_addr; // replace new root w/ it's right child in old root
  b_meta.l_child_leaf = new_root_meta.l_child_addr;

  new_root_meta.r_child_addr = b_ptr.addr; // new root's right child is the old root
  new_root_meta.r_child_leaf = b_ptr.leaf_idx;

  // update heights
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);

  AVLMetadata null_metadata = {
    .l_child_leaf = 0,
    .l_child_addr = 0,
    .r_child_leaf = 0,
    .r_child_addr = 0,
    .height = 0,
  };
  
  AVLMetadata new_root_left_meta =
    (b_right == NULL) ? null_metadata : parse_metadata(get_block(new_root_meta.l_child_addr,
							     new_root_meta.l_child_leaf)->metadata);
  new_root_meta.height = 1 + std::max(new_root_left_meta.height, b_meta.height);

  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  return BlockPtr(new_root->addr, new_root->leaf_idx);
}

template<typename V>
BlockPtr SetClient<V>::left_rotate(BlockPtr b_ptr) {
  Block *b = get_block(b_ptr);
  AVLMetadata b_meta = parse_metadata(b->metadata);

  Block *new_root = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);
  AVLMetadata new_root_meta = parse_metadata(new_root->metadata);

  b_meta.r_child_addr = new_root_meta.l_child_addr; // replace new root w/ it's left child in old root
  b_meta.r_child_leaf = new_root_meta.l_child_leaf;

  new_root_meta.l_child_addr = b_ptr.addr; // new root's left child is the old root
  new_root_meta.l_child_leaf = b_ptr.leaf_idx;

  // update heights
  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;
  b_meta.height = 1 + std::max(b_left_height, b_right_height);

  AVLMetadata null_metadata = {
    .l_child_leaf = 0,
    .l_child_addr = 0,
    .r_child_leaf = 0,
    .r_child_addr = 0,
    .height = 0,
  };
  
  AVLMetadata new_root_right_meta =
    (b_right == NULL) ? null_metadata : parse_metadata(get_block(new_root_meta.r_child_addr,
							     new_root_meta.r_child_leaf)->metadata);
  new_root_meta.height = 1 + std::max(b_meta.height, new_root_right_meta.height);

  serialize_metadata(b->metadata, b_meta);
  serialize_metadata(new_root->metadata, new_root_meta);

  return BlockPtr(new_root->addr, new_root->leaf_idx);
}

template<typename V>
int SetClient<V>::get_balance(BlockPtr b_ptr) {
  Block *b = get_block(b_ptr);
  AVLMetadata b_meta = parse_metadata(b->metadata);

  Block *b_left = get_block(b_meta.l_child_addr, b_meta.l_child_leaf);
  Block *b_right = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);

  int b_left_height = (b_left == NULL) ? 0 : parse_metadata(b_left->metadata).height;
  int b_right_height = (b_right == NULL) ? 0 : parse_metadata(b_right->metadata).height;

  int balance = b_left_height - b_right_height;
  return balance;
}

template<typename V>
BlockPtr SetClient<V>::min_node(BlockPtr b_ptr) {
  Block *b = get_block(b_ptr);
  AVLMetadata b_meta = parse_metadata(b->metadata);

  while(b_meta.r_child_addr != 0) {
    b = get_block(b_meta.r_child_addr, b_meta.r_child_leaf);
    b_meta = parse_metadata(b->metadata);
  }
  return BlockPtr(b->addr, b->leaf_idx);
}

template<typename V>
Block* SetClient<V>::get_block(BlockPtr b_ptr) {
  return get_block(b_ptr.addr, b_ptr.leaf_idx);
}

template<typename V>
Block* SetClient<V>::get_block(unsigned int addr, unsigned int leaf_idx) {
  if(addr == 0) return NULL;

  // check stash
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      it->in_use = true;
      flush_stash();
      return &(*it);
    }
  }

  // check server
  Block *b = NULL;
  get_blocks(leaf_idx);
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      it->in_use = true;
      b = &(*it);
      break;
    }
  }
  dump_stash(leaf_idx);

  if(b == NULL) { 
    std::cerr << "Non-NULL block not found in stash or on server...\n";
    std::abort();
    return NULL;
  }

  return b;
}

template<typename V>
AVLMetadata SetClient<V>::parse_metadata(char *buf) {
  AVLMetadata m;
  memcpy((char*)&m, buf, sizeof(AVLMetadata));
  return m;
}

template<typename V>
void SetClient<V>::serialize_metadata(char *buf, AVLMetadata m) {
  memcpy(buf, (char*)&m, sizeof(AVLMetadata));
}

template<typename V>
void SetClient<V>::prefix_print() {
  prefix_print(BlockPtr(root_addr, root_leaf));
  std::cout << "\n";
  for(auto it = stash.begin(); it != stash.end(); ++it) (*it).in_use = false;
}

template<typename V>
void SetClient<V>::prefix_print(BlockPtr b_ptr) {
  if(b_ptr.addr == 0) {
    return;
  }

  // Block *b = get_block(b_ptr);
  // std::cout << *(V*)(b->data) << " ";
  std::cout << "X";

  AVLMetadata b_meta = parse_metadata(get_block(b_ptr)->metadata);
  prefix_print(BlockPtr(b_meta.l_child_addr, b_meta.l_child_leaf)); // left
  prefix_print(BlockPtr(b_meta.r_child_addr, b_meta.r_child_leaf)); // right
}

template<typename V>
int SetClient<V>::compare_value(V v1, V v2) {
  if(v1 < v2) return -1;
  else if(v1 > v2) return 1;
  else return 0;
}


