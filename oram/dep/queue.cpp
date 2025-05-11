#include <cstring>
#include <stdlib.h>

#include "queue.h"

QueueClient::QueueClient(std::string server_addr, int port) : ORAMClient(server_addr, port) {
  head = tail = 1; // head = next to pop, tail = next to add -> 0 addr is reserved
  front_leaf = next_leaf = 0;
}

void QueueClient::push(char data[BLOCK_SIZE]) {
  unsigned int leaf_idx = next_leaf;
  get_blocks(leaf_idx);

  next_leaf = random_leaf_idx();
  char metadata[METADATA_SIZE];
  memcpy(metadata, (char*)(&next_leaf), sizeof(unsigned int));

  stash.push_back(make_oram_block(0, tail++, leaf_idx, data, metadata));

  // write blocks from the stash back to the path
  dump_stash(leaf_idx);
}

int QueueClient::pop(char *buf) {
  if(head == tail) {
    return 0;
  }

  unsigned int leaf_idx = front_leaf;
  get_blocks(leaf_idx);
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == head) {
      front_leaf = *(unsigned int*)(&(*it).metadata);
      memcpy(buf, (*it).data, BLOCK_SIZE);
      stash.erase(it, std::next(it));
      break;
    }
  }

  dump_stash(leaf_idx);
  head++;

  return BLOCK_SIZE;
}
