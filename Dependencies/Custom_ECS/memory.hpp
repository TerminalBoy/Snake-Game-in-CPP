#pragma once

#include <vector>
#include <unordered_map>


// only a temp solution until i create my own dynamic array and the other memory utils

// ENTITY :

using entity = std::size_t;


namespace myecs {

  template<typename T>
  using d_array = std::vector<T>;

  template<typename T, typename Y = T>
  using unordered_map = std::unordered_map<T, Y>;



}