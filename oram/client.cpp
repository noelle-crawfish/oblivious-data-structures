
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <unordered_map>
#include <queue>
#include "new_oram.h"
#include "stack.h"
#include "queue.h"
#include "set.h"
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
    //std::cout << client.stash_size() << "\n";
  }
  std::cout << "\n\n";

  for(int i = 1; i <= N; ++i) {
    client.pop(data);
    std::cout << *((int*)data) << " ";
    // std::cout << client.stash_size() << "\n";
  }
  std::cout << "\n";

  // std::cout << client.stash_size() << "\n";

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

void set_test() {
  SetClient<int> client = SetClient<int>("127.0.0.1", 8080);
  for(int i = 1; i <= N; ++i) {
    std::cout << i << " ";
    client.insert(i);
  }
  std::cout << "\n";

  for(int i = 1; i <= N; ++i) {
    std::cout << i << " ";
    // std::cout << i << " " << std::string(25-int(i / 10), '-') << "\n";
    client.remove(i);
  }

  std::cout << "\n";
  client.exit();
}

void map_test() {
  MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080);

  for(int i = 1; i <= N; ++i) {
    std::cout << i << " ";
    client.insert(i, 5*i);
  }
  std::cout << "\n";

  std::cout << "Client contains key 2? " << client.contains(2) << "\n";
  client.remove(2);
  std::cout << "Client contains key 2? " << client.contains(2) << "\n";

  for(int i = 1; i <= N; ++i) {
    if(i != 2) std::cout << i << ": " << client.at(i) << "\n";
  }

  for(int i = 1; i <= N; ++i) {
    std::cout << i << " " << std::string(25-int(i / 10), '-') << "\n";
    // std::cout << i << "\n";
    if(i != 2) client.remove(i);
  }

  client.exit();
}

// uniform distribution across operatiosn 
void map_rand_test(int total_ops, int biggest_key) {

  std::unordered_map<int, int> store = std::unordered_map<int, int>(); 
  MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080);

  int choice;
  std::queue<int> q; 

  for (int i = 0; i < biggest_key; ++i){
    q.push(i); 
  }
  int arr[4] = {0,0,0,0};
  for (int i = 0; i < total_ops; ++i) {
    choice = std::rand();
    int tmp = rand() % biggest_key; 
    if (choice % 4 == 0 && !q.empty()) { // insert
      int idx = q.front();
      std::cout<<"insert"<< idx <<","<< tmp<< "\n" ; 
      
      store[idx] = tmp; 
      client.insert(idx, tmp);
      q.pop();
      arr[0]++; 
      
    } else if (choice % 4 == 1 && store.find(tmp) != store.end()) { // at 
      std::cout<<"at"<< tmp <<"\n"; 
      if (store[tmp] != client.at(tmp)) {
        std::cout <<"values arent correct \n";
      }
      
      arr[1]++; 
    } else if (store.find(tmp) != store.end() && choice % 4 == 2) { // remove
      assert(store.find(tmp) != store.end());
      std::cout<<"remove"<< tmp <<"\n"; 
      store.erase(tmp);
      client.remove(tmp);
      q.push(tmp); 
      arr[2]++; 
      
       
    } else if (choice % 4 == 3){ // contains
      std::cout<<"contains"<< tmp <<"\n"; 
      if (store.find(tmp) == store.end()) {
        if (client.contains(tmp)) {
          std::cout <<"contains found nonexisting\n"; 
        }
      } else if (!client.contains(tmp)){
        std::cout << "contains couldnt find existing\n";
      }
      arr[3]++; 
    }
  } 
  std::cout << "graceful exit\n "; 
  std::cout << " inserts: " <<arr[0]; 
  std::cout << " ats: " <<arr[1]; 
  std::cout << " contains: " <<arr[2]; 
  std::cout << " removes: " <<arr[3]; 
  client.exit();
}
void simple_map() {
  MapClient<int, int> client = MapClient<int, int>("127.0.0.1", 8080);
  client.contains(2);
  client.contains(1);
  client.insert(1,4); 
  client.at(1);
  client.exit();
}
int main(int argc, char* argv[]) {
  if(argc < 2) {
    std::cout << "Usage: ./client [data structure (stack, queue, map, set)]\n";
    return -1;
  }

  char* ds = argv[1];

  if(strcmp(ds, "stack") == 0) stack_test();
  else if(strcmp(ds, "queue") == 0) queue_test();
  else if(strcmp(ds, "map") == 0) map_test();
  else if(strcmp(ds, "set") == 0) set_test();
  else if(strcmp(ds, "map_rand") == 0) map_rand_test(50000, 10);
  else if(strcmp(ds, "simple_map") == 0) simple_map();
  else {
    std::cout << "Bad data structure. Choose one of: stack, queue, map, set.\n";
    return -1;
  }

  return 0;
}
