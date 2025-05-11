
#ifndef BUCKET_H
#define BUCKET_H

#include <cstring>
#include <vector>

#define BUCKET_SIZE 16

#define BLOCK_SIZE 1024
#define METADATA_SIZE 512

struct Block {
  unsigned int nonce;
  unsigned int addr;
  unsigned int leaf_idx;
  char data[BLOCK_SIZE];
  char metadata[METADATA_SIZE];
  bool in_use;
};

Block make_block(unsigned int nonce, unsigned int addr, unsigned int leaf_idx, char data[BLOCK_SIZE],
		 char metadata[METADATA_SIZE]) {
  Block b;
  b.nonce = nonce;
  b.addr = addr;
  b.leaf_idx = leaf_idx;
  memcpy(b.data, data, BLOCK_SIZE);
  memcpy(b.metadata, metadata, METADATA_SIZE);
  b.in_use = 0;
}

class Bucket {
 public:
  Bucket();
  bool add_block(Block b);
  void clear();
 std::vector<Block> blocks;
};

#endif
