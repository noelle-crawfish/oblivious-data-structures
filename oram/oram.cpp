#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdlib.h>

#include "oram.h"


Node::Node(int height, Node *parent) {
  this->bucket = new Bucket();

  this->l_child = NULL;
  this->r_child = NULL;
  this->parent = parent;

  if(height != 0) {
    this->l_child = new Node(height-1);
    this->r_child = new Node(height-1);
  }
}

ORAM::ORAM() {
  root = new Node(L-1);
  mappings = std::map<unsigned int, unsigned int>();
}

Block ORAM::read(unsigned int addr) {
  Block *b = NULL;

  // lookup path containing address -> TODO recursive ORAM?
  unsigned int leaf_idx = mappings.at(addr);

  // dump all blocks on path to stash
  traverse_path(leaf_idx);

  // identify our block of interest
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      b = &(*it);
      break;
    }
  }

  // update mapping
  b->leaf_idx = random_leaf_idx();
  mappings[addr] = leaf_idx;

  // write blocks from the stash back to the path
  dump_stash(leaf_idx);

  return *b;
}

void ORAM::write(unsigned int addr, char data[BLOCK_SIZE]) {
  Block *b = NULL;

  // lookup path containing address -> TODO recursive ORAM?
  unsigned int leaf_idx = mappings[addr]; // purposefully no using .at() b/c what if it's the first write...

  // dump all blocks on path to stash
  traverse_path(leaf_idx);

  // identify our block of interest
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      b = &(*it);
      memcpy(b->data, data, BLOCK_SIZE);
      break;
    }
  }
  if(b == NULL) {
    stash.push_back(Block(addr, data));
    b = &stash[stash.size()-1];
  }

  // update mapping
  b->leaf_idx = random_leaf_idx();
  mappings[addr] = leaf_idx;

  // write blocks from the stash back to the path
  dump_stash(leaf_idx);
}

void ORAM::traverse_path(unsigned int leaf_idx) {
  Node *curr = root;
  for(int i = 0; i < L; ++i) {
    for(auto it = curr->bucket->blocks.begin(); it != curr->bucket->blocks.end(); ++it) {
      stash.push_back(*it);
    }
    curr->bucket->clear();

    if((leaf_idx & 0x01) == 0) curr = curr->l_child;
    else curr = curr->r_child;
    leaf_idx >>= 1;
  }
}

void ORAM::dump_stash(unsigned int leaf_idx) {
  // start at bottom and work our way up, returning oram blocks
  Node *curr = get_leaf(leaf_idx);
  int level = L;

  while(curr != NULL) {
    std::copy_if(stash.begin(), stash.end(), std::back_inserter(curr->bucket->blocks),
		 [this, leaf_idx, level](Block b) { return on_path_at_level(b.leaf_idx, leaf_idx, level); });
    stash.erase(std::remove_if(stash.begin(), stash.end(),
			       [this, leaf_idx, level](Block b) {
				 return on_path_at_level(b.leaf_idx, leaf_idx, level);
			       }), stash.end());

    // If we copied too many, return them
    int overflow = BUCKET_SIZE - curr->bucket->blocks.size();
    if(overflow > 0) {
      auto new_end = curr->bucket->blocks.end() - overflow;
      std::copy(new_end, curr->bucket->blocks.end(), std::back_inserter(curr->bucket->blocks));
      curr->bucket->blocks.erase(new_end, curr->bucket->blocks.end());
    }

    curr = curr->parent;
    level -= 1;
  }
}

Node *ORAM::get_leaf(unsigned int leaf_idx) {
  Node *curr = root;
  for(int i = 0; i < L; ++i) {
    if((leaf_idx & 0x01) == 0) curr = curr->l_child;
    else curr = curr->r_child;
    leaf_idx >>= 1;
  }

  return curr;
}

bool ORAM::on_path_at_level(unsigned int idx1, unsigned int idx2, int level) {
  Node *curr1 = root, *curr2 = root;
  for(int i = 0; i < level; ++i) {
    if((idx1 & 0x01) == 0) curr1 = curr1->l_child;
    else curr1 = curr1->r_child;
    if((idx2 & 0x01) == 0) curr2 = curr2->l_child;
    else curr2 = curr2->r_child;
  }
  return curr1 == curr2;
}

unsigned int ORAM::random_leaf_idx() {
  return std::rand() % N_LEAVES;
}
