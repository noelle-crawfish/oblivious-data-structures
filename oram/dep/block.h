
#ifndef BLOCK_H
#define BLOCK_H

#define BLOCK_SIZE 1024
#define METADATA_SIZE 512

class Block {
 public:
  Block();
  Block(unsigned int addr, char data[BLOCK_SIZE]);
  unsigned int nonce;
  unsigned int addr;
  unsigned int leaf_idx;
  char data[BLOCK_SIZE];
  char metadata[METADATA_SIZE];
  bool in_use;
};

#endif
