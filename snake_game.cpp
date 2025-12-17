

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

#include <memory>
#include <iostream> // only  for debugging
#include <cstdlib>  
#include <vector>
#include <string>
//#include <array> - not in use
//#include <unordered_map> - not in use
#include <limits>
#include <cassert>

#include <algorithm>
#include <execution>
#include <SFML/Graphics.hpp>
//#include "Dependencies/SFML/include/SFML/Graphics.hpp" // i am sorry for this mess, but the library has hard coded the "<SFML/Graphics/**>" paths
#include "Dependencies/Custom_ECS/memory.hpp"
#include "Dependencies/Custom_ECS/components.hpp"
#include "Dependencies/Custom_ECS/generated_components_create.hpp"
#include "Dependencies/Custom_ECS/generated_components_delete.hpp"
//#include <Custom_ECS_libs/memory.hpp>// will test later




// 

// ECS : enitity component system 
// 
// why use it instead of OOP style
// : it provides better cache usage, by utilizing contigious memory and making the compiler easy to apply SIMD
// optimizations, lesser missed cache, lesser ram fetchinmg = overall better perfomance
// And its very easy to scale than the classic OOP structure

// Entities :
// its is just an id referrinmg to a game object (not the OOP one), like snake, snake bodypart, window, food

// Components :
// this the data of game objects stored in struct or arrays (SoA), acessed by specifying the entitiy id or by a internal Hash Map (unordered_map)

// System :
// this is the actual logic of the game, stored in functions

// 



// trying ECS


// Entity
// 
// entity defined in memory.hpp in "Dependencies/Custom_ECS/memory.hpp"
//
// using entity = std::size_t;
//

static entity GLOBAL_ENTITY_COUNTER = 0; 

constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();
constexpr std::size_t MULTITHREADING_SEED = 50000; // (no. of allocations )from the threshold where multithreading should start
// COMPONENTS :
 
// defined in "components.hpp"


// SYSTEM :

// i am sorry for these messy comments, having a tough time making the code architecture, but i am tring my best

// creation of entity == entity++; // only counter is incremented

// initialization of component (from scratch) == first component is stored in heap with its adress stored in 
// static unique_ptr (just initial address storage) -> this is just the creation of component container / structure

// no bridging needed now!
// (only initiliazed if used -> type id dispatch, only compile-time)


// creation of component == as the component is initialized now -> we will increment its static size counter 
// to keep track of its array sizes at runtime -> bridging of components is done with the entities

// component contents acessed only by its static - unique_ptr
  

namespace myecs {
  
  

  // A new way to store metadata 
  // completely elimininates, function statics' guard checks,
  // increasing prefomance exponentially in large ecs data and systems
  // here the metadata is initialized before main() and dosent involve runtime guard checks
  // only pure fast, type id dispatched fetching // as fast as fetching a normal global variable
  // even at the cpu instruction level (no extra cycles)
  template <typename component>
  struct storage {
    inline static std::size_t size = 0;
    inline static std::unique_ptr<component> pointer = std::make_unique<component>();
    inline static myecs::d_array<std::size_t> sparse; // sparse<component>[entity_id] = component_index 
                                            //^^^^^^there is not even a need for a dense array because the components arrays are always dense by design
    inline static myecs::d_array<std::size_t> reverse_sparse; // reverse_sparse<component>[component_index] = owning_entity_id;
  };
 
  template <typename component>
  void sparse_allocator(const entity& id, std::size_t corresponding_comp_index) { // allocates and links the values
    assert(id < GLOBAL_ENTITY_COUNTER && "The provided entity never existed");
    std::size_t old_reverse_sparse_size = myecs::storage<component>::reverse_sparse.size();
    std::size_t old_sparse_size = myecs::storage<component>::sparse.size();
    

    // I dont know why guards, may it bring peace my mind
    if (old_sparse_size < GLOBAL_ENTITY_COUNTER)
      myecs::storage<component>::sparse.resize(GLOBAL_ENTITY_COUNTER);
    
    if (old_reverse_sparse_size < myecs::storage<component>::size)
      myecs::storage<component>::reverse_sparse.resize(myecs::storage<component>::size);
    
    // trying multithreading, i recently got introcuded to this concept
    if (GLOBAL_ENTITY_COUNTER - old_sparse_size >= MULTITHREADING_SEED) { // guards for where multithreading is not required (thread safety) preventing undefined behavior
      //std::cerr << "\nYes multithreading\n";
      std::fill(
        std::execution::par_unseq,
        myecs::storage<component>::sparse.begin() + old_sparse_size,
        myecs::storage<component>::sparse.end(),
        INVALID_INDEX
      );
    }
    else { // no multithreading
      //std::cerr << "\nNo multithreading\n";
      std::fill(
        myecs::storage<component>::sparse.begin() + old_sparse_size,
        myecs::storage<component>::sparse.end(),
        INVALID_INDEX
      );
    }

    if (GLOBAL_ENTITY_COUNTER - old_reverse_sparse_size >= MULTITHREADING_SEED) {
      //std::cerr << "\nYes multithreading\n";
      std::fill(
        std::execution::par_unseq,
        myecs::storage<component>::reverse_sparse.begin() + old_reverse_sparse_size,
        myecs::storage<component>::reverse_sparse.end(),
        INVALID_INDEX
      );
    }
    else {
      //std::cerr << "\nNo multithreading\n";
      std::fill(
        myecs::storage<component>::reverse_sparse.begin() + old_reverse_sparse_size,
        myecs::storage<component>::reverse_sparse.end(),
        INVALID_INDEX
      );
    }

    myecs::storage<component>::sparse[id] = corresponding_comp_index;
    myecs::storage<component>::reverse_sparse[corresponding_comp_index] = id;
  }


  // ecs functions

  entity create_entity() { // first step
    GLOBAL_ENTITY_COUNTER++;
    return GLOBAL_ENTITY_COUNTER - 1;
  }

  template <typename component> // helper only, no need to call explictly
  inline void entity_component_linker(const entity& id, std::size_t corresponding_comp_index) {
    assert(id < GLOBAL_ENTITY_COUNTER && "The provided entity never existed");
    sparse_allocator<component>(id, corresponding_comp_index);
  }

 
  template <typename component>
  bool has_component(const entity& id) { // (has branching) only to be used in asserts
    assert(id < GLOBAL_ENTITY_COUNTER && "The provided entity never existed");
    if (id < myecs::storage<component>::sparse.size() && myecs::storage<component>::sparse[id] != INVALID_INDEX) return true;
    return false;
  }

  template <typename component>
  bool has_entity(const std::size_t& comp_index) { // (has branching) only to be used in asserts
    if (comp_index < myecs::storage<component>::reverse_sparse.size() && myecs::storage<component>::reverse_sparse[comp_index] != INVALID_INDEX) return true;
    return false;
  }

  template <typename component>
  void add_comp_to(const entity& id) { // third step
    //assert((id >= myecs::storage<component>::sparse.size() || myecs::storage<component>::sparse[id] == INVALID_INDEX) && "Component already exists for the provided entity id");
    assert(id < GLOBAL_ENTITY_COUNTER && "The provided entity never existed");
    assert(myecs::has_component<component>(id) == false && "Component already exists for the provided entity id");
    myecs::create_component(myecs::storage<component>::pointer); // from "generated_components_create.hpp"
    myecs::storage<component>::size++;
    entity_component_linker<component>(id, myecs::storage<component>::size - 1); 
  
  }

  template <typename component>
  void remove_comp_from(const entity& id) {
    assert(id < GLOBAL_ENTITY_COUNTER && "The provided entity never existed");
    //assert((id < myecs::storage<component>::sparse.size() && myecs::storage<component>::sparse[id] != INVALID_INDEX) && "Component does not exist for the provided entity id");
    assert(myecs::has_component<component>(id) && "Component does not exist for the provided entity id");
    myecs::d_array<std::size_t>& sparse = myecs::storage<component>::sparse;
    myecs::d_array<std::size_t>& reverse_sparse = myecs::storage<component>::reverse_sparse;
    
    std::size_t component_remove_index = sparse[id];
    std::size_t component_last_index_bd = myecs::storage<component>::size - 1; //_bd = before deletion
    std::size_t entity_id_at_last_component_bd = reverse_sparse[component_last_index_bd]; //_bd = before deletion
    std::size_t entity_id_at_component_remove_index = reverse_sparse[component_remove_index];

    if (component_remove_index != component_last_index_bd) {
      // deleting component at component_remove_index
      myecs::delete_component(myecs::storage<component>::pointer, component_remove_index); // from "generated_components_delete.hpp"
      
      // updating maps of deleted components
      sparse[id] = INVALID_INDEX;
      reverse_sparse[component_last_index_bd] = INVALID_INDEX; // not updating reverse_sparse[component_remove_index] because at the place of that removed old component, last component will be copied
       
      // updating maps of shifted components 
      sparse[entity_id_at_last_component_bd] = component_remove_index;
      reverse_sparse[component_remove_index] = entity_id_at_last_component_bd;
    }
    else {
      myecs::delete_component(myecs::storage<component>::pointer, component_remove_index); // from "generated_components_delete.hpp"
      sparse[id] = INVALID_INDEX;
      reverse_sparse[component_last_index_bd] = INVALID_INDEX;
    }
    myecs::storage<component>::size--;
  }

  
  template <typename component>
  inline const std::size_t& comp_index_of(const entity& id) {
    assert(id < GLOBAL_ENTITY_COUNTER && "The provided entity never existed");
    assert(myecs::has_component<component>(id) && "Entity does not have the component");
    return myecs::storage<component>::sparse[id];
  }

  template <typename component>
  inline const std::size_t& entity_index_of(const std::size_t& comp_index) {
    assert(comp_index < myecs::storage<component>::reverse_sparse.size() && "Component index out of bounds");
    assert(myecs::has_entity<component>(comp_index) && "Component index does not have an entity");
    return myecs::storage<component>::reverse_sparse[comp_index];
  }
}


#define comp_pointer(component_type_only) myecs::storage<component_type_only>::pointer
#define comp_index_of_en(entity, component_type) myecs::comp_index_of<component_type>(entity)
#define ecs_access(component_type, entity_id, element) myecs::storage<component_type>::pointer->element[myecs::comp_index_of<component_type>(entity_id)]
#define access_fill(component_type, entity_id, element, value) myecs::storage<component_type>::pointer->element[myecs::comp_index_of<component_type>(entity_id)] = value



namespace mygame {
  
  std::size_t ui = 0; // universal declarations for hot calls, avoids branch prediction
  std::size_t usize = 0; 

  constexpr std::uint32_t cell_width = 20;
  constexpr std::uint32_t cell_height = 20;
  constexpr float direction_left = 0;
  constexpr float direction_right = 1;
  constexpr float direction_up = 2;
  constexpr float direction_down = 3;

  //template <typename T, typename... args>
  struct renderables{
    inline static sf::VertexArray snake;
    inline static sf::VertexArray snake_food;
  };

  static
    void set_snake_direction(const entity& snake_head, const float& snake_direction) {
    ecs_access(comp::physics, snake_head, direction) = snake_direction;
  }

  static
  void update_snake_vertices(const std::vector<entity>& entities) { 
    //std::cout << "entitiy/ snake size : " << entities.size() << std::endl;
    std::size_t entities_size = entities.size();
    
    renderables::snake.setPrimitiveType(sf::Quads);
    renderables::snake.resize(entities_size * 4);
    
    std::size_t base = 0;
    std::size_t i = 0;

    const float& width = ecs_access(comp::rectangle, entities[0], width);
    const float& height = ecs_access(comp::rectangle, entities[0], height);

   
    for (i = 0; i < entities_size; i++) {
    
      base = i * 4;

      const float& x = ecs_access(comp::position, entities[i], x);
      const float& y = ecs_access(comp::position, entities[i], y);
      
      renderables::snake[base + 0].position = { x, y }; // top left

      renderables::snake[base + 1].position = { x + width, y };// top right

      renderables::snake[base + 2].position = { x + width, y + height }; // bottom right

      renderables::snake[base + 3].position = { x, y + height }; // bottom left
      
      // 
      renderables::snake[base + 0].color = sf::Color::Green;
      renderables::snake[base + 1].color = sf::Color::Green;
      renderables::snake[base + 2].color = sf::Color::Green;
      renderables::snake[base + 3].color = sf::Color::Green;
    }
    
  }

  static
  void make_snake(std::vector<entity>& snake, std::vector<entity>& followup_buffer, const std::uint16_t& size) {
    assert(size != 0 && "Poor snake, you didnt give it a size and also took his head | size cannot be 0");
    assert(snake.size() == 0 && "Please dont destroy another snake to create your own | snake array should be empty in order to create entities");
    assert(followup_buffer.size() == 0 && "Followup buffer should be empty");

    followup_buffer.emplace_back(myecs::create_entity());
    snake.emplace_back(myecs::create_entity()); // head

    myecs::add_comp_to<comp::position>(followup_buffer[followup_buffer.size() - 1]);
    myecs::add_comp_to<comp::position>(snake[snake.size() - 1]);

    myecs::add_comp_to<comp::rectangle>(snake[snake.size() - 1]); // only the head will have rectangle info
    myecs::add_comp_to<comp::physics>(snake[snake.size() - 1]); // only the head will have physics info
    
    for (std::uint16_t i = 0; i < size - 1; i++) {
      followup_buffer.emplace_back(myecs::create_entity());
      snake.emplace_back(myecs::create_entity());
      myecs::add_comp_to<comp::position>(snake[snake.size() - 1]);
      myecs::add_comp_to<comp::position>(followup_buffer[followup_buffer.size() - 1]);
    }
  }
  
  static
  void init_snake(std::vector<entity>& entities, const std::uint32_t& cell_width, const std::uint32_t& cell_height) {
    //assert(size > 0 && "When grouping, size cannot be zero");
    assert(entities.size() != 0 && "entity array cannot be empty");
    
    ecs_access(comp::physics, entities[0], speed) = 2;
    ecs_access(comp::rectangle, entities[0], width) = cell_width;
    ecs_access(comp::rectangle, entities[0], height) = cell_height;
    std::size_t entities_size = entities.size();
    std::size_t i; // avoid branch guessing
    
    for (i = 0; i < entities_size; i++) {
      ecs_access(comp::position, entities[i], x) = static_cast<std::size_t>(cell_width) * ((entities_size - i) - 1);
      ecs_access(comp::position, entities[i], y) = 0;
    } 
  }


  inline void take_movement_input(const entity& id) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && ecs_access(comp::physics, id, direction) != mygame::direction_down)
      ecs_access(comp::physics, id, direction) = mygame::direction_up;
    
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && ecs_access(comp::physics, id, direction) != mygame::direction_up) 
      ecs_access(comp::physics, id, direction) = mygame::direction_down;
    
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && ecs_access(comp::physics, id, direction) != mygame::direction_right) 
      ecs_access(comp::physics, id, direction) = mygame::direction_left;

    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && ecs_access(comp::physics, id, direction) != mygame::direction_left) 
      ecs_access(comp::physics, id, direction) = mygame::direction_right;
  }

  void warp_snake(const entity& id, const std::uint32_t& window_width, const std::uint32_t& window_height) {
    if (ecs_access(comp::position, id, x) > static_cast<float>(window_width) - ecs_access(comp::rectangle, id, width))
      ecs_access(comp::position, id, x) = 0;

    else if (ecs_access(comp::position, id, x) < 0)
      ecs_access(comp::position, id, x) = static_cast<float>(window_width) - ecs_access(comp::rectangle, id, width);

    else if (ecs_access(comp::position, id, y) > static_cast<float>(window_height) - ecs_access(comp::rectangle, id, height))
      ecs_access(comp::position, id, y) = 0;

    else if (ecs_access(comp::position, id, y) < 0)
      ecs_access(comp::position, id, y) = static_cast<float>(window_height) - ecs_access(comp::rectangle, id, height);
  }



  // hot function call (more than 60 times per second)
  inline void move_snake(const std::vector<entity>& snake, const std::vector<entity> followup_buffer) {
    usize = snake.size();
    if (ecs_access(comp::physics, snake[0], direction) == direction_right) {
      ecs_access(comp::position, snake[0], x) += cell_width;
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
    
    } else if (ecs_access(comp::physics, snake[0], direction) == direction_left) {
      ecs_access(comp::position, snake[0], x) -= cell_width;
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
    
    } else if (ecs_access(comp::physics, snake[0], direction) == direction_up){
      ecs_access(comp::position, snake[0], y) -= cell_height;
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
  
    } else if (ecs_access(comp::physics, snake[0], direction) == direction_down){
      ecs_access(comp::position, snake[0], y) += cell_height;
      for (ui = 1; ui < usize; ui++) {
        ecs_access(comp::position, snake[ui], x) = ecs_access(comp::position, followup_buffer[ui - 1], x);
        ecs_access(comp::position, snake[ui], y) = ecs_access(comp::position, followup_buffer[ui - 1], y);
      }
    }
        
  } // end - move_snake()
  
  
  inline void update_followup(const std::vector<entity>& snake, const std::vector<entity>& followup_buffer){
    usize = snake.size();
    for (ui = 0; ui < usize; ui++) {
      ecs_access(comp::position, followup_buffer[ui], x) = ecs_access(comp::position, snake[ui], x);
      ecs_access(comp::position, followup_buffer[ui], y) = ecs_access(comp::position, snake[ui], y);
    }
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
  
  
  constexpr std::uint32_t width_multiplier = 35;
  constexpr std::uint32_t height_multiplier = 25;
  constexpr std::uint32_t window_height = mygame::cell_width * height_multiplier;
  constexpr std::uint32_t window_width = mygame::cell_height * width_multiplier;

  const std::string game_window_title = "Snake Game in ECS github@TerminalBoy";


  //sf::RectangleShape snake_body_shape; // no need for this, we are good with vertex arrays
  //sf::CircleShape snake_food_shape;

  sf::RenderWindow game_window(sf::VideoMode(window_width, window_height), game_window_title);

  entity speed_controller = myecs::create_entity();
  entity snake_food = myecs::create_entity();
  std::vector<entity> snake; // we will allot later
  std::vector<entity> followup_buffer;
  
  mygame::make_snake(snake, followup_buffer, 10); // entities of the bodies are created
  mygame::init_snake(snake, mygame::cell_width, mygame::cell_height);
  mygame::set_snake_direction(snake[0], mygame::direction_right);
  
  mygame::update_followup(snake, followup_buffer);
  
  mygame::update_snake_vertices(snake);
  
  game_window.setFramerateLimit(60);
  //ecs_access(comp::physics, snake[0], direction) = mygame::direction_right;
  
  ecs_access(comp::physics, snake[0], speed) = 10;

  myecs::add_comp_to<comp::speed_handler>(speed_controller);
  ecs_access(comp::speed_handler, speed_controller, move_interval) = 1.f / ecs_access(comp::physics, snake[0], speed);
  ecs_access(comp::speed_handler, speed_controller, time_accumulator) = 0.f;
  ecs_access(comp::speed_handler, speed_controller, dt) = 0.f;
  
  sf::Clock clock;

  // game loop
  while (game_window.isOpen()) {
    ecs_access(comp::speed_handler, speed_controller, dt) = clock.restart().asSeconds();
    ecs_access(comp::speed_handler, speed_controller, time_accumulator) += ecs_access(comp::speed_handler, speed_controller, dt);

    sf::Event event;
    while (game_window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) game_window.close();
    }
    
    ecs_access(comp::speed_handler, speed_controller, move_interval) = 1.f / ecs_access(comp::physics, snake[0], speed);
    mygame::take_movement_input(snake[0]);
    while (ecs_access(comp::speed_handler, speed_controller, time_accumulator) >= ecs_access(comp::speed_handler, speed_controller, move_interval)) {
      mygame::move_snake(snake, followup_buffer);
      mygame::warp_snake(snake[0], window_width, window_height);
      ecs_access(comp::speed_handler, speed_controller, time_accumulator) -= ecs_access(comp::speed_handler, speed_controller, move_interval);
    }
    mygame::update_snake_vertices(snake);

    mygame::update_followup(snake, followup_buffer);
    game_window.clear(sf::Color::Black);
    game_window.draw(mygame::renderables::snake);
    game_window.display();
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
