

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
#include <array>
#include <unordered_map>

#include "Dependencies/SFML/include/SFML/Graphics.hpp" // i am sorry for this mess, but the library has hard coded the "<SFML/Graphics/**>" paths
#include "Dependencies/Custom_ECS/memory.hpp"
#include "Dependencies/Custom_ECS/components.hpp"
#include "Dependencies/Custom_ECS/generated_components_create.hpp"
#include "Dependencies/Custom_ECS/generated_components_delete.hpp"
//#include <Custom_ECS_libs/memory.hpp>// will test later




// ==============================================================================================================

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

// ==============================================================================================================



// trying ECS


// Entity
// 
// entity defined in memory.hpp in "Dependencies/Custom_ECS/memory.hpp"
//
// using entity = std::uint16_t;
//

static entity GLOBAL_ENTITY_COUNTER = 0;

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
    inline static myecs::unordered_map<std::size_t> entity_component_link;
  };
 

  // ecs functions

  entity create_entity() { // first step
    GLOBAL_ENTITY_COUNTER++;
    return GLOBAL_ENTITY_COUNTER - 1;
  }

  template <typename component> // helper only, no need to call explictly
  inline void entity_component_linker(entity base_entity, std::size_t corresponding_comp_index,
    myecs::unordered_map<std::size_t>& eclink = myecs::storage<component>::entity_component_link) {
    eclink [base_entity] = corresponding_comp_index;
  }

 
  template <typename component>
  void add_comp_to(entity& id) { // third step
    myecs::create_component(myecs::storage<component>::pointer); // from "generated_components_create.hpp"
    myecs::storage<component>::size++;
    entity_component_linker<component>(id, myecs::storage<component>::size - 1); 
  }

  template <typename component>
  void remove_comp_from(entity& id, myecs::unordered_map<entity, std::size_t>& ECbridge = myecs::storage<component>::ECbridge) {
   
  }

  template <typename component>
  inline const std::size_t& comp_index_of(entity& id) {
    return myecs::storage<component>::entity_component_link[id];
  }

}



int main() {
  entity rectangle = myecs::create_entity();
  entity point = myecs::create_entity();

  //myecs::new_init_component<comp::rectangle>();
  //myecs::new_init_component<comp::position>();
  
  myecs::add_comp_to<comp::rectangle>(rectangle);
  myecs::add_comp_to<comp::position>(rectangle);
  myecs::add_comp_to<comp::position>(point);
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
  std::cout << "point component id at entity id (point): " << myecs::comp_index_of<comp::rectangle>(point) << std::endl;
  std::cout << "entity id (rectangle): " << rectangle << std::endl;
  std::cout << "entity id (point): " << point << std::endl;

  std::cout << "point,s x: " << myecs::storage<comp::position>::pointer->x[myecs::comp_index_of<comp::position>(point)] << std::endl;
  std::cout << "point,s y: " << myecs::storage<comp::position>::pointer->y[myecs::comp_index_of<comp::position>(point)] << std::endl;
  std::cout << "rectangle,s x: " << myecs::storage<comp::position>::pointer->x[myecs::comp_index_of<comp::position>(rectangle)] << std::endl;
  std::cout << "rectangle,s y: " << myecs::storage<comp::position>::pointer->y[myecs::comp_index_of<comp::position>(rectangle)] << std::endl;
  


  // reading the components very effiently
  // crazy fast for rendering systems needing the vertex arrays of all entities // very cache friendly
  for (int i = 0; i < myecs::storage<comp::position>::size; i++) {
    std::cout << "comp::position' x: " << myecs::storage<comp::position>::pointer->x[i] << std::endl;
    std::cout << "comp::position' y: " << myecs::storage<comp::position>::pointer->y[i] << std::endl;
  }

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