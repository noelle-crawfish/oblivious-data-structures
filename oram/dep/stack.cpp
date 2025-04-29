#include <cstring>

#include "stack.h"

ObliviousStack::ObliviousStack() {
  ctr = 0;
  last_leaf = -1;
}

void ObliviousStack::push(char data[BLOCK_SIZE]) {
  unsigned int addr = ctr++;

  char metadata[METADATA_SIZE];
  memset(metadata, 0, METADATA_SIZE);
  memcpy(metadata, (char*)&addr, sizeof(addr));

  last_leaf = write(addr, data, metadata); 
}

Block ObliviousStack::pop() {
  ctr--;
  Block b = read(ctr);
  last_leaf = *(unsigned int*)(b.metadata);
  return b;
}

StackClient::StackClient(std::string server_ip, int port) : ORAMClient(server_ip, port) {

}

void StackClient::push(char data[BLOCK_SIZE]) {

}

int StackClient::pop(char *buf) {
  if(ctr == 0) {
    return 0;
  }

  unsigned int leaf_idx = last_leaf;
  get_blocks(leaf_idx);

  for(auto it = stash.begin(); it != stash.end(); ++it) {
    // TODO need some decoding ot happen
    if(it->addr == ctr) {
      last_leaf = *(unsigned int*)(&(*it).metadata);
      memcpy(buf, (*it).data, BLOCK_SIZE);
      stash.erase(it, it+1);
      break;
    }
  }

  dump_stash(leaf_idx);

  return BLOCK_SIZE;
}
