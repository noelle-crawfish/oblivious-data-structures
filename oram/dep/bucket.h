
#ifndef BUCKET_H
#define BUCKET_H

#include <cstring>
#include <vector>

#define BUCKET_SIZE 16

#define BLOCK_SIZE 1024
#define METADATA_SIZE 512
#define AES_BLOCK_SIZE 16

struct Block {
  unsigned int nonce;
  unsigned int addr;
  unsigned int leaf_idx;
  char data[BLOCK_SIZE];
  char metadata[METADATA_SIZE];
  bool in_use;
  // char padding[16];
};

static_assert(sizeof(Block) % AES_BLOCK_SIZE == 0);

void make_oram_block(Block b, unsigned int nonce, unsigned int addr, unsigned int leaf_idx,
		      char data[BLOCK_SIZE], char metadata[METADATA_SIZE]);

class Bucket {
 public:
  Bucket();
  bool add_block(Block b);
  void clear();
 std::vector<Block> blocks;
};

#endif
