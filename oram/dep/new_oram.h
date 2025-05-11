#ifndef ORAM2_H
#define ORAM2_H

#include <list>

/* #include <rpc/client.h> */
/* #include <rpc/server.h> */

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>

#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "oram.h"

enum Opcode {
  BLOCK, // single block in a sequence (use after starting bulk send)
  GET_BLOCKS,
  DUMP_STASH, 
  EXIT,
  POPULATE_TREE,
};

struct Cmd {
  Opcode opcode;
  Block block;
  unsigned int leaf_idx;
};

class ORAMClient {
 public:
  ORAMClient(std::string server_ip, int port);
  int read(char *buf, unsigned int addr);
  void write(unsigned int addr, char data[BLOCK_SIZE]);
  void exit();
  void initTree();
 protected:
  void dump_stash(unsigned int leaf_idx); // interface with server to dump stash
  bool on_path_at_level(unsigned int idx1, unsigned int idx2, int level);
  unsigned int random_leaf_idx();
  void get_blocks(unsigned int leaf_idx); // modifies the stash

 std::map<unsigned int, unsigned int> mappings;
 std::list<Block> stash;
 int client_socket;
};

class ORAMServer {
 public:
  ORAMServer(uint16_t port);

  void run(); // run server, this should be the last thing called, or in another thread
 private:
  Node *root;
  int client_socket;

  void dump_stash(unsigned int leaf_idx); // receive client dump stash request and refill buckets
  void get_blocks(unsigned int leaf_idx);
  Node *get_leaf(unsigned int leaf_idx);
  void populate_tree();
  void clear_tree(Node *root); 
};

#endif
