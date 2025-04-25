
#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE 1024

class Block {
 public:
  Block();
  Block(unsigned int addr, char data[BLOCK_SIZE]);
  unsigned int addr;
  unsigned int leaf_idx;
  char data[BLOCK_SIZE];
};

#endif
