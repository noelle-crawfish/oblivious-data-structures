#include "new_oram.h"

#include <iostream>

// TODO eventually move node here

void flush(int socket) {
  char *null_char = "\0";
  send(socket, null_char, 1, MSG_DONTWAIT);
}

/* --------------------------------------------- */

ORAMClient::ORAMClient(std::string server_ip, int port) {
  client_socket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY; // server_ip; // INADDR_ANY? for testing...

  
  int flag = 1;
  if(setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int))) {
    std::cout << "No Nagles\n";
  }

  connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

  mappings = std::map<unsigned int, unsigned int>();
}

int ORAMClient::read(char *buf, unsigned int addr) {

  Block *b = NULL;

  unsigned int leaf_idx = mappings.at(addr);
  get_blocks(leaf_idx);

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
  unsigned int leaf_idx = mappings[addr]; // use [] instead of .at() in case this is first write

  get_blocks(leaf_idx);

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
  std::cout << "Dumping stash -- or am I? \n";
  // char buf[sizeof(Cmd)];

  Cmd cmd = {
    .opcode = DUMP_STASH,
    .block = Block(),
    .leaf_idx = leaf_idx,
  };
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);

  // for(int level = 0; level < L; ++level) {
  for(int level = 0; level < L; ) {// ++level) {
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

void ORAMClient::get_blocks(unsigned int leaf_idx) {
  std::cout << "Asking for blocks\n";
  char buf[sizeof(Cmd)];

  Cmd cmd = {
    .opcode = GET_BLOCKS,
    .leaf_idx = leaf_idx,
  };
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);

  for(int level = 0; level < L; ++level) {
    for(int j = 0; j < BUCKET_SIZE; ++j) {
      int bytes = recv(client_socket, buf, sizeof(Cmd), 0);
      std::cout << "Recieved block #" << level*BUCKET_SIZE + j << " " << bytes << "\n";
      stash.push_back(((Cmd*)buf)->block);
    }
  }
}

void ORAMClient::exit() {
  Cmd cmd = {
    .opcode = EXIT,
  };
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
}

/* --------------------------------------------- */

ORAMServer::ORAMServer(uint16_t port) {
  root = new Node(L-1);

  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;


  int flag = 1;
  if(setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int))) {
    std::cout << "NO nagle's\n";
  }

  int sndbuf_size = 4*sizeof(Cmd); // 64 KB
  setsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, sizeof(sndbuf_size));

  bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  listen(server_socket, 5);

  client_socket = accept(server_socket, nullptr, nullptr);
}

void ORAMServer::run() {
  bool done = false;

  while(!done) {
    char buf[sizeof(Cmd)];
    int bytes = recv(client_socket, buf, sizeof(Cmd), 0);

    if(bytes > 0) {
      std::cout << bytes << "\n";
      Cmd *cmd = (Cmd*)buf;

      switch(cmd->opcode) {
      case GET_BLOCKS:
	get_blocks(cmd->leaf_idx);
	std::cout << "done giving blocks\n";
	break;
      case DUMP_STASH:
	dump_stash(cmd->leaf_idx);
	break;
      case EXIT:
	done = true;
      default:
	break;
      }
    }

  }
  std::cout << "Exiting run\n";
}

void ORAMServer::dump_stash(unsigned int leaf_idx) {
  std::cout << "Waiting for stash dump...\n";
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
  std::cout << "get_blocks(" << leaf_idx << ");\n";
  Node *curr = root;
  while(curr != NULL) {
    for(auto it = curr->bucket->blocks.begin(); it != curr->bucket->blocks.end(); ++it) {
      std::cout << "HERE\n";
      Cmd cmd = {
	.opcode = BLOCK,
	.block = *it,
      };
      send(client_socket, (char*)(&cmd), sizeof(Cmd), MSG_DONTWAIT);
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
