#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;

// Forward Declarations
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
  int temp_max_size = snake::max_size; // Backup for further use of resizing
  
  max_size = f_max_size; // Override with provided values
  size = f_size;
  
  // If part[] array is empty or uninitialized, no need of backing it up
  if (part == nullptr){ 
    part = new snake_part[max_size];
    return; //exits method
  }
  //====================================================================
  
  
  //creating temp array of same size as part
  snake_part* temp_part = new snake_part[temp_max_size];
 
  // Deep Copying object of snake_part* part[] array into temp_part[]
  
      // Technically a backup of part[] array
  snake_part::deep_copy(part, temp_part, temp_max_size);
  
  // ===============================================      
  
  // After taking backup of part[] array, we will delete it
  // to itialize a new part[] array with a overridden size
  
  delete[] part;
  part = new snake_part[max_size];// Initialized with new max_size

  //==============================================================
  
  
  
  // As we have resized the part[] array.. We also need to give it
  // back its data that we took the backup of
  
  snake_part::deep_copy(temp_part, part, temp_max_size); // It will copy till max size of temp_part

  //==============================================================
  
  
  // So now our part array is completely resized and transformed... 
  // so we ned to delete the backup .. freeing memory
  
  delete[] temp_part;
  
  //===============================================================
  

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
    
    if (window_event.type == sf::Event::KeyPressed){
      
      if (window_event.key.code == sf::Keyboard::Down){
        cout<<endl<<"Key pressed: Down"<<endl;
      }
      
      if (window_event.key.code == sf::Keyboard::Up){
        cout<<endl<<"Key pressed: Up"<<endl;
      }
      
      if (window_event.key.code == sf::Keyboard::Left){
        cout<<endl<<"Key pressed: Left"<<endl;
      }
      
      if (window_event.key.code == sf::Keyboard::Right){
        cout<<endl<<"Key pressed: Right"<<endl;
      }
    }
    
    
    window.clear();
    window.draw(rect);
    window.display();
  
  }
  return 0;
}
