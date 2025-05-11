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

#include <map>
#include <vector>

#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "bucket.h"

#define L 4
#define N_LEAVES (2 << (L-1) )

#define STASH_THRESHOLD 5

// 16 bytes = 128 bits -> std for AES

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

class Node {
 public:
  Node(int height, int path, Node *parent);
  Node(int height) : Node(height, 0, NULL) {};
  Node *l_child;
  Node *r_child;
  Node *parent;
  Bucket *bucket;
};

class ORAMClient {
 public:
  ORAMClient(std::string server_ip, int port);
  int read(char *buf, unsigned int addr);
  void write(unsigned int addr, char data[BLOCK_SIZE]);
  void exit();
  void init_tree();
 protected:
  void dump_stash(unsigned int leaf_idx); // interface with server to dump stash
  bool on_path_at_level(unsigned int idx1, unsigned int idx2, int level);
  unsigned int random_leaf_idx();
  void get_blocks(unsigned int leaf_idx); // modifies the stash
  Block encrypt_block(Block b);
  Block decrypt_block(Block b);
  void fill_random_data(char *buf, unsigned int num_bytes);
  void flush_stash();

  std::vector<unsigned char> *key;
  std::vector<unsigned char> *iv;
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
  void populate_tree(Node *root);
  void clear_tree(Node *root); 
};

#endif
