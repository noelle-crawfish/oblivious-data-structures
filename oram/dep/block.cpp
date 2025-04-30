
#include <cstring>

#include "block.h"

Block::Block() {
  this->addr = 0;
  this->leaf_idx = 0;
  memset(this->data, 0, BLOCK_SIZE);
}

Block::Block(unsigned int addr, char data[BLOCK_SIZE]) {
  this->addr = addr;
  this->leaf_idx = 0; // Initialized after its pushed on to the stash, before its back from the stash. 
  memcpy(this->data, data, BLOCK_SIZE);
}
