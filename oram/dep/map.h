#ifndef MAP_H
#define MAP_H

#include "set.h"

template <typename K, typename V>
class MapClient : public SetClient<std::pair<K, V>> {
public:
  MapClient(std::string server_addr, int port, unsigned int levels, unsigned int bucket_size,
	    unsigned int threshold);
  MapClient(std::string server_addr, int port) :
    MapClient(server_addr, port, L, BUCKET_SIZE, STASH_THRESHOLD) {};
  void insert(K k, V v);
  void remove(K k);
  bool contains(K k);
  V at(K k);
private:
  BlockPtr find(K k, BlockPtr b_ptr);
  int compare_value(std::pair<K, V> kv1, std::pair<K, V> kv2);
};


#endif
