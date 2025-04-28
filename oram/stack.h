#ifndef STACK_H
#define STACK_H

#include "oram.h"

class ObliviousStack : public ORAM {
 public:
  ObliviousStack();
  void push(char data[BLOCK_SIZE]);
  Block pop();
 private:
  unsigned int ctr;
  unsigned int last_leaf;
};
 // leaf can go out of date? but does this matter -> yeah

#endif
