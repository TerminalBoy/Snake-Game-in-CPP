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
    myecs::d_array<sf::Color> value;
  };

  struct segment {
    myecs::d_array<entity> obj;
  };

}

// ##-STOP_PARSE-##