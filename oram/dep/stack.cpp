#include "stack.h"

StackClient::StackClient(std::string server_ip, int port) : ORAMClient(server_ip, port) {
  ctr = 0;
  last_leaf = 0;
}

void StackClient::push(char data[BLOCK_SIZE]) {
  unsigned int leaf_idx = random_leaf_idx();
  get_blocks(leaf_idx);

  char metadata[METADATA_SIZE];
  memcpy(metadata, (char*)(&last_leaf), sizeof(unsigned int));

  stash.push_back(make_oram_block(0, ++ctr, leaf_idx, data, metadata));
  Block *b = &stash.back();

  last_leaf = b->leaf_idx;

  // write blocks from the stash back to the path
  dump_stash(leaf_idx);
}

int StackClient::pop(char *buf) {
  if(ctr == 0) {
    return 0;
  }

  unsigned int leaf_idx = last_leaf;
  get_blocks(leaf_idx);

  for(auto it = stash.begin(); it != stash.end(); ++it) {
    // TODO need some decoding ot happen
    if(it->addr == ctr) {
      last_leaf = *(unsigned int*)(&(*it).metadata);
      memcpy(buf, (*it).data, BLOCK_SIZE);
      stash.erase(it, std::next(it));
      break;
    }
  }

  dump_stash(leaf_idx);
  ctr--;

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
