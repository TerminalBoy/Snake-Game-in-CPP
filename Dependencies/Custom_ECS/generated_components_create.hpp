#include "components.hpp"

template <typename T>
void create_component(std::unique_ptr<T>& pointer) {}
 
template <>
void create_component<comp::position>(std::unique_ptr<comp::position>& pointer){
  pointer->x.emplace_back();
  pointer->y.emplace_back();
}
 
template <>
void create_component<comp::rectangle>(std::unique_ptr<comp::rectangle>& pointer){
  pointer->width.emplace_back();
  pointer->height.emplace_back();
  pointer->shape.emplace_back();
}
 
template <>
void create_component<comp::circle>(std::unique_ptr<comp::circle>& pointer){
  pointer->radius.emplace_back();
  pointer->shape.emplace_back();
}
 
template <>
void create_component<comp::color>(std::unique_ptr<comp::color>& pointer){
  pointer->value.emplace_back();
}
 
template <>
void create_component<comp::segment>(std::unique_ptr<comp::segment>& pointer){
  pointer->obj.emplace_back();
}
