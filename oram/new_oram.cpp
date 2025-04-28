#include "new_oram.h"

// Node::Node(int height, Node *parent) {
//   this->bucket = new Bucket();

//   this->l_child = NULL;
//   this->r_child = NULL;
//   this->parent = parent;

//   if(height != 0) {
//     this->l_child = new Node(height-1);
//     this->r_child = new Node(height-1);
//   }
// }

/* --------------------------------------------- */

ORAMClient::ORAMClient() {
  mappings = std::map<unsigned int, unsigned int>();
}

/* --------------------------------------------- */

ORAMServer::ORAMServer(int port) {
  // initialize tree structure
  root = new Node(L-1);

  // create RPC server
  server = rpc::server(port);
  // TODO bindings server.bind()
}
