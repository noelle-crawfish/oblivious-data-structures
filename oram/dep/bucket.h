
#ifndef BUCKET_H
#define BUCKET_H

#include <vector>

#include "block.h"

#define BUCKET_SIZE 16

class Bucket {
 public:
  Bucket();
  bool add_block(Block b);
  void clear();
 std::vector<Block> blocks;
};

#endif
