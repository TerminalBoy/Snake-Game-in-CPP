#include <utility>

#pragma once

// A lighter dynamic array 

namespace myecs {

template <typename T>
class d_array {

  size_t size = 0;
  T* arr = nullptr;

  

public:
  d_array() = default;

  void push_back(T f_arg){
    if (size == 0) {
      arr = new T[1];
      arr[0] = f_arg;
      size++;
    }
    else{
      T* temp = arr;
      arr = new T[size + 1];
      for (size_t i = 0; i < size; i++) arr[i] = temp[i];
      arr[size] = f_arg;
      size++;
      delete[] temp;
    }
  }

  ~d_array() {
    delete[] arr;
  }

  inline T operator[](const size_t& i) {
    return arr[i];
  }

  inline size_t getsize(){ return size; }
};



};