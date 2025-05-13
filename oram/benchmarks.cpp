
#include <set>

#include "stack.h"
#include "queue.h"
#include "set.h"
#include "map.h"

#define N 100

void stack_benchmark(unsigned int levels, unsigned int bucket_size, unsigned int threshold) {
  std::cout << "Testing L = " << levels << ", Z = " << bucket_size << ", THRES = " << threshold << "\n";
  int max_stash_size = 0;
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);

  StackClient client = StackClient("127.0.0.1", 8080, levels, bucket_size, threshold);

  for(int i = 1; i <= N; ++i) {
    memcpy(data, (char*)&i, sizeof(int));
    client.push(data);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }

  std::cout << "max_stash_size = " << max_stash_size << "\n";
  std::cout << "bw_usage = " << client.get_bw_usage() << "\n";

  client.exit();
}

void queue_benchmark(unsigned int levels, unsigned int bucket_size, unsigned int threshold) {
  std::cout << "Testing L = " << levels << ", Z = " << bucket_size << ", THRES = " << threshold << "\n";
  int max_stash_size = 0;
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);

  QueueClient client = QueueClient("127.0.0.1", 8080, levels, bucket_size, threshold);

  for(int i = 1; i <= N; ++i) {
    memcpy(data, (char*)&i, sizeof(int));
    client.push(data);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }

  std::cout << "max_stash_size = " << max_stash_size << "\n";
  std::cout << "bw_usage = " << client.get_bw_usage() << "\n";

  client.exit();
}

void avl_benchmark(unsigned int levels, unsigned int bucket_size, unsigned int threshold) {
  std::cout << "Testing L = " << levels << ", Z = " << bucket_size << ", THRES = " << threshold << "\n";
  int max_stash_size = 0;
  MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080, levels, bucket_size, threshold);
  
  for(int i = 1; i <= N; ++i) {
    client.insert(i, 5*i);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }
  
  client.contains(2);
  max_stash_size = std::max(max_stash_size, client.stash_size());
  client.remove(2);
  max_stash_size = std::max(max_stash_size, client.stash_size());
  client.contains(2);
  max_stash_size = std::max(max_stash_size, client.stash_size());
  
  for(int i = 1; i <= N; ++i) {
    if(i != 2) client.at(i);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }
  
  for(int i = 1; i <= N; ++i) {
    if(i != 2) client.remove(i);
    max_stash_size = std::max(max_stash_size, client.stash_size());
  }
  
  std::cout << "max_stash_size = " << max_stash_size << "\n";
  std::cout << "bw_usage = " << client.get_bw_usage() << "\n";

  client.exit();
}

int main(int argc, char* argv[]) {
  if(argc < 5) {
    std::cout << "Usage: ./client [data structure] [levels] [bucket_size] [threshold]\n";
    return -1;
  }

  char* ds = argv[1];

  unsigned int levels = std::stoi(argv[2]);
  unsigned int bucket_size = std::stoi(argv[3]);
  unsigned int threshold = std::stoi(argv[4]);

  if(strcmp(ds, "stack") == 0) stack_benchmark(levels, bucket_size, threshold);
  else if(strcmp(ds, "queue") == 0) queue_benchmark(levels, bucket_size, threshold);
  else if(strcmp(ds, "avl") == 0) avl_benchmark(levels, bucket_size, threshold);
  else {
    std::cout << "Bad data structure. Choose one of: stack, queue, map, set.\n";
    return -1;
  }

  return 0;
}
