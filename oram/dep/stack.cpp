#include <cstring>

#include "stack.h"

ObliviousStack::ObliviousStack() {
  ctr = 0;
  last_leaf = -1;
}

void ObliviousStack::push(char data[BLOCK_SIZE]) {
  unsigned int addr = ctr++;

  char metadata[METADATA_SIZE];
  memset(metadata, 0, METADATA_SIZE);
  memcpy(metadata, (char*)&addr, sizeof(addr));

  last_leaf = write(addr, data, metadata); 
}

Block ObliviousStack::pop() {
  ctr--;
  Block b = read(ctr);
  last_leaf = *(unsigned int*)(b.metadata);
  return b;
}
