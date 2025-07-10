//#include <iostream> // only  for debugging
#include <cstdlib>  
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

// Forward Declarations
class snake;

class snake_part;

// Class Declarations

class game_window{
  public:
 
  int width = 0; // Width and Height of game screen 
  int height = 0;
  int max_x = 0; // (Usually width - 1)
  int max_y = 0; // (Usually height - 1)
  sf::Font font;
  
  std::string title = "";
  
  game_window(){}
  
  game_window(std::string f_title, int f_width, int f_height){
    width = f_width;
    height = f_height;
    title = f_title;
  }
  
    
};

class snake { // Will hold Snake information  
  public:
  
  float speed = 5.0f;
  //float move_interval= 1.0f / speed;
  float move_interval;
  float move_timer = 0.0f;
  
  enum class direction {up, down, left, right, stop};
  
  direction dir;
 
  std::vector<snake_part> part;
  
  
  snake(){}
  
  void draw_snake(sf::RenderWindow& f_window);
  
  inline void stop() {
    dir = snake::direction::stop;
  }
  
  void init();
  
  void update_speed(float f_speed){ //updates speed live
    speed = f_speed;
    move_interval= 1.0f / speed;
  }
  
  void plus_size(int f_size){
    for (int i = 0; i < f_size; i++){
      part.emplace_back();
    }
  }
  
  void minus_size(int f_size){
    if (f_size > part.size()) f_size = part.size(); // bounds checking
    for (int i = 0; i < f_size; i++){
      part.pop_back();
    }
  }
  
  void set_size(const int& f_size){ // is slow and has overheads
    if (f_size > part.size())
      plus_size(f_size - part.size());
      
    else if (f_size < part.size())
      minus_size(part.size() - f_size);
  }
  
  inline int get_size(){
    return part.size();
  }
  
  ~snake();
  
  protected:

};

class snake_part{
  public:
  //int x = 0; // Coordinates of current blocks
  //int y = 0;
  
  
  sf::Vector2f position;
  sf::Vector2f followup;  //Coordinates of previous blocks
  
  
  static sf::RectangleShape shape;
  
};

class snake_food{
  public:
  
  float radius;
  sf::Color color;

  sf::Vector2f position;
  sf::CircleShape shape;
  
  bool eaten = false;
  
  snake_food(){
    shape.setRadius(10.f);
    color = sf::Color::Red;
  }
  
  void put_food(snake& f_snake, sf::RenderWindow& f_window){

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
      
      // make sure food dosent come on snake body
      for (int i = 0; i < f_snake.get_size(); i++){
        // make the logic for not adding food on snake, snake body/part
        if (position == f_snake.part[i].position) {
          exit_loop = false;
          break;
        } 
        exit_loop = true;
      }
    } while (exit_loop == false);
    
    eaten = false;
      
    shape.setPosition(position);
    shape.setFillColor(color);
    f_window.draw(shape);
  }
};

// Out of Class, Member function declaritions - To resolve Circular Dependencies

void snake::draw_snake(sf::RenderWindow& f_window){
  sf::Color temp = snake_part::shape.getFillColor();
  
  snake_part::shape.setPosition(part[0].position);
  snake_part::shape.setFillColor(sf::Color::Black);
  f_window.draw(snake_part::shape);
  
  snake_part::shape.setFillColor(temp);
  
  for (int i = 1; i < get_size(); i++){
    snake_part::shape.setPosition(part[i].position);
    f_window.draw(snake_part::shape);
  }
}

snake::~snake(){
  
}

void snake::init(){
  
  update_speed(speed);
  
  set_size(5);
  
  for (int i = 0; i < get_size(); i++){
    part[i].position.x = (20 * (get_size() - i));
    part[i].position.y = 0.f;
  }
}
// ============================


// Global Declarations
sf::RectangleShape snake_part::shape;
//=============================


int main(){
  srand(time(0));
  
  game_window g_window;
  g_window.width = 700;
  g_window.height = 500;
  g_window.max_x = g_window.width - 1;
  g_window.max_y = g_window.height - 1;
  g_window.title = "Snake Game - @skshazkamil";
  
  game_window game_over;
  game_over.width = 200;
  game_over.height = 100;
  game_over.max_x = game_over.width - 1;
  game_over.max_y = game_over.height - 1;
  game_over.title = "Game Over";
  
  snake snk;
  snk.update_speed(5);
  snk.set_size(5);
    
  
  snake_part::shape = sf::RectangleShape(sf::Vector2f(20.f, 20.f));  
  snake_part::shape.setFillColor(sf::Color(150, 150, 150));
  snake_part::shape.setOutlineColor(sf::Color::White);  
  snake_part::shape.setOutlineThickness(1.f); 
  
  snake_food food;
  //food.position.x = 400;
  //food.position.y = 400;
  food.color = sf::Color::Red;
  
  food.eaten = true; // to first initialize the food
  
  snk.stop();

  snk.init(); //initialize snake

  sf::RenderWindow window(sf::VideoMode(g_window.width, g_window.height),g_window.title);
  
  sf::Font font;
  
  sf::Clock clock;
  
  sf::Event window_event;
  
  if (!font.loadFromFile("Figtree-VariableFont_wght.ttf")) return -1;

  while(window.isOpen()){
    
    
        
    while (window.pollEvent(window_event)){
      if (window_event.type == sf::Event::Closed) window.close();
    }
    
    sf::Time delta_time = clock.restart();
    snk.move_timer += delta_time.asSeconds();
    
    if (window_event.type == sf::Event::KeyPressed){
      if (window_event.key.code == sf::Keyboard::Down && snk.dir != snake::direction::up){
        snk.dir = snake::direction::down;
      } else if (window_event.key.code == sf::Keyboard::Up && snk.dir != snake::direction::down){
        snk.dir = snake::direction::up;
      } else if (window_event.key.code == sf::Keyboard::Left && snk.dir != snake::direction::right){
        snk.dir = snake::direction::left;
      } else if (window_event.key.code == sf::Keyboard::Right && snk.dir != snake::direction::left){
        snk.dir = snake::direction::right;
      }  
    }
    
    
    if (snk.move_timer >= snk.move_interval){ //Frames as per snake speed
      
      snk.move_timer -= snk.move_interval;
      
      // Debug =============
      /*
      if (window_event.type == sf::Event::KeyPressed){
        
        if (window_event.key.code == sf::Keyboard::LShift 
                                || 
            window_event.key.code == sf::Keyboard::RShift){
          snk.size += 1;
        }
        
        if (window_event.key.code == 37) { //LCtrl
          snk.update_speed(snk.speed + 1);
        }
        
      } */
      
      // to detect if food is eaten or not 
      if (snk.part[0].position == food.shape.getPosition()){
        food.eaten = true;
        snk.update_speed(snk.speed + 0.5f);
        
        //if (snk.size > snk.get_max_size() - 2) snk.transform(snk.get_max_size() + 100);
        snk.plus_size(1);
        // Debug //cout<<endl<<"Food Eated"<<endl;
      }
      
      // Update Followup
      for (int i = 0; i < snk.get_size(); i++){
        snk.part[i].followup = snk.part[i].position;
      }    
      //==================

      // Snake Movement Logic :
      if (snk.dir == snake::direction::down){ //down
        snk.part[0].position.y += 20.f;
        
        if (snk.part[0].position.y >= g_window.max_y) //Teleportation when out of screen
        {snk.part[0].position.y = 0;}
        
        for (int i = 1; i <snk.get_size(); i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
      
      else if (snk.dir == snake::direction::up){ // up
        snk.part[0].position.y -= 20.f;
        
        if (snk.part[0].position.y <= -20)
        {snk.part[0].position.y = g_window.max_y - 19;} 
        
        for (int i = 1; i <snk.get_size(); i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
      
      else if (snk.dir == snake::direction::left){ // left
        snk.part[0].position.x -= 20.f;
        
        if (snk.part[0].position.x <= -20)
        {snk.part[0].position.x = g_window.max_x - 19;}
                
        for (int i = 1; i <snk.get_size(); i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
      
      else if (snk.dir == snake::direction::right){ // right
        snk.part[0].position.x += 20.f;
        
        if (snk.part[0].position.x >= g_window.max_x) 
        {snk.part[0].position.x = 0;}
        
        for (int i = 1; i <snk.get_size(); i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
    }
    
    
    
    
    // to detect self collision
    for (int i = 1; i < snk.get_size(); i++){ // Check Self Collision
      if (snk.part[0].position == snk.part[i].position){
        
        //cout<<endl <<"GAME OVER" <<endl;
        snk.stop();
        
        sf::RenderWindow game_over_window (sf::VideoMode(game_over.width, game_over.height), game_over.title);
        
        sf::Event game_over_event;
        
        //window.clear(sf::Color::White);
        
        sf::Text text;
        text.setFont(font);
        text.setString("Game Over !! \n Press R to Restart \n Press X to Exit");
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        text.setPosition(10,10);
        game_over_window.draw(text);
        game_over_window.display();
        
        while(game_over_window.isOpen()){
          while (game_over_window.pollEvent(game_over_event)){
          
            if (game_over_event.type == sf::Event::Closed) {
              game_over_window.close();
              window.close();
            }
          
          
            if (game_over_event.type == sf::Event::KeyPressed){
              if (game_over_event.key.code == sf::Keyboard::X){
                game_over_window.close();
                window.close();
              }
              if (game_over_event.key.code == sf::Keyboard::R){
                snk.set_size(5);
                snk.update_speed(5);
                snk.init();
                i = snk.get_size() - 1; // to end the for loop
                game_over_window.close();
              }
            }
            
            game_over_window.clear(sf::Color::Black);
            game_over_window.draw(text);
            game_over_window.display();
            
          } //while (game_over_window.pollEvent(game_over_event))
        } //while(game_over_window.isOpen())
      } //if (snk.part[0].position == snk.part[i].position)
    
    }// for loop ends
    
    //Main window =======
    window.clear(sf::Color::White);
    food.put_food(snk, window);
    snk.draw_snake(window);
    window.display();
    
  } // Main loop ends
  return 0;
}
