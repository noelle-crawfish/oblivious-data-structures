#include "oram.h"

#include <cstring>
#include <iostream>

int main() {
  ORAM oram = ORAM(); // TODO maybe I should make this templated for n leaves, block size, etc.

  char data[BLOCK_SIZE];
  memcpy(data, "hello, world!\0", 15);
  
  oram.write(0xcafebabe, data);
  Block b = oram.read(0xcafebabe);

  std::cout << b.data << "\n";

  return 0;
}
