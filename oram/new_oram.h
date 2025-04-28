#ifndef ORAM2_H
#define ORAM2_H

#include <rpc/client.h>
#include <rpc/server.h>

#include "oram.h"

class ORAMClient {
 public:
  ORAMClient();
  int read(char *buf, unsigned int addr);
  void write(unsigned int addr, char data[BLOCK_SIZE]);
 private:
  void dump_stash(unsigned int leaf_idx); // TODO figure out how to do over network efficiently
  bool on_path_at_level(unsigned int idx1, unsigned int idx2, int level);
  unsigned int random_leaf_idx();

 std::map<unsigned int, unsigned int> mappings;
 std::vector<Block> stash;
};

class ORAMServer {
 public:
  ORAMServer(int port);
private:
  Node *root;
 rpc::server server;

};

#endif
