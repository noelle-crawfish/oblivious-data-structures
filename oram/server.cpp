
#include <iostream>

#include "new_oram.h"

int main() {
  ORAMServer server = ORAMServer(8080);

  server.run();

  return 0;
}
