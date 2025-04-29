#include "new_oram.h"

// TODO eventually move node here

/* --------------------------------------------- */

ORAMClient::ORAMClient(std::string server_ip, int port) {
  client_socket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY; // server_ip; // INADDR_ANY? for testing...

  connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

  mappings = std::map<unsigned int, unsigned int>();
}

int ORAMClient::read(char *buf, unsigned int addr) {

  Block *b = NULL;

  unsigned int leaf_idx = mappings.at(addr);

  // std::vector<Block> = get_blocks(unsigned int leaf_idx); -> TODO this should be an rpc call
  // then add to stash

  for(auto it = stash.begin(); it != stash.end(); ++it) {
    // TODO need some decoding ot happen
    if(it->addr == addr) {
      b = &(*it);
      break;
    }
  }

  if(b == NULL) {
    return 0;
  }

  memcpy(buf, b->data, BLOCK_SIZE);

  b->leaf_idx = random_leaf_idx();
  mappings[addr] = b->leaf_idx;

  dump_stash(leaf_idx);

  return BLOCK_SIZE;
}

void ORAMClient::write(unsigned int addr, char data[BLOCK_SIZE]) {
  Block *b = NULL;
  unsigned int leaf_idx = mappings.at(addr);

  // RPC get blocks, add to stash -> what about response RPC add to stash by many RPC from .

  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      b = &(*it);
      memcpy(b->data, data, BLOCK_SIZE);
      break;
    }
  }

  if(b == NULL) stash.push_back(Block(addr, data));
  b = &stash.back();

  b->leaf_idx = random_leaf_idx();
  mappings[addr] = b->leaf_idx;

  dump_stash(leaf_idx);
}


void ORAMClient::dump_stash(unsigned int leaf_idx) {
  // TODO 
  for(int level = 0; level < L; ++level) {
    // TODO is level 0 bottom or top?

    // find up to Z blocks on_path_at_level(b->leaf_idx, leaf_idx, level);
    // if needed pad with empty Block()
    // send over RPC to server
  }
}


bool ORAMClient::on_path_at_level(unsigned int idx1, unsigned int idx2, int level) {
  idx1 >>= (L - level);
  idx2 >>= (L - level);

  return (idx1 == idx2);
}

unsigned int ORAMClient::random_leaf_idx() {
  return std::rand() % N_LEAVES;
}

/* --------------------------------------------- */

ORAMServer::ORAMServer(uint16_t port) {
  root = new Node(L-1);

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  listen(server_socket, 5);

  client_socket = accept(server_socket, nullptr, nullptr);
}

void ORAMServer::run() {
  // server.run();
};

void ORAMServer::dump_stash(unsigned int leaf_idx) {
  char buf[sizeof(Cmd)];

  Node *curr = get_leaf(leaf_idx);
  while(curr != NULL) {
    for(int i = 0; i < BUCKET_SIZE; ++i) {
      recv(client_socket, buf, sizeof(Cmd), 0);
      Cmd cmd = *(Cmd*)buf;

      assert(cmd.opcode == BLOCK);
      curr->bucket->blocks.push_back(cmd.block);
    }
    assert(curr->bucket->blocks.size() == BUCKET_SIZE);
    curr = curr->parent;
 }
}

void ORAMServer::get_blocks(unsigned int leaf_idx) {
  Node *curr = root;
  while(curr != NULL) {
    for(auto it = curr->bucket->blocks.begin(); it != curr->bucket->blocks.end(); ++it) {
      Cmd cmd = {
	.opcode = BLOCK,
	.block = *it,
      };
      send(client_socket, (char*)(&cmd), sizeof(cmd), 0);
    }
    curr->bucket->clear();

    if((leaf_idx & 0x01) == 0) curr = curr->l_child;
    else curr = curr->r_child;
    leaf_idx >>= 1;
  }
}

Node* ORAMServer::get_leaf(unsigned int leaf_idx) {
  Node *curr = root;
  for(int i = 0; i < L; ++i) {
    if((leaf_idx & 0x01) == 0) curr = curr->l_child;
    else curr = curr->r_child;
    leaf_idx >>= 1;
  }

  return curr;
}
