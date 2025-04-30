#ifndef STACK_H
#define STACK_H

#include "oram.h"
#include "new_oram.h"

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

class StackClient : public ORAMClient {
public:
  StackClient(std::string server_ip, int port);
  void push(char data[BLOCK_SIZE]);
  int pop(char *buf);
  bool empty();
  int size();
private:
  unsigned int ctr;
  unsigned int last_leaf;

};

#endif
