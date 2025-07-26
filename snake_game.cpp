#include <iostream> // only  for debugging
#include <cstdlib>  
#include <vector>
#include <string>
#include <array>
#include <SFML/Graphics.hpp>


// Forward Declarations
class snake;

class snake_part;

class snake_food;

// Class Declarations

class game_window{
  public:
  
  enum class display_state{on, off};
  
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
  
  enum class direction {up, down, left, right, stop};
  
  direction dir = direction::stop;
 
  std::vector<snake_part> part;
  
  
  snake() : dir(direction::stop) {}
  snake(int f_size, int f_speed);

  inline void stop();
    
  template <int N>
  static void draw_snake(snake* const (&snakes)[N], sf::RenderWindow& f_window, sf::Color f_head_color, sf::Color f_body_color);
  
  template <int N>
  static void init(snake* const (&snakes)[N]);
  
  void update_speed(float f_speed); //updates speed live
  void plus_size(int f_size);
  void minus_size(int f_size);  
  void set_size(const int& f_size); // is slow and has overheads
  
  inline int get_size() const; //read only

  void process_input(game_window& ft_window, input_style f_input_style = input_style::arrow);
  void process_movement(game_window& ft_window);
  
  template <int N>
  void process_eating(snake_food* const (&f_food)[N]);
  
  void process_gameover(game_window& main_ft_window, game_window& ft_window);

  void process_self_collision(game_window& main_ft_window, game_window& ft_window);
  
  static void process_other_collision(snake* const (&snakes)[], game_window& main_ft_window, game_window& ft_window);
  
  ~snake();

};



class snake_food{
  public:
  
  float radius;
  sf::Color color;

  sf::Vector2f position;
  sf::CircleShape shape;
  
  bool eaten = false;
  
  snake_food();
  
  //void put_food(snake& f_snake, sf::RenderWindow& f_window);
  template <int N>
  void put_food(const snake (&f_snake)[N], sf::RenderWindow& f_window);
};

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
      sf_window.create(sf::VideoMode(f_width, f_height), f_title);
}

void game_window::set_font(sf::Font& f_font){
  if (!font.loadFromFile("Figtree-VariableFont_wght.ttf")){
    return;
  }
}

snake_part::snake_part(sf::Vector2f pos){
  position = pos;
}

snake_part::snake_part(int x, int y){
  position.x = x;
  position.y = y;
}

snake::snake(int f_size, int f_speed){
  stop();
  set_size(f_size);
  update_speed(f_speed);
}



inline void snake::stop() {
  dir = snake::direction::stop;
}

template <int N>
void snake::draw_snake(snake* const (&snakes)[N], sf::RenderWindow& f_window, sf::Color f_head_color, sf::Color f_body_color){
  //sf::Color temp = snake_part::shape.getFillColor();
  for (int __i = 0; __i < N; __i++){
  
    snake_part::shape.setPosition(snakes[__i]->part[0].position);
    snake_part::shape.setFillColor(f_head_color);
    f_window.draw(snake_part::shape);
    
    snake_part::shape.setFillColor(f_body_color);
    
    for (int i = 1; i < snakes[__i]->get_size(); i++){
      snake_part::shape.setPosition(snakes[__i]->part[i].position);
      f_window.draw(snake_part::shape);
    }
  }
}

template <int N>
void snake::init(snake* const (&snakes)[N]){
  for (int __i = 0; __i < N; __i++){
    snakes[__i]->update_speed(snakes[__i]->speed);
    
    snakes[__i]->set_size(5);
    
    for (int i = 0; i < snakes[__i]->get_size(); i++){
      snakes[__i]->part[i].position.x = (20 * (snakes[__i]->get_size() - i));
      snakes[__i]->part[i].position.y = 0.f + (20 * __i);
    }
  }
}

void snake::update_speed(float f_speed){ //updates speed live
  speed = f_speed;
  move_interval= 1.0f / speed;
}

void snake::plus_size(int f_size){
  int temp = part.size() - 1;
  for (int i = 0; i < f_size; i++){
    part.emplace_back();
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

inline int snake::get_size() const{
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
      // Debug //cout<<endl<<"Food Eated"<<endl;
    }  
  }
}

void snake::process_gameover(game_window& main_ft_window, game_window& ft_window){
  

  stop();

  //sf::RenderWindow game_over_window (sf::VideoMode(ft_window.width, ft_window.height), ft_window.title);

  //window.clear(sf::Color::White);


  ft_window.text.setFont(ft_window.font);
  ft_window.text.setString("Game Over !! \n Press R to Restart \n Press X to Exit");
  ft_window.text.setCharacterSize(20);
  ft_window.text.setFillColor(sf::Color::White);
  ft_window.text.setPosition(10, 10);
  ft_window.sf_window.draw(ft_window.text);

  ft_window.sf_window.create(sf::VideoMode(ft_window.width, ft_window.height), ft_window.title);
  ft_window.sf_window.display();

  while (ft_window.sf_window.isOpen()) {
    while (ft_window.sf_window.pollEvent(ft_window.event)) {

      if (ft_window.event.type == sf::Event::Closed) {
        ft_window.sf_window.close();
        main_ft_window.sf_window.close();
      }


      if (ft_window.event.type == sf::Event::KeyPressed) {
        if (ft_window.event.key.code == sf::Keyboard::X) {
          ft_window.sf_window.close();
          main_ft_window.sf_window.close();
        }
        if (ft_window.event.key.code == sf::Keyboard::R) {
          set_size(5);
          update_speed(5);
          snake::init({ this });
          ft_window.sf_window.close();
        }
      }

      ft_window.sf_window.clear(sf::Color::Black);
      ft_window.sf_window.draw(ft_window.text);
      ft_window.sf_window.display();

    } //while (game_over_window.pollEvent(game_over_event))
  } //while(game_over_window.isOpen())
 
}

void snake::process_self_collision(game_window& main_ft_window, game_window& ft_window){

  // to detect self collision
  for (int i = 1; i < get_size(); i++){ // Check Self Collision
    if (part[0].position == part[i].position) {
      process_gameover(main_ft_window, ft_window);
      break;
    }
  
  }// for loop ends

}


void snake::process_other_collision(snake* const (&snakes)[], game_window& main_ft_window, game_window& ft_window) {
  
  for (int i = 0; i < snakes[0]->get_size(); i++) {
    if (snakes[0]->part[0].position == snakes[1]->part[i].position){
      std::cout << "Collision";
      snake::process_other_collision({ snakes[0], snakes[1] }, main_ft_window, ft_window);
    }
  }

  for (int i = 0; i < snakes[1]->get_size(); i++) {
    if (snakes[1]->part[0].position == snakes[0]->part[i].position){
      std::cout << "Collision";
      snake::process_other_collision({ snakes[0], snakes[1] }, main_ft_window, ft_window);
    }
  }

}

snake::~snake(){}

snake_food::snake_food(){
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
  
  snake snk;
  snk.update_speed(5);
  snk.set_size(5);
  
  snake snk2;
  snk2.update_speed(5);
  snk2.set_size(5);
  

  
  snake_part::shape = sf::RectangleShape(sf::Vector2f(20.f, 20.f));  
  snake_part::shape.setFillColor(sf::Color(150, 150, 150));
  snake_part::shape.setOutlineColor(sf::Color::White);  
  snake_part::shape.setOutlineThickness(1.f); 
  
  snake_food food;
  //food.position.x = 400;
  //food.position.y = 400;

  snake::init({&snk, &snk2}); //initialize snake
  snk.stop();
  snk2.stop();
  while(g_window.sf_window.isOpen()){
    
    while (g_window.sf_window.pollEvent(g_window.event)){
      if (g_window.event.type == sf::Event::Closed) g_window.sf_window.close();
    }
    
    g_window.delta_time = g_window.clock.restart();
    snk.move_timer += g_window.delta_time.asSeconds();
  
    

    
    if (snk.move_timer >= snk.move_interval){ //Frames as per snake speed
      snk.move_timer -= snk.move_interval;
    
    
      snk.process_input(g_window, snake::input_style::arrow);
      snk2.process_input(g_window, snake::input_style::wasd);
      
      snk.process_movement(g_window);
      snk2.process_movement(g_window);
      
      snk.process_eating({&food});
      snk2.process_eating({&food});
      
      snk.process_self_collision(g_window, game_over);
      snk2.process_self_collision(g_window, game_over);

      snake::process_other_collision({ &snk, &snk2 }, g_window, game_over);
    
    }
    
    //Main window =======
    g_window.sf_window.clear(sf::Color::White);
    food.put_food({snk}, g_window.sf_window);
    snake::draw_snake({&snk}, g_window.sf_window, sf::Color(100, 200, 100), sf::Color(50, 250, 50));
    snake::draw_snake({&snk2}, g_window.sf_window, sf::Color(100, 100, 200), sf::Color(50, 50, 250));
    
    g_window.sf_window.display();    
  } // Main loop ends
  return 0;
}