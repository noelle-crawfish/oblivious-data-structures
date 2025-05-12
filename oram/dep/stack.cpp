#include "stack.h"

StackClient::StackClient(std::string server_ip, int port,
			 unsigned int levels, unsigned int bucket_size, unsigned int threshold) :
			   ORAMClient(server_ip, port, levels, bucket_size, threshold) {
  ctr = 0;
  last_leaf = 0;
}

void StackClient::push(char data[BLOCK_SIZE]) {
  unsigned int leaf_idx = random_leaf_idx();

  char metadata[METADATA_SIZE];
  memcpy(metadata, (char*)(&last_leaf), sizeof(unsigned int));

  Block new_block;
  make_oram_block(new_block, 0, ++ctr, leaf_idx, data, metadata);
  stash.push_back(new_block);
  Block *b = &stash.back();
  b->in_use = false;

  last_leaf = leaf_idx;

  flush_stash();
}

int StackClient::pop(char *buf) {
  if(ctr == 0) {
    return 0;
  }

  unsigned int leaf_idx = last_leaf;
  Block *b = get_block(ctr, leaf_idx);
  if(b->addr == 0) return 0;
  else memcpy(buf, b->data, BLOCK_SIZE);

  last_leaf = *(unsigned int*)(b->metadata);

  ctr--;
  flush_stash();
  return BLOCK_SIZE;
}

bool StackClient::empty() {
  return ctr == 0;
}

int StackClient::size() {
  return ctr;
}

int StackClient::stash_size() {
  return stash.size();
}
