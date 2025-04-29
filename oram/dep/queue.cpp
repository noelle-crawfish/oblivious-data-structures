#include <cstring>
#include <stdlib.h>

#include "queue.h"

ObliviousQueue::ObliviousQueue() {
  head = 0;
  tail = 0;
  next_leaf = std::rand() % N_LEAVES;
  front_leaf = 0;
}

void ObliviousQueue::push(char data[BLOCK_SIZE]) {
  unsigned int addr = tail++;
  unsigned int leaf_idx = next_leaf;

  next_leaf = std::rand() % N_LEAVES;

  char metadata[METADATA_SIZE];
  memset(metadata, 0, METADATA_SIZE);
  memcpy(metadata, (char*)&next_leaf, sizeof(next_leaf));

  write(addr, data, metadata); 

  if(head == tail-1) front_leaf = leaf_idx;
}

Block ObliviousQueue::pop() {
  Block b = read(head++);
  front_leaf = *(unsigned int*)(b.metadata);
  return b;
}
