#ifndef QUEUE_H
#define QUEUE_H

#include "oram.h"

class ObliviousQueue : public ORAM {
 public:
  ObliviousQueue();
  void push(char data[BLOCK_SIZE]);
  Block pop();
 private:
  unsigned int head;
  unsigned int tail;
  unsigned int front_leaf;
  unsigned int next_leaf;
};

#endif
