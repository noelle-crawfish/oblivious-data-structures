#ifndef QUEUE_H
#define QUEUE_H

#include "oram.h"
#include "new_oram.h"

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

class QueueClient : public ORAMClient {
public:
  QueueClient(std::string server_addr, int port);
  void push(char data[BLOCK_SIZE]);
  int pop(char *buf);

private:
  unsigned int head, tail, front_leaf, next_leaf;
};

#endif
