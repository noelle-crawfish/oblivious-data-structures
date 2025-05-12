
#include <iostream>
#include <queue>
#include <random>
#include <cstdarg>
#include <arpa/inet.h> // required for inet_pton
// TODO eventually move node here

#include <openssl/evp.h>
#include <openssl/rand.h>
 
#include "new_oram.h"
// #define TRACE 0 // comment thiis to disable trace

#ifdef TRACE // if you want actual msgs you have to use fprintf i think 
#define trace() std::cout << "\n This line hit: "<< __LINE__ << "\n"
#else
#define trace() do {} while(0)
#endif

/* --------------------------------------------- */

Node::Node(int height, int path, Node *parent, unsigned int levels, unsigned int bucket_size) {
  unsigned int leaf_idx = (unsigned int)path << (levels-height);
  this->bucket = new Bucket(bucket_size);

  Block b;
  b.addr = 0;
  b.leaf_idx = leaf_idx;
  
  for(int i = 0; i < bucket_size; ++i) {
    bucket->blocks.push_back(b);
  }

  this->l_child = NULL;
  this->r_child = NULL;
  this->parent = parent;

  if(height != 0) {
    this->l_child = new Node(height-1, (path << 1), this, levels, bucket_size);
    this->r_child = new Node(height-1, (path << 1) | 0x01, this, levels, bucket_size);
  }
}

/* --------------------------------------------- */

ORAMClient::ORAMClient(std::string server_ip, int port, unsigned int levels, unsigned int bucket_size,
		       unsigned int threshold) {
  this->levels = levels;
  n_leaves = (2 << (levels-1));
  this->bucket_size = bucket_size;
  this->stash_threshold = threshold;

  // generate AES key
  key = new std::vector<unsigned char>(32); 
  iv = new std::vector<unsigned char>(16); 

  RAND_bytes(key->data(), key->size());
  RAND_bytes(iv->data(), iv->size());

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

  init_tree();
}

int ORAMClient::read(char *buf, unsigned int addr) {

  Block *b = NULL;

  unsigned int leaf_idx = mappings.at(addr);
  get_blocks(leaf_idx);

  for(auto it = stash.begin(); it != stash.end(); ++it) {
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

  // if(b == NULL) stash.push_back(Block(addr, data));
  char null_metadata[METADATA_SIZE];
  if(b == NULL) {
    Block new_block;
    make_oram_block(new_block, 0, addr, leaf_idx, data, null_metadata);
    stash.push_back(new_block);
  }
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

  for(int level = (levels-1); level >= 0; --level) {
    for(int i = 0; i < bucket_size; ++i) {
      bool found_block = false;
      for(unsigned int j = 0; j < stash.size(); ++j) {
	auto stash_at_j = stash.begin();
	std::advance(stash_at_j, j);
	if(on_path_at_level(stash_at_j->leaf_idx, leaf_idx, level) && !stash_at_j->in_use) {
	  // std::cout << "(" << stash_at_j->addr << ", " << stash_at_j->leaf_idx << ") dumped @ level " << level << "\n";
	  cmd.block = *stash_at_j;
	  cmd.block.nonce += 1;
	  cmd.block = encrypt_block(cmd.block);

	  stash.erase(stash_at_j, std::next(stash_at_j));
	  found_block = true;
	  break;
	}
      }
      if(!found_block) {
	cmd.block = Block();
	cmd.block.leaf_idx = 0;
	fill_random_data(cmd.block.data, BLOCK_SIZE);
	cmd.block = encrypt_block(cmd.block);
      }
      send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
    }
  }

}


bool ORAMClient::on_path_at_level(unsigned int idx1, unsigned int idx2, int level) {
  // L is the bottom...
  idx1 >>= (levels - level - 1);
  idx2 >>= (levels - level - 1);

  return (idx1 == idx2);
}

unsigned int ORAMClient::random_leaf_idx() {
  return std::rand() % n_leaves;
}

void ORAMClient::get_blocks(unsigned int leaf_idx) {
  // flush_stash();

  char buf[sizeof(Cmd)];

  Block null_block;
  Cmd cmd = {
    .opcode = GET_BLOCKS,
    .block = null_block,
    .leaf_idx = leaf_idx,
  };
  trace();
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
  for(int level = 0; level < levels; ++level) {
    for(int j = 0; j < bucket_size; ++j) {
      recv(client_socket, buf, sizeof(Cmd), 0);
      Block *b = &((Cmd*)buf)->block;
      Block dec_b = decrypt_block(*b);
      // if(b->addr != 0) stash.push_back(decrypt_block(*b));
      if(dec_b.addr != 0) stash.push_back(dec_b);
    }
  }
}

void ORAMClient::exit() {
  Cmd cmd;
  cmd.opcode = EXIT;
  send(client_socket, (char*)(&cmd), sizeof(Cmd), 0);
  close(client_socket);
}


void ORAMClient::init_tree() {
  unsigned long num_buckets = 0; 
  for(unsigned long i = 0; i < levels; ++i) num_buckets += (1 << i); 

  unsigned long num_blocks = bucket_size * num_buckets;

  Block tmp_block; 
  tmp_block.addr = 0;

  Cmd populate_tree_cmd = {
    .opcode = POPULATE_TREE,
    .block = tmp_block,
    .leaf_idx = 0, 
  };

  send(client_socket, (char*)(&populate_tree_cmd), sizeof(Cmd), 0);

  Cmd send_block = {
    .opcode = BLOCK,
    .block = tmp_block,
    .leaf_idx = 0,
  };

  for (unsigned long i = 0; i < num_blocks; i++) {
    fill_random_data(tmp_block.data, BLOCK_SIZE);
    send_block.block = encrypt_block(tmp_block);
    send(client_socket, (char*)(&send_block), sizeof(Cmd), 0);
  }
}

Block ORAMClient::encrypt_block(Block b) {
  // if(b.addr != 0) std::cout << "encrypt_block " << b.addr << ", " << b.leaf_idx << "\n";

  std::vector<unsigned char> plaintext((char*)&b, (char*)&b + sizeof(Block));
  std::vector<unsigned char> ciphertext;

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_set_padding(ctx, 0);
  
  if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key->data(), iv->data())) {
    EVP_CIPHER_CTX_free(ctx);
    std::cerr << "EVP_EncryptInit_ex(...) failed.\n";
    std::abort();
  }
  
  ciphertext.resize(plaintext.size());
  int len = 0, ciphertext_len = 0;

  if (!EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size())) {
    EVP_CIPHER_CTX_free(ctx);
    std::cerr << "EVP_EncryptUpdate(...) failed.\n";
    std::abort();
  }
  ciphertext_len = len;

  if (!EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
    EVP_CIPHER_CTX_free(ctx);
    std::cerr << "EVP_EncryptFinal_ex(...) failed.\n";
    std::abort();
  }
  ciphertext_len += len;

  // std::cout << ciphertext_len << " " << sizeof(Block) << "\n";
  ciphertext.resize(ciphertext_len);

  EVP_CIPHER_CTX_free(ctx); 

  memcpy(&b, &(*ciphertext.begin()), sizeof(Block));
  return b; 
}

Block ORAMClient::decrypt_block(Block b) {
  std::vector<unsigned char> ciphertext((char*)&b, (char*)&b + sizeof(Block));
  std::vector<unsigned char> plaintext;

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_set_padding(ctx, 0);

  if(!ctx) {
    std::cerr << "EVP_CIPHER_CTX_new() failed.\n";
    std::abort();
  }

  if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key->data(), iv->data())) {
    EVP_CIPHER_CTX_free(ctx);
    std::cerr << "EVP_DecryptInit_ex(...) failed.\n";
    std::abort();
  }

  plaintext.resize(ciphertext.size());
  int len = 0, plaintext_len = 0;
  
  if (!EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size())) {
    EVP_CIPHER_CTX_free(ctx);
    std::cerr << "EVP_DecryptUpdate(...) failed.\n";
    std::abort();
  }
  plaintext_len = len;
  
  if (!EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
    EVP_CIPHER_CTX_free(ctx);
    std::cerr << "EVP_DecryptFinal_ex(...) failed.\n";
    std::abort();
  }
  plaintext_len += len;
  plaintext.resize(plaintext_len);
  
  EVP_CIPHER_CTX_free(ctx);

  memcpy(&b, &(*plaintext.begin()), sizeof(Block));
  // if(b.addr != 0) std::cout << "decrypt_block " << b.addr << ", " << b.leaf_idx << "\n";
  return b; 
}

void ORAMClient::fill_random_data(char *buf, unsigned int num_bytes) {
  std::random_device rd;
  for (unsigned int i = 0; i < num_bytes; ++i) {
    buf[i] = static_cast<uint8_t>(rd() & 0xFF);
  }
}

void ORAMClient::flush_stash() {
  if(stash.size() > stash_threshold) {
    unsigned int leaf_idx = random_leaf_idx();
    get_blocks(leaf_idx);
    dump_stash(leaf_idx);
  }
}

Block* ORAMClient::get_block(BlockPtr b_ptr) {
  return get_block(b_ptr.addr, b_ptr.leaf_idx);
}

Block* ORAMClient::get_block(unsigned int addr, unsigned int leaf_idx) {
  // std::cout << "(" << addr << ", " << leaf_idx << ")\n";
  if(addr == 0) return NULL;

  // check stash
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      it->in_use = true;
      flush_stash();
      return &(*it);
    }
  }

  // check server
  Block *b = NULL;
  get_blocks(leaf_idx);
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == addr) {
      it->in_use = true;
      b = &(*it);
      break;
    }
  }
  dump_stash(leaf_idx);

  if(b == NULL) { 
    std::cerr << "Non-NULL block not found in stash or on server...\n";
    std::abort();
    return NULL;
  }

  return b;
}

void ORAMClient::delete_block(unsigned int addr) {
  delete_block(BlockPtr(addr, 0));
}

void ORAMClient::delete_block(BlockPtr b_ptr) {
  for(auto it = stash.begin(); it != stash.end(); ++it) {
    if(it->addr == b_ptr.addr) {
      stash.erase(it, std::next(it));
      return;
    }
  }

  std::cerr << "delete_block(...) could not find block to delete.\n";
  std::abort();
}

int ORAMClient::stash_size() {
  return stash.size();
}

int ORAMClient::get_bw_usage() {
  int num_buckets = 0; 
  for(unsigned long i = 0; i < levels; ++i) num_buckets += (1 << i); 
  int setup_cost = (bucket_size*levels*num_buckets + 1)*sizeof(Cmd);
  return setup_cost + 2*(bucket_size*levels*num_server_rw+1)*sizeof(Cmd);
}

/* --------------------------------------------- */

ORAMServer::ORAMServer(uint16_t port, unsigned int levels, unsigned int bucket_size) {
  this->levels = levels;
  this->bucket_size = bucket_size;
  root = new Node(levels-1, levels, bucket_size);

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
    std::cout << bytes; 
    if (bytes == 0) {
      return; 
    }
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
      case POPULATE_TREE: 
      populate_tree();
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
    for(int i = 0; i < bucket_size; ++i) {
      recv(client_socket, buf, sizeof(Cmd), 0);
      Cmd cmd = *(Cmd*)buf;

      assert(cmd.opcode == BLOCK);
      curr->bucket->blocks.push_back(cmd.block);
      std::cout << "(" << cmd.block.addr << ", " << cmd.block.leaf_idx << ") ";
    }
    std::cout << "\n";
    assert(curr->bucket->blocks.size() == bucket_size);
    curr = curr->parent;
 }
 std::cout << "-----------------------------------------------\n";
}

void ORAMServer::get_blocks(unsigned int leaf_idx) {
  trace();
  Node *curr = root;
  std::cout << "---------- GET BLOCKS IDX " << leaf_idx << " ----------\n";
  // while(curr != NULL) {
  for(int i = levels-1; i >= 0; --i) { // 2, 1, 0
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
  for(int i = levels-2; i >= 0; --i) {
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


void ORAMServer::populate_tree() {
  clear_tree(root);
  populate_tree(root);
}

void ORAMServer::populate_tree(Node *root) {
  Cmd cmd;

  // read blocks into current nodes bucket
  for(int i = 0; i < bucket_size; ++i) {
    int bytes = recv(client_socket, (char*)&cmd, sizeof(Cmd), 0);
    if(bytes <= 0) {
      std::cerr << "Read <= 0 bytes from client during initialization.\n";
      std::abort();
    }
    assert(cmd.opcode == BLOCK);
    root->bucket->blocks.push_back(cmd.block);
  }

  // read blocks into left subtree
  if(root->l_child != NULL) populate_tree(root->l_child);
  // read blocks into right subtree
  if(root->r_child != NULL) populate_tree(root->r_child);
}

void ORAMServer::clear_tree(Node *root){
  Node *curr = root;
  if (curr == NULL) {
    return;
  }
  curr->bucket->clear();
  clear_tree(curr->l_child); 
  clear_tree(curr->r_child); 
}
