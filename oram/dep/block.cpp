
#include <cstring>

#include "block.h"

// TODO this should've been a struct not a class :skull:

Block::Block() {
  this->nonce = 0;
  this->addr = 0;
  this->leaf_idx = 0;
  memset(this->data, 0, BLOCK_SIZE);
  in_use = false;
}

Block::Block(unsigned int addr, char data[BLOCK_SIZE]) {
  this->nonce = 0;
  this->addr = addr;
  this->leaf_idx = 0; // Initialized after its pushed on to the stash, before its back from the stash. 
  memcpy(this->data, data, BLOCK_SIZE);
  in_use = false;
}
