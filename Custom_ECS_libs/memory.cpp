#include <utility>

#pragma once;

// A lighter dynamic array 

namespace myecs {

template <typename T>
class d_array {

  size_t size = 0;
  T* arr = nullptr;

  d_array (){

  }

  void push_back(T f_arg){
    if (size == 0) {
      size ++;
      arr = new T[size];
    }
    else{
      size ++;
      T* temp = arr;
      arr = new T[size];
    }
  }


};



};