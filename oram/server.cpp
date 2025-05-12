
#include <iostream>
#include <chrono>
#include <thread>

#include "benchmarks.h"
#include "new_oram.h"

int main() {
  int port = 8080; 
  for(auto level_it = levels.begin(); level_it != levels.end(); ++level_it) {
    for(auto bucket_size_it = bucket_sizes.begin(); bucket_size_it != bucket_sizes.end(); ++bucket_size_it) {
      for(auto thresh_it = stash_thresholds.begin(); thresh_it != stash_thresholds.end(); ++thresh_it) {
	// stash_threshold doesn't matter to server, it's just a repetition
	ORAMServer server = ORAMServer(port++, *level_it, *bucket_size_it); 
	server.run();
	std::this_thread::sleep_for(std::chrono::seconds(1)); // client likes to connect first idk why
      }
    }
  }
  return 0;
}
