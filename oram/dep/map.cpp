#include "map.h"

// template class MapClient<int, int>;

template<typename K, typename V>
MapClient<K, V>::MapClient(std::string server_addr, int port) :
  SetClient<std::pair<K, V>>::SetClient(server_addr, port) {
  // nop
}

template<typename K, typename V>
void MapClient<K, V>::insert(K k, V v) {
  std::pair<K, V> data = std::pair(k, v);

  SetClient<std::pair<K, V>>::insert(data);
}

template<typename K, typename V>
void MapClient<K, V>::remove(K k) {
  V null_v;
  std::pair<K, V> data = std::pair(k, null_v);

  SetClient<std::pair<K, V>>::remove(data);
}

template<typename K, typename V>
bool MapClient<K, V>::contains(K k) {
  V null_v;
  std::pair<K, V> data = std::pair(k, null_v);

  return SetClient<std::pair<K, V>>::contains(data);
}

template<typename K, typename V>
int MapClient<K, V>::compare_value(std::pair<K, V> kv1, std::pair<K, V> kv2) {
  if(kv1.first < kv2.first) return -1;
  else if(kv1.first > kv2.first) return 1;
  else return 0;
}
