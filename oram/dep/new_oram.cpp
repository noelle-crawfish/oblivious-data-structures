#include "new_oram.h"

#include <iostream>
#include <cstdarg>
#include <arpa/inet.h> // required for inet_pton
// TODO eventually move node here
 
#define TRACE 0 // comment thiis to disable trace

#ifdef TRACE // if you want actual msgs you have to use fprintf i think 
#define trace() std::cout << "\n This line hit: "<< __LINE__ << "\n"
#else
#define trace() do {} while(0)
#endif

/* --------------------------------------------- */

ORAMClient::ORAMClient(std::string server_ip, int port) {
  std::cout << server_ip << "\n";
  client_socket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
    perror("inet_pton");
    return;
  }
  
  int flag = 1;
  setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
  int temp = 1; 
  while(temp != 0) {
    temp = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
  }
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
  // char buf[sizeof(Cmd)];
  trace();

  Cmd cmd = {
    .opcode = DUMP_STASH,
    .block = Block(),
    .leaf_idx = leaf_idx,
  };
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);

  cmd.opcode = BLOCK;
  // cmd.leaf_idx = -1; // no longer used

  // std::cout << "Dumping stash for leaf idx " << leaf_idx << ": before dump size = " << stash.size() << "\n";

  for(int level = (L-1); level >= 0; --level) {
    for(int i = 0; i < BUCKET_SIZE; ++i) {
      bool found_block = false;
      for(unsigned int j = 0; j < stash.size(); ++j) {
	if(on_path_at_level(stash[j].leaf_idx, leaf_idx, level) && !stash[j].in_use) {
	  // std::cout << "(" << stash[j].addr << ", " << stash[j].leaf_idx << ") dumped @ level " << level << "\n";
	  cmd.block = stash[j];
	  stash.erase(stash.begin()+j, stash.begin()+j+1);
	  found_block = true;
	  break;
	}
      }
      if(!found_block) {
	cmd.block = Block();
	cmd.block.leaf_idx = 0;;
      }
      send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
    }
  }

  // std::cout << "Dumping stash: after dump size = " << stash.size() << "\n";

}


bool ORAMClient::on_path_at_level(unsigned int idx1, unsigned int idx2, int level) {
  // L is the bottom...
  idx1 >>= (L - level - 1);
  idx2 >>= (L - level - 1);

  return (idx1 == idx2);
}

unsigned int ORAMClient::random_leaf_idx() {
  return std::rand() % N_LEAVES;
}

void ORAMClient::get_blocks(unsigned int leaf_idx) {
  // std::cout << "get_blocks(" << leaf_idx << ")\n";
  char buf[sizeof(Cmd)];

  Block null_block;
  Cmd cmd = {
    .opcode = GET_BLOCKS,
    .block = null_block,
    .leaf_idx = leaf_idx,
  };
  trace();
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
  // std::cout << "Getting blocks...\n";
  for(int level = 0; level < L; ++level) {
    for(int j = 0; j < BUCKET_SIZE; ++j) {
      recv(client_socket, buf, sizeof(Cmd), 0);
      Block *b = &((Cmd*)buf)->block;
      if(b->addr != 0) stash.push_back(*b);
      // std::cout << "(" << b->addr << ", " << b->leaf_idx << ") ";
    }
    // std::cout << "\n";
  }
}

void ORAMClient::exit() {
  Block null_block;
  Cmd cmd = {
    .opcode = EXIT,
    .block = null_block,
    .leaf_idx = 0,
  };
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
  close(client_socket);
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
  setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

  int sndbuf_size = sizeof(Cmd); // idek if this is needed
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

    trace();
    if(bytes > 0) {
      Cmd *cmd = (Cmd*)buf;

      switch(cmd->opcode) {
      case GET_BLOCKS:
	get_blocks(cmd->leaf_idx);
	break;
      case DUMP_STASH:
	dump_stash(cmd->leaf_idx);
	break;
      case EXIT:
	done = true;
        close(client_socket);
      default:
	break;
      }
    }

  }
}

void ORAMServer::dump_stash(unsigned int leaf_idx) {
  trace();
  char buf[sizeof(Cmd)];

  Node *curr = get_leaf(leaf_idx);
  assert(curr != NULL);

  std::cout << "---------- STASH DUMP IDX " << leaf_idx << " ----------\n";
  while(curr != NULL) {
    for(int i = 0; i < BUCKET_SIZE; ++i) {
      recv(client_socket, buf, sizeof(Cmd), 0);
      Cmd cmd = *(Cmd*)buf;

      assert(cmd.opcode == BLOCK);
      curr->bucket->blocks.push_back(cmd.block);
      std::cout << "(" << cmd.block.addr << ", " << cmd.block.leaf_idx << ") ";
    }
    std::cout << "\n";
    assert(curr->bucket->blocks.size() == BUCKET_SIZE);
    curr = curr->parent;
 }
 std::cout << "-----------------------------------------------\n";
}

void ORAMServer::get_blocks(unsigned int leaf_idx) {
  trace();
  Node *curr = root;
  std::cout << "---------- GET BLOCKS IDX " << leaf_idx << " ----------\n";
  // while(curr != NULL) {
  for(int i = L-1; i >= 0; --i) { // 2, 1, 0
    for(auto it = curr->bucket->blocks.begin(); it != curr->bucket->blocks.end(); ++it) {
      Cmd cmd = {
	.opcode = BLOCK,
	.block = *it,
	.leaf_idx = 0, // unused
      };
      send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
      std::cout << "(" << cmd.block.addr << ", " << cmd.block.leaf_idx << ") ";
    }
    std::cout << "\n";
    curr->bucket->clear();

    if((leaf_idx & (0x01 << (i-1))) == 0) curr = curr->l_child;
    else curr = curr->r_child;
    // leaf_idx >>= 1;
  }
  std::cout << "-----------------------------------------------\n";
}

Node* ORAMServer::get_leaf(unsigned int leaf_idx) {
  Node *curr = root;
  for(int i = L-2; i >= 0; --i) {
    std::cout << i << " ";
    if((leaf_idx & (0x01 << i)) == 0) {
      curr = curr->l_child;
      std::cout << "left -> ";
    } else {
      curr = curr->r_child;
      std::cout << "right -> ";
    }
    // leaf_idx >>= 1;
  }
  std::cout << "\n";

  return curr;
}
