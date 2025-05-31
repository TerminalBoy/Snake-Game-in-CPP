#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;

class snake;
class snake_part;
class game_window{
  public:
 
    int width = 0; // Width and Height of game screen 
    int height = 0;
    int max_x = 0; // (Usually width - 1)
    int max_y = 0; // (Usually height - 1)
    
    string title = "";
    
    game_window(){}
    
    void draw_snake(snake*&);
};
  
class snake { // Will hold Snake information  
  public:

    int size;
    int max_size;
    snake_part* part = nullptr; // Initially is a null pointer, will be set later
    
    void transform(int, int);
    
};

class snake_part{
  public:
    int x; // Coordinates of current blocks
    int y;
    int followup_x;  //Coordinates of previous blocks
    int followup_y;  
    
    static void deep_copy(snake_part*& copy_from, snake_part*& copy_to, int copy_till){
      for (int i = 0; i < copy_till; i++){
        
        copy_to[i].x = copy_from[i].x;
        copy_to[i].y = copy_from[i].y;
        
        copy_to[i].followup_x = copy_from[i].followup_x;
        copy_to[i].followup_y = copy_from[i].followup_y;
      
      }
      
    }
};


// Out of Class, Member function declaritions - To resolve Circular Dependencies

void game_window::draw_snake(snake*& sn){
  
}

void snake::transform(int f_max_size, int f_size){ // Will resize the snake_part array
  int temp_max_size = max_size;
  
  
  //creating temp array of same size as part
  snake_part* temp_part = new snake_part[temp_max_size];
 
  max_size = f_max_size;
  size = f_size;
        
  delete[] part; // Delete old part array
  
  part = new snake_part[max_size]; // New part array with desired size
  

  
  
  // Deep Copying object of snake_part* part[] array
  
  snake_part::deep_copy(temp_part, part, temp_max_size);
  delete[] temp_part;
  
  // ===============================================      

}


// ============================


int main(){
  game_window g_window;
  g_window.width = 700;
  g_window.height = 500;
  g_window.title = "Nigga Snaaakeeee";
  
  sf::RenderWindow
    window 
    (sf::VideoMode(g_window.width, g_window.height),g_window.title)
  ;
  
  sf::Event window_event;
  
  sf::RectangleShape rect(sf::Vector2f(200.f, 200.f));
  rect.setFillColor(sf::Color::Green);
  
  while(window.isOpen()){
    while (window.pollEvent(window_event)){
      if (window_event.type == sf::Event::Closed) window.close();
    }
    
    window.clear();
    window.draw(rect);
    window.display();
  
  }
  return 0;
}
