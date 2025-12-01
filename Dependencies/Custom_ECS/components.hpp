#pragma once

#include "memory.hpp"
#include "../SFML/include/SFML/Graphics.hpp"

// ##-START_PARSE-##

namespace comp {
  struct position {
    myecs::d_array<float> x;
    myecs::d_array<float> y;
  };

  struct rectangle {
    myecs::d_array<float> width;
    myecs::d_array<float> height;
    myecs::d_array<sf::RectangleShape> shape;
  };

  struct circle {
    myecs::d_array<float> radius;
    myecs::d_array<sf::CircleShape> shape;
  };

  struct color {
    myecs::d_array<std::uint8_t> r;
    myecs::d_array<std::uint8_t> g;
    myecs::d_array<std::uint8_t> b;
  };

  struct segment {
    myecs::d_array<entity> obj;
  };

  struct physics {
    myecs::d_array<float> speed;
  };

}

// ##-STOP_PARSE-##