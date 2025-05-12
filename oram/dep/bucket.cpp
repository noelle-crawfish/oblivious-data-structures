#include "bucket.h"

#include <iostream>

Bucket::Bucket() {
  blocks = *(new std::vector<Block>());
}

bool Bucket::add_block(Block b) {
  if(blocks.size() < BUCKET_SIZE) {
    blocks.push_back(b);
    return true;
  } else return false;
}

void Bucket::clear() {
  blocks.erase(blocks.begin(), blocks.end());
}

void make_oram_block(Block &b, unsigned int nonce, unsigned int addr, unsigned int leaf_idx,
		      char data[BLOCK_SIZE], char metadata[METADATA_SIZE]) {
  b.nonce = nonce;
  b.addr = addr;
  b.leaf_idx = leaf_idx;
  memcpy(&b.data, data, BLOCK_SIZE);
  memcpy(&b.metadata, metadata, METADATA_SIZE);
  b.in_use = 0;
}
