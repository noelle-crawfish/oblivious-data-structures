
#include <iostream>
#include <chrono>
#include <thread>

#include "new_oram.h"

int main(int argc, char* argv[]) {
  if(argc < 3) {
    std::cout << "Usage: ./server [levels] [bucket_size]\n";
    return -1;
  }

  unsigned int levels = std::stoi(argv[1]);
  unsigned int bucket_size = std::stoi(argv[2]);

  // stash_threshold doesn't matter to server, it's just a repetition
  ORAMServer server = ORAMServer(8080, levels, bucket_size); 
  server.run();

  return 0;
}
