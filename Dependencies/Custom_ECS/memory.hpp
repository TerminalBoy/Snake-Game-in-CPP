#pragma once
#include <cstddef> // for std::size_t
#include <cassert>
#include <vector>
#include <unordered_map>
#include <limits>
#include <cstdint>
#include <type_traits> // the standard one
#include "../compile_time_utils/type_traits.hpp" // custom one

// only a temp solution until i create my own dynamic array and the other memory utils

// ENTITY :

using entity = std::size_t;

namespace numeric_type{
  using i32 = std::int32_t;
  using i16 = std::int16_t;
  using i8 = std::int8_t;
}

namespace create_strong_scalar {
  template <typename target_type, typename type_tag>
  struct type {
    static_assert(std::is_scalar<target_type>::value, "Please use strong_nonscalar::type<T1, T2> for non scalar types");

  private:
    target_type value;

  public:

    constexpr type() noexcept = default;
    constexpr type(const type&) = default;
    constexpr type(type&&) = default;

    explicit constexpr type(target_type data) noexcept
      : value(data) {}

    constexpr target_type get() const noexcept {
      return value;
    }

    constexpr void set(target_type data) noexcept{
      value = data;
    }

    type& operator = (const type& data) = default;
    type& operator = (type&& data) = default;

  };

  template <typename target_type, typename type_tag>
  constexpr bool operator > (const type<target_type, type_tag>& left, const type<target_type, type_tag>& right) noexcept {
    return left.get() > right.get();
  }

  template <typename target_type, typename type_tag>
  constexpr bool operator < (const type<target_type, type_tag>& left, const type<target_type, type_tag>& right) noexcept {
    return left.get() < right.get();
  }

  template <typename target_type, typename type_tag>
  constexpr bool operator == (const type<target_type, type_tag>& left, const type<target_type, type_tag>& right) noexcept {
    return left.get() == right.get();
  }
  
  template <typename target_type, typename type_tag>
  constexpr bool operator != (const type<target_type, type_tag>& left, const type<target_type, type_tag>& right) noexcept {
    return left.get() != right.get();
  }

}


// strong immutable types for critical gameplay components, (prevents hard to debug implicit mutations and bugs)
namespace component {
  
  namespace type_tag { // type tags for strong type creation
    struct PosPix_x;
    struct PosPix_y;

    struct PosGrid_x;
    struct PosGrid_y;

    struct WidthPix;
    struct HeightPix;
    
    struct WidthGrid;
    struct HeightGrid;

    struct RadiusPix;

    struct Speed;
    struct Direction;
  }
  
  namespace type {

    using PosPix_x = create_strong_scalar::type<std::int32_t, type_tag::PosPix_x>;
    using PosPix_y = create_strong_scalar::type<std::int32_t, type_tag::PosPix_y>;

    using PosGrid_x = create_strong_scalar::type<std::int32_t, type_tag::PosGrid_x>;
    using PosGrid_y = create_strong_scalar::type<std::int32_t, type_tag::PosGrid_y>;

    using WidthPix = create_strong_scalar::type<std::int32_t, type_tag::WidthPix>;
    using HeightPix = create_strong_scalar::type<std::int32_t, type_tag::HeightPix>;

    using WidthGrid = create_strong_scalar::type<std::int32_t, type_tag::WidthGrid>;
    using HeightGrid = create_strong_scalar::type<std::int32_t, type_tag::HeightGrid>;

    using RadiusPix = create_strong_scalar::type<std::int32_t, type_tag::RadiusPix>;

    using Speed = create_strong_scalar::type<float, type_tag::Speed>;
    using Direction = create_strong_scalar::type<float, type_tag::Direction>;

  }
}

namespace myecs {

  template<typename T>
  using d_array = std::vector<T>;

  template<typename T, typename Y = T>
  using unordered_map = std::unordered_map<T, Y>;

  template <typename number_key, typename link>
  struct sparse_set {

    static_assert(myutils::is_unsigned_number_type<number_key>::value, "Argument <typename key> in struct sparse_set<T>{}; should only be of any unsigned number type");
    static_assert(!myutils::is_same_type<link, bool>::value, "Dont use booleans as a linked type, not supported");

    static constexpr number_key INVALID_KEY = std::numeric_limits<number_key>::max();
    static constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();

    myecs::d_array<std::size_t> index_at_key; // takes key, returns dense index
    myecs::d_array<number_key> key_at_index; // takes dense index, returns key

    myecs::d_array<link> dense; // actual tightly packed data

    sparse_set() {

    }

    void resize(const std::size_t& target) {
     
    }

    void set_link(const number_key& key, const link& data) {
      assert((key >= sparse.size() || sparse[key] == INVALID_INDEX) && "Key is already linked somewhere");
      assert(key_at_index.size() == dense.size() && "key_at_index[] is desynced with dense[]");

      if (key >= index_at_key.size())
        index_at_key.resize(key + 1, INVALID_INDEX);

      key_at_index.push_back(INVALID_KEY);

      index_at_key[key] = dense.size();
      key_at_index[dense.size()] = key;

      dense.emplace_back(data);
    }

    void remove(const number_key& key) {
      assert(key < index_at_key.size() && "Key out of bounds of sparse");
      assert(index_at_key[key] != INVALID_INDEX && "Key is deleted or is never initialized");

      std::size_t replace_index = index_at_key[key];

      std::size_t last_index = dense.size() - 1; // before deletion
      
      number_key key_last_index = key_at_index[last_index]; // before deletion

      if (replace_index != last_index) {
        
        index_at_key[key] = INVALID_INDEX;
        key_at_index.pop_back(); // as the last dense index has to be popped, it points to garbage (invalid key)

        index_at_key[key_last_index] = replace_index; // last index data will be moved to the replace index so sparse[key_last_index] points to the replace_index
        key_at_index[replace_index] = key_last_index; // last index data will be moved to the replace index so key_at_index[replace_index] points to the key_last_index
        
        dense[replace_index] = dense.back(); // swap with last dense index
        
        dense.pop_back(); // remove the last index
      
      }
      else if (replace_index == last_index) {
        index_at_key[key] = INVALID_INDEX;

        key_at_index.pop_back();
        dense.pop_back(); // remove the last index
      }

    }

  };

}