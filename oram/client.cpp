
#include <iostream>

#include "new_oram.h"

int main() {
  ORAMClient client = ORAMClient("127.0.0.1", 8080);

  char data[BLOCK_SIZE] = "Hello, world!\0";
  client.write(0x1, data);

  char buf[BLOCK_SIZE];
  client.read(buf, 0x1);

  std::cout << "Read:" << buf << "\n";

  client.exit();

  return 0;
}
