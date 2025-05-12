
#include <set>

#include "benchmarks.h"

#include "stack.h"
#include "queue.h"
#include "set.h"
#include "map.h"

void stack_benchmark() {
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);

  StackClient client = StackClient("127.0.0.1", 8080);

  for(int i = 1; i <= N; ++i) {
    memcpy(data, (char*)&i, sizeof(int));
    client.push(data);
  }

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
  }

  client.exit();
}

void queue_benchmark() {
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);

  QueueClient client = QueueClient("127.0.0.1", 8080);

  for(int i = 1; i <= N; ++i) {
    memcpy(data, (char*)&i, sizeof(int));
    client.push(data);
  }
  std::cout << "\n\n";

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
  }

  client.exit();
}

void avl_benchmark() {
  int port = 8080; 
  for(auto level_it = levels.begin(); level_it != levels.end(); ++level_it) {
    for(auto bucket_size_it = bucket_sizes.begin(); bucket_size_it != bucket_sizes.end(); ++bucket_size_it) {
      for(auto thresh_it = stash_thresholds.begin(); thresh_it != stash_thresholds.end(); ++thresh_it) {
	std::cout << "Testing L = " << *level_it << ", Z = " << *bucket_size_it << ", THRES = " << *thresh_it
	<< " " << std::string(15, '-') << "\n";
	int max_stash_size = 0;
	MapClient<int, int> client = MapClient<int, int>("127.0.0.1", port++,
							 *level_it, *bucket_size_it, *thresh_it);

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
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    std::cout << "Usage: ./client [data structure (stack, queue, map, set)]\n";
    return -1;
  }

  char* ds = argv[1];

  if(strcmp(ds, "stack") == 0) stack_benchmark();
  else if(strcmp(ds, "queue") == 0) queue_benchmark();
  else if(strcmp(ds, "avl") == 0) avl_benchmark();
  else {
    std::cout << "Bad data structure. Choose one of: stack, queue, map, set.\n";
    return -1;
  }

  return 0;
}
