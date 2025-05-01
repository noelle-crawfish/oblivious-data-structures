
#include <iostream>
#include <thread>
#include <chrono>

#include "new_oram.h"
#include "stack.h"
#include "queue.h"
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
  }
  std::cout << "\n\n";

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
    std::cout << *((int*)data) << " ";
  }
  std::cout << "\n";

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

void map_test() {

  MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080);
  for(int i = 1; i < N; ++i) {
  // for(int i = N-1; i >=0; --i) {
    client.insert(i, i*2);
    std::cout << "Inserted: " << i << ", " << i*2 << "\n";
    std::cout << "-----------------------------------------------------------------\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }


  for(int i = 6; i < 9; ++i) {
    if(client.contains(i)) client.remove(i);
  }

  std::cout << "Size of tree: " << client.size() << "\n";

  for(int i = 1; i < N; ++i) {
    // if(i < 5 && i >= 10)
    if(client.contains(i)) std::cout << "client.at(" << i << ") = " << client.at(i) << " \n";
     // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  std::cout << "\n";

  std::cout << "Does key 3 exist? " << client.contains(3) << "\n";
  client.remove(3);
  std::cout << "Does key 3 exist? " << client.contains(3) << "\n";

  client.exit();
}

int main() {

  // stack_test();
  // queue_test();
  map_test();

  return 0;
}
