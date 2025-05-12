#include <cstring>
#include <stdlib.h>

#include "queue.h"

QueueClient::QueueClient(std::string server_addr, int port) : ORAMClient(server_addr, port) {
  head = tail = 1; // head = next to pop, tail = next to add -> 0 addr is reserved
  front_leaf = next_leaf = 0;
}

void QueueClient::push(char data[BLOCK_SIZE]) {
  unsigned int leaf_idx = next_leaf;

  next_leaf = random_leaf_idx();
  char metadata[METADATA_SIZE];
  memcpy(metadata, (char*)(&next_leaf), sizeof(unsigned int));

  Block new_block;
  make_oram_block(new_block, 0, tail++, leaf_idx, data, metadata);
  new_block.in_use = false;
  stash.push_back(new_block);

  flush_stash();
}

int QueueClient::pop(char *buf) {
  if(head == tail) {
    return 0;
  }

  unsigned int leaf_idx = front_leaf;

  Block *b = get_block(head, leaf_idx);
  front_leaf = *(unsigned int*)(b->metadata);
  memcpy(buf, b->data, BLOCK_SIZE);

  delete_block(b->addr);
  head++;
  flush_stash();
  return BLOCK_SIZE;
}
