

// About including
// you can include as the directory starts from the project root
// compiler does not matter

// Even in Visual Studio, the (.sln) file is configured to have the project root as one of its include directory


// About linking
// the linker root is also configired to have the project root as its starting or its root directory (Visual Studio)

// Conclusion : 
// Whether your compiler is the Visual Studio one (MSVC) or your are using Visual Studio IDE and its configuration (.sln)
// or wheather you are using any compiler or any IDE
// Note that all directories related to libraries(including and linking) are in perspective of project root or repository root

// I have tried my best too keep the project as compiler and IDE independent as i can

// the include and link are as transperent as they are in perspective of project/repository root;
#include <cstddef> // for std::size_t
#include <cstdint>
#include <memory>
#include <iostream> // only  for debugging
#include <cstdlib>  
#include <vector>
#include <string>
//#include <array> - not in use
#include <unordered_map> //- not in use
#include <limits>
#include <cassert>

#include <algorithm>
#include <execution>
#include <chrono> // for getting PRNG seed
#include <SFML/Graphics.hpp>

#include "Dependencies/Custom_ECS/memory.hpp"
#include "Dependencies/Custom_ECS/ECS_CORE.hpp" // the custom core ecs
#include "Dependencies/Custom_ECS/components.hpp"
#include "Dependencies/Custom_ECS/generated_components_create.hpp"
#include "Dependencies/Custom_ECS/generated_components_delete.hpp"

#include "Dependencies/MyGameLibs/include/random.hpp" // for xorshift32 PRNG


namespace mygame {
  
  namespace hot { // declaring in advance to avoid branch prediction in hot loops
    std::int32_t grid_x{};
    std::int32_t grid_y{};
    std::int32_t key{};
    std::size_t product{};
    std::int32_t ran_x{};
    std::int32_t ran_y{};
    std::int32_t pos{};
    std::int32_t diff{};
  }

  std::size_t ui = 0; // universal declarations for hot calls, avoids branch prediction
  std::size_t usize = 0; 

  constexpr component::type::WidthPix cell_width{ 20 };
  constexpr component::type::HeightPix cell_height{ 20 };

  constexpr component::type::Direction direction_left{ 0 };
  constexpr component::type::Direction direction_right{ 1 };
  constexpr component::type::Direction direction_up{ 2 };
  constexpr component::type::Direction direction_down{ 3 };

  bool move = false;
  bool food_eaten = true;

  namespace helper {
    
    // basic conversions encapsulated to avoid clutter

    inline std::int32_t Grid_x_TO_Pix_x(const component::type::PosGrid_x pos_x_grid) {
      return pos_x_grid.get() * cell_width.get();
    }

    inline std::int32_t Grid_y_TO_Pix_y(const component::type::PosGrid_y pos_y_grid) {
      return pos_y_grid.get() * cell_height.get();
    }



    inline std::int32_t Pix_x_TO_Grid_x(const component::type::PosPix_x pos_x_pix) {
      return pos_x_pix.get() / cell_width.get();
    }

    inline std::int32_t Pix_y_TO_Grid_y(const component::type::PosPix_y pos_y_pix) {
      return pos_y_pix.get() / cell_height.get();
    }

    inline std::int32_t Pix_Pos_TO_GridIndex(const component::type::PosPix_x pix_x, const component::type::PosPix_y pix_y, const component::type::WidthGrid width_grid) {
      // (x,y) to index = (width * y) + x
      auto grid_x = Pix_x_TO_Grid_x(pix_x);
      auto grid_y = Pix_y_TO_Grid_y(pix_y);
      return (width_grid.get() * grid_y) + grid_x;
    }

  }


  //template <typename T, typename... args>
  struct renderables{
    inline static sf::VertexArray snake;
    inline static sf::VertexArray snake_food;
  };

  static
    void set_snake_direction(const entity& snake_head, const component::type::Direction& snake_direction) {
    ecs_access(comp::physics, snake_head, direction) = snake_direction;
  }

  static
  void update_snake_food_vertices(entity snake_food) {
 
    renderables::snake_food.setPrimitiveType(sf::Quads);
    renderables::snake_food.resize(4);

    const std::uint32_t width = ecs_access(comp::rectangle, snake_food, width).get();
    const std::uint32_t height = ecs_access(comp::rectangle, snake_food, height).get();

    const std::int32_t x = ecs_access(comp::position, snake_food, x).get();
    const std::int32_t y = ecs_access(comp::position, snake_food, y).get();

    const auto r = ecs_access(comp::color, snake_food, r);
    const auto g = ecs_access(comp::color, snake_food, g);
    const auto b = ecs_access(comp::color, snake_food, b);

    renderables::snake_food[0].position.x = x;          // top left
    renderables::snake_food[0].position.y = y;          // top left

    renderables::snake_food[1].position.x = x + width;  // top right
    renderables::snake_food[1].position.y = y;          // top right

    renderables::snake_food[2].position.x = x + width;  // bottom right
    renderables::snake_food[2].position.y = y + height; // bottom right

    renderables::snake_food[3].position.x = x;          // bottom left
    renderables::snake_food[3].position.y = y + height; // bottom left

    renderables::snake_food[0].color = sf::Color(r, g, b);
    renderables::snake_food[1].color = sf::Color(r, g, b);
    renderables::snake_food[2].color = sf::Color(r, g, b);
    renderables::snake_food[3].color = sf::Color(r, g, b);
   
  }

  static
  void update_snake_vertices(const std::vector<entity>& entities) { 
    //std::cout << "entitiy/ snake size : " << entities.size() << std::endl;
    std::size_t entities_size = ecs_access(comp::magnitude, entities[0], size).get();
    
    renderables::snake.setPrimitiveType(sf::Quads);
    renderables::snake.resize(entities_size * 4);
    
    std::size_t base = 0;
    std::size_t i = 0;

    const std::int32_t width = ecs_access(comp::rectangle, entities[0], width).get();
    const std::int32_t height = ecs_access(comp::rectangle, entities[0], height).get();;

   
    for (i = 0; i < entities_size; i++) {
    
      base = i * 4;

      const std::int32_t x = ecs_access(comp::position, entities[i], x).get();
      const std::int32_t y = ecs_access(comp::position, entities[i], y).get();
      
      const auto r = ecs_access(comp::color, entities[i], r);
      const auto g = ecs_access(comp::color, entities[i], g);
      const auto b = ecs_access(comp::color, entities[i], b);

      renderables::snake[base + 0].position.x = x;          // top left
      renderables::snake[base + 0].position.y = y;          // top left

      renderables::snake[base + 1].position.x = x + width;  // top right
      renderables::snake[base + 1].position.y = y;          // top right

      renderables::snake[base + 2].position.x = x + width;  // bottom right
      renderables::snake[base + 2].position.y = y + height; // bottom right

      renderables::snake[base + 3].position.x = x;          // bottom left
      renderables::snake[base + 3].position.y = y + height; // bottom left

      // 
      renderables::snake[base + 0].color = sf::Color(r, g, b);
      renderables::snake[base + 1].color = sf::Color(r, g, b);
      renderables::snake[base + 2].color = sf::Color(r, g, b);
      renderables::snake[base + 3].color = sf::Color(r, g, b);
    }
    move = true;
  }

  static
  void make_snake(std::vector<entity>& snake, std::vector<entity>& followup_buffer, const std::int32_t& size) {
    assert(size != 0 && "Poor snake, you didnt give it a size and also took his head | size cannot be 0");
    assert(snake.size() == 0 && "Please dont destroy another snake to create your own | snake array should be empty in order to create entities");
    assert(followup_buffer.size() == 0 && "Followup buffer SHOULD be empty");

    followup_buffer.emplace_back(myecs::create_entity()); // head
    snake.emplace_back(myecs::create_entity()); // head

    myecs::add_comp_to<comp::position>(followup_buffer.back());
    myecs::add_comp_to<comp::position>(snake.back());
    myecs::add_comp_to<comp::color>(snake.back());

    myecs::add_comp_to<comp::rectangle>(snake.back()); // only the head will have rectangle info
    myecs::add_comp_to<comp::physics>(snake.back()); // only the head will have physics info
    myecs::add_comp_to<comp::magnitude>(snake.back()); // only the head will have size info

    ecs_access(comp::magnitude, snake.back(), size).set(size); // assinng size to head

    // adding head color
    ecs_access(comp::color, snake.back(), r) = 22;
    ecs_access(comp::color, snake.back(), g) = 109;
    ecs_access(comp::color, snake.back(), b) = 240;

    for (std::int32_t i = 0; i < size - 1; i++) {
      followup_buffer.emplace_back(myecs::create_entity());
      snake.emplace_back(myecs::create_entity());

      // adding data to snake
      myecs::add_comp_to<comp::position>(snake.back());
      myecs::add_comp_to<comp::color>(snake.back());
      // adding body color (dark blue)
      ecs_access(comp::color, snake.back(), r) = 75;
      ecs_access(comp::color, snake.back(), g) = 142;
      ecs_access(comp::color, snake.back(), b) = 242;

      myecs::add_comp_to<comp::position>(followup_buffer.back());
    }
  }
  
  static
  void init_snake(std::vector<entity>& entities, const component::type::WidthPix& cell_width, const component::type::HeightPix& cell_height) {
    //assert(size > 0 && "When grouping, size cannot be zero");
    assert(entities.size() != 0 && "entity array cannot be empty");
    
    ecs_access(comp::physics, entities[0], speed).set(2.f);
    ecs_access(comp::rectangle, entities[0], width) = cell_width;
    ecs_access(comp::rectangle, entities[0], height) = cell_height;
    std::int32_t entities_size = ecs_access(comp::magnitude, entities[0], size).get();
    std::int32_t i; // calculations stay within in signed types
    
    for (i = 0; i < entities_size; i++) {
      ecs_access(comp::position, entities[i], x).set(cell_width.get() * ((entities_size - i) - 1));
      ecs_access(comp::position, entities[i], y).set(0);
    } 
  }

  void make_snake_food(entity snake_food) {
    myecs::add_comp_to<comp::rectangle>(snake_food);
    myecs::add_comp_to<comp::position>(snake_food);
    myecs::add_comp_to<comp::color>(snake_food);
  }

  void init_snake_food(entity snake_food) {
    ecs_access(comp::rectangle, snake_food, width).set(mygame::cell_width.get());
    ecs_access(comp::rectangle, snake_food, height).set(mygame::cell_height.get());
    ecs_access(comp::color, snake_food, r) = 200;
    ecs_access(comp::color, snake_food, g) = 0;
    ecs_access(comp::color, snake_food, b) = 0;
  }


  inline void take_movement_input(const entity& id) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && ecs_access(comp::physics, id, direction) != mygame::direction_down && move)
      ecs_access(comp::physics, id, direction) = mygame::direction_up;
    
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && ecs_access(comp::physics, id, direction) != mygame::direction_up && move)
      ecs_access(comp::physics, id, direction) = mygame::direction_down;
    
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && ecs_access(comp::physics, id, direction) != mygame::direction_right && move)
      ecs_access(comp::physics, id, direction) = mygame::direction_left;

    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && ecs_access(comp::physics, id, direction) != mygame::direction_left && move)
      ecs_access(comp::physics, id, direction) = mygame::direction_right;
    move = false;
  }

  void warp_snake(const entity& snake_head, const component::type::WidthPix& window_width, const component::type::HeightPix& window_height) {
    if (ecs_access(comp::position, snake_head, x).get() > window_width.get() - ecs_access(comp::rectangle, snake_head, width).get())
        ecs_access(comp::position, snake_head, x).set(0);

    else
    if (ecs_access(comp::position, snake_head, x).get() < 0)
        ecs_access(comp::position, snake_head, x).set(window_width.get() - ecs_access(comp::rectangle, snake_head, width).get());

    else
    if (ecs_access(comp::position, snake_head, y).get() > window_height.get() - ecs_access(comp::rectangle, snake_head, height).get())
        ecs_access(comp::position, snake_head, y).set(0);

    else
    if (ecs_access(comp::position, snake_head, y).get() < 0)
        ecs_access(comp::position, snake_head, y).set(window_height.get() - ecs_access(comp::rectangle, snake_head, height).get());
  }



  // hot function call (more than 60 times per second)
  inline void move_snake(const std::vector<entity>& snake, const std::vector<entity> followup_buffer) {
    usize = ecs_access(comp::magnitude, snake[0], size).get();
    if (ecs_access(comp::physics, snake[0], direction) == direction_right) {
      ecs_access(comp::position, snake[0], x).set(ecs_access(comp::position, snake[0], x).get() + cell_width.get());
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
    
    } else if (ecs_access(comp::physics, snake[0], direction) == direction_left) {
      ecs_access(comp::position, snake[0], x).set(ecs_access(comp::position, snake[0], x).get() - cell_width.get());
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
    
    } else if (ecs_access(comp::physics, snake[0], direction) == direction_up){
      ecs_access(comp::position, snake[0], y).set(ecs_access(comp::position, snake[0], y).get() - cell_height.get());
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
  
    } else if (ecs_access(comp::physics, snake[0], direction) == direction_down){
      ecs_access(comp::position, snake[0], y).set(ecs_access(comp::position, snake[0], y).get() + cell_height.get());
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
    }
        
  } // end - move_snake()
  
  
  inline void update_followup(const std::vector<entity>& snake, std::vector<entity>& followup_buffer){
    usize = ecs_access(comp::magnitude, snake[0], size).get();
    hot::diff = usize - followup_buffer.size();
    if (usize > followup_buffer.size()) {
      for (int i = 0; i < hot::diff; i++) {
        followup_buffer.emplace_back(myecs::create_entity());
        myecs::add_comp_to<comp::position>(followup_buffer.back());
      }
    }
    for (ui = 0; ui < usize; ui++) {
      ecs_access(comp::position, followup_buffer[ui], x) = ecs_access(comp::position, snake[ui], x);
      ecs_access(comp::position, followup_buffer[ui], y) = ecs_access(comp::position, snake[ui], y);
    }
  }

  void gameover(sf::RenderWindow& ft_window) {
    std::cout << "Gameover";
    sf::Texture tex;
    tex.loadFromFile("Assets/GameOver.png");
    sf::Sprite spr(tex);

    spr.setPosition((ft_window.getSize().x / 2.f) - (tex.getSize().x / 2.f), (ft_window.getSize().y/ 2.f) - (tex.getSize().y / 2.f));

    
    ft_window.setFramerateLimit(10);
    while (ft_window.isOpen()) {

      sf::Event event;
      while (ft_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) ft_window.close();
      }
      ft_window.draw(spr);
      ft_window.display();

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        //restart
        ft_window.setFramerateLimit(60);
        return;
      }
      else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
        exit(0);
      }
      
    }
  }

  void snake_self_collision(const std::vector<entity>& snake, sf::RenderWindow& ft_window) {
    usize = ecs_access(comp::magnitude, snake[0], size).get();
    for (ui = 3; ui < usize; ui++) {
      if (ecs_access(comp::position, snake[0], x) == ecs_access(comp::position, snake[ui], x) &&
          ecs_access(comp::position, snake[0], y) == ecs_access(comp::position, snake[ui], y)
        ) mygame::gameover(ft_window);
      
    }
  }

  template <typename key, typename link>
  inline void init_free_cells(myecs::sparse_set<key, link>& sset, const std::vector<entity>& snake, const component::type::WidthGrid& grid_width, const component::type::HeightGrid& grid_height) {
  
    hot::product = static_cast<std::size_t>(grid_width.get()) * static_cast<std::size_t>(grid_height.get());
    
    for (std::size_t i = 0; i < hot::product; i++) {
      //if (sset.contains(i)) std::cout << "Key already exists\n";
      //else 
      sset.insert(i, i);
    }
    
    // here we are freeing (deleting the occupied) the cells according to the occoupying of snake body (all snake entities)
    for (std::size_t i = 0; i < ecs_access(comp::magnitude, snake[0], size).get(); i++) {
      
      hot::grid_x = ecs_access(comp::position, snake[i], x).get() / mygame::cell_width.get();
      hot::grid_y = ecs_access(comp::position, snake[i], y).get() / mygame::cell_height.get();
      
      hot::key = (grid_width.get() * hot::grid_y) + hot::grid_x;

      assert(sset.contains(hot::key) && "Key does not exists");
      sset.erase(hot::key);
 
    }
    
  }

  // hot function call
  template <typename key, typename link>
  void update_free_cells(myecs::sparse_set<key, link>& sset, const std::vector<entity>& snake_array, const std::vector<entity>& followup_buffer, const component::type::WidthGrid& grid_width) {
    
    // checking if the snake's size has NOT changed
    
    if (
        (ecs_access(comp::position, snake_array.back(), x) != ecs_access(comp::position, followup_buffer.back(), x)) ||
        (ecs_access(comp::position, snake_array.back(), y) != ecs_access(comp::position, followup_buffer.back(), y))
        ){ // not increased so the tail will free a cell

      // cell freed by the sanke's tail, added back to the sparse_set, as it was removed a move before
      hot::grid_x = ecs_access(comp::position, followup_buffer.back(), x).get() / mygame::cell_width.get();
      hot::grid_y = ecs_access(comp::position, followup_buffer.back(), y).get() / mygame::cell_height.get();

      hot::key = (grid_width.get() * hot::grid_y) + hot::grid_x;

      assert(!sset.contains(hot::key) && "Key already exists");
      sset.insert(hot::key, hot::key);
    }
    else {
      std::cout << "\nSkipping tail cell freeing\n";
    }
    // cell occupied by head, removed from the sparse set

    hot::grid_x = ecs_access(comp::position, snake_array.front(), x).get() / mygame::cell_width.get();
    hot::grid_y = ecs_access(comp::position, snake_array.front(), y).get() / mygame::cell_height.get();

    hot::key = (grid_width.get() * hot::grid_y) + hot::grid_x;

    assert(sset.contains(hot::key) && "Key does not exists");
    sset.erase(hot::key);

  }

  template <typename key, typename link>
  void randomize_snake_food_position(entity snake_food, component::type::WidthGrid grid_width, myecs::sparse_set<key, link>& free_cells){
    if (!mygame::food_eaten) return;
    static std::uint32_t PRNG_seed = mgl::make_seed_xorshift32();
    std::cout << PRNG_seed << std::endl;
    auto width = grid_width.get();
    auto pos = mgl::xorshift32(PRNG_seed) % free_cells.dense.size();
    link free_cell_index = free_cells.dense[pos];
    ecs_access(comp::position, snake_food, x).set((free_cell_index % width) * cell_width.get());
    ecs_access(comp::position, snake_food, y).set((free_cell_index / width) * cell_height.get());
    mygame::food_eaten = false;
  }

  void test_eat_food() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) food_eaten = true;
  }
  

  static bool is_food_eaten(entity snake_head, entity food) {
    if (
      ecs_access(comp::position, snake_head, x) == ecs_access(comp::position, food, x) &&
      ecs_access(comp::position, snake_head, y) == ecs_access(comp::position, food, y)
      ) {
      // std::cout << "\nFOOD EATEN\n";
      mygame::food_eaten = true;
      return true;
    }
    return false;
  }

  static void increase_snake_size(std::vector<entity>& snake_array, std::vector<entity>& followup_buffer) {
    snake_array.emplace_back(myecs::create_entity());
    
    myecs::add_comp_to<comp::position>(snake_array.back());
    myecs::add_comp_to<comp::color>(snake_array.back());

    ecs_access(comp::color, snake_array.back(), r) = ecs_access(comp::color, snake_array[1], r);
    ecs_access(comp::color, snake_array.back(), g) = ecs_access(comp::color, snake_array[1], g);
    ecs_access(comp::color, snake_array.back(), b) = ecs_access(comp::color, snake_array[1], b);

    ecs_access(comp::position, snake_array.back(), x) = ecs_access(comp::position, followup_buffer.back(), x);
    ecs_access(comp::position, snake_array.back(), y) = ecs_access(comp::position, followup_buffer.back(), y);
    
    ecs_access(comp::magnitude, snake_array[0], size).set(ecs_access(comp::magnitude, snake_array[0], size).get() + 1);
  }

  inline static void increase_snake_speed(entity snake_head) {
    ecs_access(comp::physics, snake_head, speed).set(
      ecs_access(comp::physics, snake_head, speed).get() + 1.f
    );
  }

  namespace cheat {
    void increase_snake_size(std::vector<entity>& snake_array, std::vector<entity>& folloup_buffer){
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) mygame::increase_snake_size(snake_array, folloup_buffer);
    }
  }

  template <typename key, typename link>
  bool validate_free_cells(const myecs::sparse_set<key, link>& free_cells,
                           const std::vector<entity>& snake_array, 
                           const component::type::WidthGrid width_grid,
                           const component::type::HeightGrid height_grid) {

    static component::type::PosPix_x pix_x{};
    static component::type::PosPix_y pix_y{};
    static component::type::PosGrid_x grid_x{};
    static component::type::PosGrid_y grid_y{};

    static std::int32_t index{};
    
    // if any snake body is in free_cells
    for (std::int32_t i = 0; i < snake_array.size(); i++) { 
      
      pix_x.set(ecs_access(comp::position, snake_array[i], x).get());
      pix_y.set(ecs_access(comp::position, snake_array[i], y).get());

      index = helper::Pix_Pos_TO_GridIndex(pix_x, pix_y, width_grid);
      if (free_cells.contains(index)) { 
        std::cout << "A snake pos is found in free_cells, i : " << i << " x = " << pix_x.get() << " | y = " << pix_y.get() << std::endl;
        return false; 
      }
    }
    if (free_cells.dense.size() != free_cells.index_at_key.size() - ecs_access(comp::magnitude, snake_array.front(), size).get()) {
      std::cout << "free_cells.dense.size() != free_cells.index_at_key.size() - ecs_access(comp::magnitude, snake_array.front(), size).get()";
      return false;
    }
    return true;
  }
} // end of namespace mygame


int main() {
  /*
  entity rectangle = myecs::create_entity();
  entity point = myecs::create_entity();

  //myecs::new_init_component<comp::rectangle>();
  //myecs::new_init_component<comp::position>();
  
  myecs::add_comp_to<comp::rectangle>(rectangle);
  myecs::add_comp_to<comp::position>(rectangle);
  myecs::add_comp_to<comp::position>(point);
  // myecs::add_comp_to<comp::position>(rectangle); // can test the assertion
  myecs::add_comp_to<comp::segment>(point);
  // entity id == 0
  // component id == 0

  // adding data to its component
  myecs::storage<comp::position>::pointer->x[myecs::comp_index_of<comp::position>(point)] = 400;
  myecs::storage<comp::position>::pointer->y[myecs::comp_index_of<comp::position>(point)] = 500;

  myecs::storage<comp::position>::pointer->x[myecs::comp_index_of<comp::position>(rectangle)] = 500;
  myecs::storage<comp::position>::pointer->y[myecs::comp_index_of<comp::position>(rectangle)] = 501;
  
  myecs::storage<comp::rectangle>::pointer->height[myecs::comp_index_of<comp::rectangle>(rectangle)] = 10;
  myecs::storage<comp::rectangle>::pointer->width [myecs::comp_index_of<comp::rectangle>(rectangle)] = 20;


  // reading data from component

  std::cout << "rectangle height: " << myecs::storage<comp::rectangle>::pointer->height[myecs::comp_index_of<comp::rectangle>(rectangle)] << std::endl;
  std::cout << "rectangle component id at entity id (rectangle): " << myecs::comp_index_of<comp::rectangle>(rectangle) << std::endl;
  std::cout << "point component id at entity id (point): " << myecs::comp_index_of<comp::position>(point) << std::endl;
  std::cout << "entity id (rectangle): " << rectangle << std::endl;
  std::cout << "entity id (point): " << point << std::endl;

  std::cout << "point,s x: " << myecs::storage<comp::position>::pointer->x[myecs::comp_index_of<comp::position>(point)] << std::endl;
  std::cout << "point,s y: " << myecs::storage<comp::position>::pointer->y[myecs::comp_index_of<comp::position>(point)] << std::endl;
  std::cout << "rectangle,s x: " << myecs::storage<comp::position>::pointer->x[myecs::comp_index_of<comp::position>(rectangle)] << std::endl;
  std::cout << "rectangle,s y: " << myecs::storage<comp::position>::pointer->y[myecs::comp_index_of<comp::position>(rectangle)] << std::endl;
  
  // deleting component from entities

  std::cout << "before deletion \n sparse[point] = " << myecs::storage<comp::position>::sparse[point];

  myecs::remove_comp_from<comp::position>(point);
  // myecs::remove_comp_from<comp::position>(5000); // can test assertion
  

  std::cout << " \n after deletion \n sparse[point] = " << myecs::storage<comp::position>::sparse[point];
  std::cout << "\nmyecs::storage<comp::position>::size = " << myecs::storage<comp::position>::size;
  std::cout << "\n reverse_sparse[point] = " << myecs::storage<comp::position>::reverse_sparse[0];
  std::cout << std::endl;
  // reading the components very effiently
  // crazy fast for rendering systems needing the vertex arrays of all entities // very cache friendly
  for (int i = 0; i < myecs::storage<comp::position>::size; i++) {
    std::cout << "comp::position' x: " << myecs::storage<comp::position>::pointer->x[i] << std::endl;
    std::cout << "comp::position' y: " << myecs::storage<comp::position>::pointer->y[i] << std::endl;
  }
  */
  
  /*
  myecs::sparse_set<std::size_t, std::string> sets;
  sets.set_link(1, "Hello");
  sets.set_link(2, "apple");
  sets.set_link(3, "car");
  sets.set_link(4, "pen");

  
  std::cout << "sets[1] : " << sets.access(1) << std::endl;
  std::cout << "sets[2] : " << sets.access(2) << std::endl;
  std::cout << "sets[3] : " << sets.access(3) << std::endl;
  std::cout << "sets[4] : " << sets.access(4) << std::endl;

  sets.remove(2);

  for (std::size_t i = 0; i < sets.dense.size(); i++) {
    std::cout << "sets.dense[" << i << "] = " << sets.dense[i] << std::endl;
  }
  */

  

  constexpr component::type::WidthGrid grid_width{ 35 };
  constexpr component::type::HeightGrid  grid_height{ 25 };

  constexpr component::type::WidthPix window_width{ mygame::cell_width.get() * grid_width.get() };
  constexpr component::type::HeightPix window_height{ mygame::cell_height.get() * grid_height.get() };

  sf::Color bg_color(238, 227, 171); // color picked from coolors.co

  const std::string game_window_title = "Snake Game in ECS github@TerminalBoy";

  sf::RenderWindow game_window(sf::VideoMode(window_width.get(), window_height.get()), game_window_title);

  myecs::sparse_set<std::uint32_t, std::uint32_t> free_cells;

  entity snake_food = myecs::create_entity();
  std::vector<entity> snake; // we will allot later
  std::vector<entity> followup_buffer;
  
  mygame::make_snake(snake, followup_buffer, 10); // entities of the bodies are created and components are added
  mygame::make_snake_food(snake_food); // components are added

  mygame::init_snake(snake, mygame::cell_width, mygame::cell_height);
  mygame::init_snake_food(snake_food);
  
  mygame::init_free_cells(free_cells, snake, grid_width, grid_height);
  
  mygame::set_snake_direction(snake[0], mygame::direction_right);
  
  mygame::update_followup(snake, followup_buffer); //followup becomes snake's replica
  
  mygame::update_snake_vertices(snake);
  
  game_window.setFramerateLimit(60); // comment this line to run at full load
  //ecs_access(comp::physics, snake[0], direction) = mygame::direction_right;
  
  ecs_access(comp::physics, snake[0], speed).set(5.f);

  
  float move_interval = 1.f / ecs_access(comp::physics, snake[0], speed).get();
  float time_accumulator = 0.f;
  float dt = 0.f;

  sf::Clock clock;

  // game loop
  while (game_window.isOpen()) {

    dt = clock.restart().asSeconds();
    time_accumulator += dt;

    sf::Event event;
    while (game_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) game_window.close();
    }
    
    move_interval = 1.f / ecs_access(comp::physics, snake[0], speed).get();
    mygame::take_movement_input(snake[0]);
    
    while (time_accumulator >= move_interval) { // this is where snake moves
      //mygame::test_eat_food(); // press 'E' to generate food
      mygame::move_snake(snake, followup_buffer);
      mygame::warp_snake(snake[0], window_width, window_height);
      mygame::snake_self_collision(snake, game_window);
      mygame::randomize_snake_food_position(snake_food, grid_width, free_cells);
      
      //mygame::cheat::increase_snake_size(snake, followup_buffer);
      
      if (mygame::is_food_eaten(snake[0], snake_food)) {
        mygame::increase_snake_size(snake, followup_buffer);
        mygame::increase_snake_speed(snake[0]);
      }
     
      mygame::update_free_cells(free_cells, snake, followup_buffer, grid_width);
      mygame::update_followup(snake, followup_buffer);
  
      mygame::update_snake_vertices(snake);
      mygame::update_snake_food_vertices(snake_food);
      time_accumulator -= move_interval;
    }

    
    
    game_window.clear(bg_color); // color picked from coolors.co
    game_window.draw(mygame::renderables::snake);
    if (!mygame::food_eaten)
    game_window.draw(mygame::renderables::snake_food);
    game_window.display();

    //assert(mygame::validate_free_cells(free_cells, snake, grid_width, grid_height) && "free_cell invariant is broken");
  }

  // ~game loop


  
  /*
  // tests 
  entity var1 = myecs::create_entity();
  entity var2 = myecs::create_entity();
  entity var3 = myecs::create_entity();
  entity var4 = myecs::create_entity();

  // adding components
  myecs::add_comp_to<comp::position>(var1);
  myecs::add_comp_to<comp::position>(var3);
  myecs::add_comp_to<comp::position>(var2);
 // myecs::add_comp_to<comp::position>(var4);
  
  myecs::remove_comp_from<comp::position>(var3);
  myecs::add_comp_to<comp::position>(var3);

  std::cout << "myecs::has_component<comp::position>(var1) : " << myecs::has_component<comp::position>(var1) << std::endl;
  std::cout << "myecs::has_component<comp::position>(var2) : " << myecs::has_component<comp::position>(var2) << std::endl;
  std::cout << "myecs::has_component<comp::position>(var3) : " << myecs::has_component<comp::position>(var3) << std::endl;
  std::cout << "myecs::has_component<comp::position>(var4) : " << myecs::has_component<comp::position>(var4) << std::endl;

  std::cout << "myecs::has_entity<comp::position>(var1) : " << myecs::has_entity<comp::position>(myecs::comp_index_of<comp::position>(var1)) << std::endl;
  std::cout << "myecs::has_entity<comp::position>(var2) : " << myecs::has_entity<comp::position>(myecs::comp_index_of<comp::position>(var2)) << std::endl;
  std::cout << "myecs::has_entity<comp::position>(var3) : " << myecs::has_entity<comp::position>(myecs::comp_index_of<comp::position>(var3)) << std::endl;
  std::cout << "myecs::has_entity<comp::position>(var4) : " << myecs::has_entity<comp::position>(myecs::comp_index_of<comp::position>(var4)) << std::endl;
  
  //std::cout << "myecs::comp_index_of<comp::position>(var4) : "; myecs::comp_index_of<comp::position>(var4);
  */
  
  return 0;
}


/*

// Forward Declarations
class snake;

class snake_part;

class snake_food;


// Class Declarations

class game_window{
  public:
  
  enum class display_state{on, off};
  enum class text_align{left, right};
  int width = 0; // Width and Height of game screen 
  int height = 0;
  int max_x = 0; // (Usually width - 1)
  int max_y = 0; // (Usually height - 1)
  sf::Font font;
  sf::Text text;
  sf::RenderWindow sf_window;
  sf::Event event{};
  sf::Clock clock; // for frame control
  sf::Time delta_time;
  std::string title = "";
  
  game_window(){}
  game_window(int f_width, int f_height, std::string, display_state f_display_state = display_state::on);
  
  void set_font(sf::Font& f_font);
  inline void fps_handler_reset();

  static void text_aligner(sf::Text& ft_text, game_window& ft_window, text_align f_arg = text_align::left);

};

class snake_part{
  public:
  //int x = 0; // Coordinates of current blocks
  //int y = 0;
  
  sf::Vector2f position;
  sf::Vector2f followup;  //Coordinates of previous blocks
  
  static sf::RectangleShape shape;
  snake_part(){}
  snake_part(sf::Vector2f pos);
  snake_part(int x, int y);

};

class snake { // Will hold Snake information  
  
  public:
  
  enum class input_style {arrow, wasd};
  
  float speed = 5.0f;
  //float move_interval= 1.0f / speed;
  float move_interval = 0;
  float move_timer = 0.0f;
  
  int score = 0;

  static int count;
  sf::Text text;
  enum class direction {up, down, left, right, stop};
  
  direction dir = direction::stop;
 
  std::vector<snake_part> part;
  
  
  snake();
  snake(int f_size, int f_speed);

  inline void stop();
    
  void draw_snake(sf::RenderWindow& f_window, sf::Color f_head_color, sf::Color f_body_color);
  
    
  void init(float f_size, float f_speed);
  
  void update_speed(float f_speed); //updates speed live
  void plus_size(int f_size);
  void minus_size(int f_size);  
  void set_size(const int& f_size); // is slow and has overheads
  
  inline int get_size() const; //read only

  void process_input(game_window& ft_window, input_style f_input_style = input_style::arrow);
  void process_movement(game_window& ft_window);
  
  template <int N>
  void process_eating(snake_food* const (&f_food)[N]);
  
  template <int N>
  static void process_gameover(snake* const (&snakes)[N], game_window& ft_window);

  void process_self_collision(game_window& ft_window);
  
  static void process_other_collision(snake* const (&snakes)[], game_window& ft_window);
  
  template <int N>
  static void display_score(snake* const (&snakes)[N], game_window& ft_window);
  

  ~snake();

};



class snake_food{
  public:
  
  float radius = 10.f;
  sf::Color color;

  sf::Vector2f position;
  sf::CircleShape shape;
  
  bool eaten = false;
  
  snake_food();
  
  //void put_food(snake& f_snake, sf::RenderWindow& f_window);
  template <int N>
  void put_food(const snake (&f_snake)[N], sf::RenderWindow& f_window);
};




// Static Var Declarations

int snake::count = 0;

// -------------------------------------------

// Out of Class, Member function declaritions - To resolve Circular Dependencies

game_window::game_window(int f_width, int f_height, std::string f_title, display_state f_display_state){
    width = f_width;
    height = f_height;
    max_x = f_width - 1;
    max_y = f_height - 1;
    title = f_title;
    font.loadFromFile("Figtree-VariableFont_wght.ttf");
    text.setFont(font);
    delta_time = clock.restart();
    
    if (f_display_state == display_state::on)
      sf_window.create(sf::VideoMode(f_width, f_height + 20), f_title);
    sf_window.setFramerateLimit(60);
}

void game_window::set_font(sf::Font& f_font){
  if (!font.loadFromFile("Figtree-VariableFont_wght.ttf")){
    return;
  }
}

inline void game_window::fps_handler_reset(){
  delta_time = clock.restart();
}

void game_window::text_aligner(sf::Text &ft_text, game_window &ft_window, text_align f_arg) {
  sf::FloatRect bounds = ft_text.getLocalBounds();
  if (f_arg == text_align::right) {
    ft_text.setPosition(ft_window.width - (bounds.width + bounds.left), ft_text.getPosition().y);
  }
}

snake_part::snake_part(sf::Vector2f pos){
  position = pos;
}

snake_part::snake_part(int x, int y){
  position.x = x;
  position.y = y;
}

snake::snake() 
: dir(direction::stop) {
  stop();
}

snake::snake(int f_size, int f_speed){
  stop();
  set_size(f_size);
  update_speed(f_speed);
}



inline void snake::stop() {
  dir = snake::direction::stop;
}

void snake::draw_snake(sf::RenderWindow& f_window, sf::Color f_head_color, sf::Color f_body_color){
  //sf::Color temp = snake_part::shape.getFillColor();
 
  
  snake_part::shape.setPosition(part[0].position);
  snake_part::shape.setFillColor(f_head_color);
  f_window.draw(snake_part::shape);
    
  snake_part::shape.setFillColor(f_body_color);
    
  for (int i = 1; i < get_size(); i++){
      snake_part::shape.setPosition(part[i].position);
      f_window.draw(snake_part::shape);
  }
  
}


void snake::init(float f_size, float f_speed){

  set_size(f_size);
  update_speed(f_speed);
  score = 0;
  for (int i = 0; i < get_size(); i++){
    part[i].position.x = (20 * (get_size() - i - 1));
    part[i].position.y = 0.f + (20 * snake::count);
  }
  snake::count += 1;
  dir = snake::direction::stop;
  move_timer = 0;
}


void snake::update_speed(float f_speed){ //updates speed live
  speed = f_speed;
  move_interval= 1.0f / speed;
}

void snake::plus_size(int f_size){
  int temp = part.size() - 1;
  for (int i = 0; i < f_size; i++){
    part.emplace_back();
    if (part.size() > 1)
      part[part.size() - 1].position = part[get_size() - 2].position;
    //part[part.size() - 1].position.y = part[get_size() - 2].position.y;
  }
}

void snake::minus_size(int f_size){
  if (f_size > part.size()) f_size = part.size(); // bounds checking
  for (int i = 0; i < f_size; i++){
    part.pop_back();
  }
}

void snake::set_size(const int& f_size){ // is slow and has overheads
  if (f_size > part.size())
    plus_size(f_size - part.size());
    
  else if (f_size < part.size())
    minus_size(part.size() - f_size);

}

inline
int snake::get_size() const{
  return part.size();
}


void snake::process_input(game_window& ft_window, input_style f_input_style){
  
  sf::Keyboard::Key target_up = sf::Keyboard::Up;
  sf::Keyboard::Key target_down = sf::Keyboard::Down;
  sf::Keyboard::Key target_left = sf::Keyboard::Left;
  sf::Keyboard::Key target_right = sf::Keyboard::Right;
  
  if (f_input_style == input_style::wasd){
    target_up = sf::Keyboard::W;
    target_down = sf::Keyboard::S;
    target_left = sf::Keyboard::A;
    target_right = sf::Keyboard::D;
  }
  
  if (sf::Keyboard::isKeyPressed(target_down) && dir != snake::direction::up){
    dir = snake::direction::down;
  } else if (sf::Keyboard::isKeyPressed(target_up) && dir != snake::direction::down){
    dir = snake::direction::up;
  } else if (sf::Keyboard::isKeyPressed(target_left) && dir != snake::direction::right){
    dir = snake::direction::left;
  } else if (sf::Keyboard::isKeyPressed(target_right) && dir != snake::direction::left){
    dir = snake::direction::right;
  }  

}


void snake::process_movement(game_window& ft_window) {

    
    // Update Followup
    for (int i = 0; i < get_size(); i++){
      part[i].followup = part[i].position;
    }    
    //==================
  
    // Snake Movement Logic :
    if (dir == snake::direction::down){ //down
      part[0].position.y += 20.f;
      
      if (part[0].position.y >= ft_window.max_y) //Teleportation when out of screen
      {part[0].position.y = 0;}
      
      for (int i = 1; i < get_size(); i++){
        part[i].position = part[i - 1].followup;
      }
    }
    
    else if (dir == snake::direction::up){ // up
      part[0].position.y -= 20.f;
      
      if (part[0].position.y <= -20)
      {part[0].position.y = ft_window.max_y - 19;} 
      
      for (int i = 1; i < get_size(); i++){
        part[i].position = part[i - 1].followup;
      }
    }
    
    else if (dir == snake::direction::left){ // left
      part[0].position.x -= 20.f;
      
      if (part[0].position.x <= -20)
      {part[0].position.x = ft_window.max_x - 19;}
              
      for (int i = 1; i < get_size(); i++){
        part[i].position = part[i - 1].followup;
      }
    }
    
    else if (dir == snake::direction::right){ // right
      part[0].position.x += 20.f;
      
      if (part[0].position.x >= ft_window.max_x) 
      {part[0].position.x = 0;}
      
      for (int i = 1; i < get_size(); i++){
        part[i].position = part[i - 1].followup;
      }
    }
    
  
  
}

template <int N>
void snake::process_eating(snake_food* const (&f_food)[N]){
// to detect if food is eaten or not 
  for(int __i = 0; __i < N; __i++){
    if (part[0].position == f_food[__i]->shape.getPosition()){
      f_food[__i]->eaten = true;
      update_speed(speed + 0.5f);
      
      //if (snk.size > snk.get_max_size() - 2) snk.transform(snk.get_max_size() + 100);
      plus_size(1);
      score++;
      // Debug //cout<<endl<<"Food Eated"<<endl;
    }  
  }
}

template <int N>
void snake::process_gameover(snake* const (&snakes)[N], game_window& ft_window){
  ft_window.fps_handler_reset();
  snake::count = 0;
  //sf::RenderWindow game_over_window (sf::VideoMode(ft_window.width, ft_window.height), ft_window.title);
  
  //window.clear(sf::Color::White);

  sf::Texture tex;
  tex.loadFromFile("Assets/GameOver.png");
  sf::Sprite spr(tex);

  spr.setPosition((ft_window.width / 2.f) - (tex.getSize().x / 2.f), (ft_window.height / 2.f) - (tex.getSize().y / 2.f));
 
  //ft_window.sf_window.create(sf::VideoMode(ft_window.width, ft_window.height), ft_window.title);
  //ft_window.sf_window.display();
 

  while (ft_window.sf_window.isOpen()) {
    while (ft_window.sf_window.pollEvent(ft_window.event)) {

      if (ft_window.event.type == sf::Event::Closed) {
        ft_window.sf_window.close();
      }
    } //while (game_over_window.pollEvent(game_over_event))

  
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
      ft_window.sf_window.close();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
      for (int __i = 0; __i < N; __i++) snakes[__i]->init(5, 5);
      return;
    }
    

    ft_window.sf_window.clear(sf::Color::White);
    ft_window.sf_window.draw(spr);
    ft_window.sf_window.display();

     
  } //while(game_over_window.isOpen())
  
}

void snake::process_self_collision(game_window& ft_window){

  // to detect self collision
  for (int i = 1; i < get_size(); i++){ // Check Self Collision
    if (part[0].position == part[i].position) {
      process_gameover({ this }, ft_window);
      break;
    }
  
  }// for loop ends

}


void snake::process_other_collision(snake* const (&snakes)[], game_window& ft_window) {
  
  for (int i = 0; i < snakes[1]->get_size(); i++) {
    if (snakes[0]->part[0].position == snakes[1]->part[i].position){
      std::cout << "Collision";
      snake::process_gameover({ snakes[0], snakes[1] }, ft_window);
      return;
    }
  }

  for (int i = 0; i < snakes[0]->get_size(); i++) {
    if (snakes[1]->part[0].position == snakes[0]->part[i].position){
      std::cout << "Collision";
      snake::process_gameover({ snakes[0], snakes[1] }, ft_window);
      return;
    }
  }

}

template <int N>
void snake::display_score(snake* const (&snakes)[N], game_window& ft_window) {
  ft_window.text.setCharacterSize(18);
  ft_window.text.setFont(ft_window.font);
  ft_window.text.setFillColor(sf::Color::Black);

  ft_window.text.setString("Snake 1 score : " + std::to_string(snakes[0]->score));
  ft_window.text.setPosition(50, ft_window.height);
  ft_window.sf_window.draw(ft_window.text);
  if (N == 2) {
    ft_window.text.setString("Snake 2 score : " + std::to_string(snakes[1]->score));
    ft_window.text.setPosition(0, ft_window.height);
    game_window::text_aligner(ft_window.text, ft_window, game_window::text_align::right);
    ft_window.text.setPosition(ft_window.text.getPosition().x - 50, ft_window.text.getPosition().y);
    ft_window.sf_window.draw(ft_window.text);
  }
  
}

snake::~snake(){}

snake_food::snake_food() {
  shape.setRadius(10.f);
  color = sf::Color::Red;
  eaten = true;
}

template <int N>
void snake_food::put_food(const snake (&f_snake)[N], sf::RenderWindow& f_window){

  if (!eaten) {
    f_window.draw(shape);
    return;
  }
  
  int ran_x;
  int ran_y;
  
  bool exit_loop = false;
  
  do{
    
    ran_x = rand() % 35; //(game_window::max_x / 20)
    ran_y = rand() % 25; //(game_window::max_y / 20)
    
    position.x = ran_x * 20; 
    position.y = ran_y * 20;
    
    // make sure food dosent come on any snake body
    for (int ix = 0; ix < N; ix++){
    
    for (int i = 0; i < f_snake[ix].get_size(); i++){
      // make the logic for not adding food on snake, snake body/part
      if (position == f_snake[ix].part[i].position) {
        exit_loop = false;
        break;
      }
      exit_loop = true;
      std::cout<<"Evaluating the food position "<<i<<"\n"; 
    }
    
    if(!exit_loop) break;
    
    }
  } while (exit_loop == false);
  
  eaten = false;
    
  shape.setPosition(position);
  shape.setFillColor(color);
  f_window.draw(shape);
}

// ============================


// Global Declarations
sf::RectangleShape snake_part::shape;
//=============================


int main(){
  srand(time(0));
  
  game_window g_window(700, 500, "Snake Game - @skshazkamil");
  
  game_window game_over(200, 100, "GameOver", game_window::display_state::off);
  
  snake_part::shape = sf::RectangleShape(sf::Vector2f(20.f, 20.f));
  snake_part::shape.setFillColor(sf::Color(150, 150, 150));
  snake_part::shape.setOutlineColor(sf::Color::White);
  snake_part::shape.setOutlineThickness(1.f);

  snake snk;
  snake snk2; 
  
  snake_food food;

  snk.init(5, 5);
  snk2.init(5, 5);

  snk.stop();
  snk2.stop();



  while(g_window.sf_window.isOpen()){
    
    while (g_window.sf_window.pollEvent(g_window.event)){
      if (g_window.event.type == sf::Event::Closed) g_window.sf_window.close();
    }
    
    g_window.delta_time = g_window.clock.restart();
    snk.move_timer += g_window.delta_time.asSeconds();
    snk2.move_timer += g_window.delta_time.asSeconds();
  
    

    
    if (snk.move_timer >= snk.move_interval){ //Frames as per snake speed
      snk.move_timer -= snk.move_interval;
    
    
      snk.process_input(g_window, snake::input_style::arrow);
      //snk2.process_input(g_window, snake::input_style::wasd);
      
      snk.process_movement(g_window);
      //snk2.process_movement(g_window);
      
      snk.process_eating({&food});
      //snk2.process_eating({&food});
      
      snk.process_self_collision(g_window);
      //snk2.process_self_collision(g_window, game_over);

      snake::process_other_collision({ &snk, &snk2 }, g_window);
    
    }

    if (snk2.move_timer >= snk2.move_interval) { //Frames as per snake speed
      snk2.move_timer -= snk2.move_interval;


      //snk.process_input(g_window, snake::input_style::arrow);
      snk2.process_input(g_window, snake::input_style::wasd);

      //snk.process_movement(g_window);
      snk2.process_movement(g_window);

      //snk.process_eating({ &food });
      snk2.process_eating({ &food });

      //snk.process_self_collision(g_window, game_over);
      snk2.process_self_collision(g_window);

      snake::process_other_collision({ &snk, &snk2 }, g_window);

    }
    
    //Main window =======
    g_window.sf_window.clear(sf::Color::White);
    food.put_food({snk}, g_window.sf_window);

    snake::display_score({ &snk, &snk2 }, g_window);

    snk.draw_snake(g_window.sf_window, sf::Color(100, 200, 100), sf::Color(50, 250, 50));
    snk2.draw_snake(g_window.sf_window, sf::Color(100, 100, 200), sf::Color(50, 50, 250));
    
    g_window.sf_window.display();    
  } // Main loop ends
  return 0;
}

*/
