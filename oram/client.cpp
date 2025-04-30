
#include <iostream>

#include "new_oram.h"
#include "stack.h"
#include "queue.h"
#include "map.h"

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
  for(int i = 0; i < 8; ++i) {
    client.insert(i, i*2);
    std::cout << "Inserted: " << i << ", " << i*2 << "\n";
  }

  // for(int i = 0; i < 8; ++i) {
  //   std::cout << client.at(i) << " ";
  // }
  // std::cout << "\n";

  client.exit();

  return 0;
}
