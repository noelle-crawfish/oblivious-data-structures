#ifndef STACK_H
#define STACK_H

#include "new_oram.h"

class StackClient : public ORAMClient {
public:
  StackClient(std::string server_ip, int port,
	      unsigned int levels, unsigned int bucket_size, unsigned int threshold);
  StackClient(std::string server_ip, int port) : StackClient(server_ip, port,
							     L, BUCKET_SIZE, STASH_THRESHOLD) {};
  void push(char data[BLOCK_SIZE]);
  int pop(char *buf);
  bool empty();
  int size();
  int stash_size();
private:
  unsigned int ctr;
  unsigned int last_leaf;

};

#endif
