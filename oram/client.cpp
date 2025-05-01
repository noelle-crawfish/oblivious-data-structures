
#include <iostream>
#include <thread>
#include <chrono>

#include "new_oram.h"
#include "stack.h"
#include "queue.h"
#include "map.h"

#define N 6

int main() {
  // char data[BLOCK_SIZE] = "Hello, world!\0";
  // char buf[BLOCK_SIZE];

  // ORAMClient client = ORAMClient("127.0.0.1", 8080);
  // client.write(0x1, data);
  // client.read(buf, 0x1);

  // StackClient client = StackClient("127.0.0.1", 8080);
  // QueueClient client = QueueClient("127.0.0.1", 8080);
  // client.push(data);

  // memcpy(data, "Goodbye, world!\0", 17);
  // client.push(data);

  // client.pop(buf);
  // std::cout << "Read: " << buf << "\n";

  // client.pop(buf);
  // std::cout << "Read: " << buf << "\n";

  MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080);
  for(int i = 1; i <= N; ++i) {
  // for(int i = N-1; i >=0; --i) {
    client.insert(i, i*2);
    std::cout << "Inserted: " << i << ", " << i*2 << "\n";
    std::cout << "-----------------------------------------------------------------\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  std::cout << "Does key 3 exist? " << client.contains(3) << "\n";
  client.remove(3);
  std::cout << "Does key 3 exist? " << client.contains(3) << "\n";

  for(int i = 1; i <= N; ++i) {
     if(i != 3) std::cout << client.at(i) << " ";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  std::cout << "\n";

  client.exit();

  return 0;
}
