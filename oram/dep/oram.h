
#ifndef ORAM_H
#define ORAM_H

#include <map>
#include <vector>

#include "block.h"
#include "bucket.h"

#define L 4
#define N_LEAVES (2 << L)

class Node {
 public:
  Node(int height, int path, Node *parent);
  Node(int height) : Node(height, 0, NULL) {};
  Node *l_child;
  Node *r_child;
  Node *parent;
  Bucket *bucket;
};

class ORAM {
 public:
  ORAM();
  Block read(unsigned int addr);
  int write(unsigned int addr, char data[BLOCK_SIZE], char metadata[METADATA_SIZE]);
 private:
  Node *root;
 std::map<unsigned int, unsigned int> mappings;
 std::vector<Block> stash;

  void traverse_path(unsigned int leaf_idx);
  void dump_stash(unsigned int leaf_idx);
  Node *get_leaf(unsigned int leaf_idx);
  bool on_path_at_level(unsigned int idx1, unsigned int idx2, int level);
  unsigned int random_leaf_idx();

};

void dump_blocks(Bucket *to, Bucket *from);

#endif
