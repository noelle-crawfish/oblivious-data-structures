#ifndef QUEUE_H
#define QUEUE_H

#include "new_oram.h"

class QueueClient : public ORAMClient {
public:
  QueueClient(std::string server_addr, int port);
  void push(char data[BLOCK_SIZE]);
  int pop(char *buf);

private:
  unsigned int head, tail, front_leaf, next_leaf;
};

#endif
