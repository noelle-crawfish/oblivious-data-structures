#include "bucket.h"

Bucket::Bucket() {
  blocks = std::vector<Block>();

}

bool Bucket::add_block(Block b) {
  if(blocks.size() < BUCKET_SIZE) {
    blocks.push_back(b);
    return true;
  } else return false;
}

void Bucket::clear() {
  blocks.erase(blocks.begin(), blocks.end());
}
