
#include <iostream>
#include <chrono>
#include <thread>

#include "new_oram.h"

int main() {
  unsigned int bucket_size = 8;
  for(unsigned int levels = 2; levels < 4; ++levels) {
    std::cout << "Re-connecting\n";
    ORAMServer server = ORAMServer(8080, levels, bucket_size);

    server.run();

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  return 0;
}
