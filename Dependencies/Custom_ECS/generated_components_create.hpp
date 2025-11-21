 
#include "components.hpp"

template <>
void create_component<comp::position>(){
  comp::position::x.emplace_back();
  comp::position::y.emplace_back();
}
 
template <>
void create_component<comp::rectangle>(){
  comp::rectangle::width.emplace_back();
  comp::rectangle::height.emplace_back();
  comp::rectangle::shape.emplace_back();
}
 
template <>
void create_component<comp::circle>(){
  comp::circle::radius.emplace_back();
  comp::circle::shape.emplace_back();
}
 
template <>
void create_component<comp::color>(){
  comp::color::value.emplace_back();
}
 
template <>
void create_component<comp::segment>(){
  comp::segment::obj.emplace_back();
}
