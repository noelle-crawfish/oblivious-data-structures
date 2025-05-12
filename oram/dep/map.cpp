#include "map.h"

template class MapClient<int, int>;

template<typename K, typename V>
MapClient<K, V>::MapClient(std::string server_addr, int port, unsigned int levels,
			   unsigned int bucket_size, unsigned int threshold) :
			     SetClient<std::pair<K, V>>::SetClient(server_addr, port, levels,
								   bucket_size, threshold) {
  // nop
}

template<typename K, typename V>
void MapClient<K, V>::insert(K k, V v) {
  std::pair<K, V> data = std::pair(k, v);

  SetClient<std::pair<K, V>>::insert(data);
  for(auto it = this->stash.begin(); it != this->stash.end(); ++it) (*it).in_use = false;
}

template<typename K, typename V>
void MapClient<K, V>::remove(K k) {
  V null_v;
  std::pair<K, V> data = std::pair(k, null_v);

  SetClient<std::pair<K, V>>::remove(data);
  for(auto it = this->stash.begin(); it != this->stash.end(); ++it) (*it).in_use = false;
}

template<typename K, typename V>
bool MapClient<K, V>::contains(K k) {
  V null_v;
  std::pair<K, V> data = std::pair(k, null_v);

  bool contains = SetClient<std::pair<K, V>>::contains(data);
  for(auto it = this->stash.begin(); it != this->stash.end(); ++it) (*it).in_use = false;
  return contains;
}

template<typename K, typename V>
V MapClient<K, V>::at(K k) {
  V null_v;
  std::pair<K, V> data = std::pair(k, null_v);
  BlockPtr b_ptr = SetClient<std::pair<K, V>>::find(data, BlockPtr(this->root_addr, this->root_leaf));

  if(b_ptr.addr == 0) {
    std::cerr << "find() could not find key\n";
    std::abort();
  }

  Block *b = ORAMClient::get_block(b_ptr);
  V v = ((std::pair<K, V>*)(b->data))->second;
  for(auto it = this->stash.begin(); it != this->stash.end(); ++it) (*it).in_use = false;
  return v;
}

template<typename K, typename V>
int MapClient<K, V>::compare_value(std::pair<K, V> kv1, std::pair<K, V> kv2) {
  if(kv1.first < kv2.first) return -1;
  else if(kv1.first > kv2.first) return 1;
  else return 0;
}
