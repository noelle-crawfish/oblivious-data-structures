
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#include "new_oram.h"
#include "stack.h"
#include "queue.h"
#include "set.h"
#include "map.h"

#define N 100

void stack_test() {
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);

  StackClient client = StackClient("127.0.0.1", 8080);

  for(int i = 1; i <= N; ++i) {
    memcpy(data, (char*)&i, sizeof(int));
    client.push(data);
    std::cout << i << " ";
    //std::cout << client.stash_size() << "\n";
  }
  std::cout << "\n\n";

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
    std::cout << *((int*)data) << " ";
    // std::cout << client.stash_size() << "\n";
  }
  std::cout << "\n";

  // std::cout << client.stash_size() << "\n";

  client.exit();
}

void queue_test() {
  char data[BLOCK_SIZE];
  memset(data, 0, BLOCK_SIZE);

  QueueClient client = QueueClient("127.0.0.1", 8080);

  for(int i = 1; i <= N; ++i) {
    memcpy(data, (char*)&i, sizeof(int));
    client.push(data);
    std::cout << i << " ";
  }
  std::cout << "\n\n";

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
    std::cout << *((int*)data) << " ";
  }

  std::cout << "\n";

  client.exit();

}

void set_test() {
  SetClient<int> client = SetClient<int>("127.0.0.1", 8080);
  for(int i = 1; i <= N; ++i) {
    std::cout << i << " ";
    client.insert(i);
  }
  std::cout << "\n";

  for(int i = 1; i <= N; ++i) {
    std::cout << i << " ";
    // std::cout << i << " " << std::string(25-int(i / 10), '-') << "\n";
    client.remove(i);
  }

  std::cout << "\n";
  client.exit();
}

void map_test() {
  // MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080);
  // for(int i = 1; i < N; ++i) {
  // // for(int i = N-1; i >=0; --i) {
  //   client.insert(i, i);
  //   std::cout << "Inserted: " << i << ", " << i << "\n";
  //   // client.prefix_print();
  //   std::cout << "-----------------------------------------------------------------\n";
  //   // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // }
  // exit(0);


  // for(int i = 6; i < 9; ++i) {
  //   if(client.contains(i)) client.remove(i);
  // }

  // std::cout << "Size of tree: " << client.size() << "\n";

  // for(int i = 1; i < N; ++i) {
  //   // if(i < 5 && i >= 10)
  //   if(client.contains(i)) std::cout << "client.at(" << i << ") = " << client.at(i) << " \n";
  //    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  // }
  // std::cout << "\n";

  // std::cout << "Does key 3 exist? " << client.contains(3) << "\n";
  // client.remove(3);
  // std::cout << "Does key 3 exist? " << client.contains(3) << "\n";


  // client.exit();
}

int main(int argc, char* argv[]) {

  if(argc < 2) {
    std::cout << "Usage: ./client [data structure (stack, queue, map, set)]\n";
    return -1;
  }

  char* ds = argv[1];

  if(strcmp(ds, "stack") == 0) stack_test();
  else if(strcmp(ds, "queue") == 0) queue_test();
  else if(strcmp(ds, "map") == 0) map_test();
  else if(strcmp(ds, "set") == 0) set_test();
  else {
    std::cout << "Bad data structure. Choose one of: stack, queue, map, set.\n";
    return -1;
  }

  return 0;
}
