#include <cstring>
#include <iostream>

#include "oram.h"
#include "stack.h"
#include "queue.h"

int main() {
  // ORAM oram = ORAM(); // TODO maybe I should make this templated for n leaves, block size, etc.

  char data[BLOCK_SIZE];
  memcpy(data, "hello, world!\0", 15);
  
  // oram.write(0xcafebabe, data, NULL);
  // Block b = oram.read(0xcafebabe);

  // std::cout << b.data << "\n";

  // stack
  // ObliviousStack stack = ObliviousStack();
  // stack.push(data);
  // Block b = stack.pop();

  // std::cout << b.data << "\n";
  
  // queue
  // ObliviousQueue queue = ObliviousQueue();
  // queue.push(data);
  // Block b = queue.pop();
  // std::cout << b.data << "\n";

  // map

  // unordered set

  return 0;
}
