#pragma once
#include <cstddef> // for std::size_t
#include <cassert>
#include <vector>
#include <unordered_map>
#include <limits>
#include "../compile_time_utils/type_traits.hpp" // custom one

// only a temp solution until i create my own dynamic array and the other memory utils

// ENTITY :

using entity = std::size_t;


namespace myecs {

  template<typename T>
  using d_array = std::vector<T>;

  template<typename T, typename Y = T>
  using unordered_map = std::unordered_map<T, Y>;

  template <typename number_key, typename link>
  struct sparse_set {
    
    static_assert(myutils::is_unsigned_number_type<number_key>::value, "Argument <typename key> in struct sparse_set<T>{}; should only be of any unsigned number type");
    static_assert(myutils::is_same_type<link, bool>::value, "Dont use booleans as a linked type, not supported");

    static constexpr number_key INVALID_KEY = std::numeric_limits<number_key>::max();
    static constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();

    myecs::d_array<std::size_t> sparse; // takes key, returns dense index
    myecs::d_array<number_key> reverse_sparse; // takes dense index, returns key
    
    myecs::d_array<link> dense; // actual tightly packed data

    sparse_set() {
      
    }

    void set_link(const number_key& key, const link& data){
  
      dense.emplace_back(data);

      sparse.resize(key + 1, INVALID_INDEX);
      reverse_sparse.resize(dense.size(), INVALID_KEY);

      sparse[key] = dense.size() - 1;
      reverse_sparse[dense.size() - 1] = key;
    }

    void remove(const number_key& key) {
      assert(key < sparse.size() && "Key out of bounds of sparse");
      assert(sparse[key] != INVALID_INDEX && "Data doesnot exist for the provided key");

      std::size_t dense_remove_index = sparse[key];
      //number_key& key_at_dense_remove_index = key;

      std::size_t dense_last_index_bd = dense.size() - 1; //_bd = before deletion
      number_key key_at_dense_last_index_bd = reverse_sparse[dense_last_index_bd]; //_bd = before deletion
      
      if (dense_remove_index != dense_last_index_bd) {
        dense[dense_remove_index] = dense.back(); // swap with last dense index
        dense.pop_back(); // remove the last index
        sparse[key] = INVALID_INDEX;
        reverse_sparse[dense_last_index_bd] = INVALID_KEY; // as the last dense index bd has popped, it points to garbage (invalid key)

        sparse[key_at_dense_last_index_bd] = dense_remove_index;
        reverse_sparse[dense_remove_index] = key_at_dense_last_index_bd;
      }
      else if (dense_remove_index == dense_last_index_bd) {
        dense.pop_back(); // remove the last index
        sparse[key] = INVALID_INDEX;
        reverse_sparse[dense_last_index_bd] = INVALID_KEY;
      }
    
    }
  };

}