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
    
    bool is_left = false;
    bool is_right = true;
    bool is_up = false;
    bool is_down = false;
    
    snake_part* part = nullptr; // Initially is a null pointer, will be set later
    
    void transform(int);
    
    void draw_snake(sf::RenderWindow& f_window);
    
    ~snake();
    
  protected:
    int max_size;
};

class snake_part{
  public:
    //int x = 0; // Coordinates of current blocks
    //int y = 0;
    
    
    sf::Vector2f position;
    sf::Vector2f followup;  //Coordinates of previous blocks
    
    
    static sf::RectangleShape shape;
    
    static void deep_copy(snake_part*& copy_from, snake_part*& copy_to, int copy_till){
      for (int i = 0; i < copy_till; i++){
        
        //copy_to[i].x = copy_from[i].x;
        //copy_to[i].y = copy_from[i].y;
        
        copy_to[i].position = copy_from[i].position;
        
        copy_to[i].followup.x = copy_from[i].followup.x;
        copy_to[i].followup.y = copy_from[i].followup.y;
      
      }
      
    }
};


// Out of Class, Member function declaritions - To resolve Circular Dependencies

void game_window::draw_snake(snake*& sn){
  
}

void snake::transform(int f_max_size){ // Will resize the snake_part array
  int temp_max_size = snake::max_size; // Backup for further use of resizing
  
  max_size = f_max_size; // Override with provided values
  
  
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

void snake::draw_snake(sf::RenderWindow& f_window){
  for (int i = 0; i < size; i++){
    snake_part::shape.setPosition(part[i].position);
    f_window.draw(snake_part::shape);
  }
}

snake::~snake(){
  delete[] part;
}
// ============================


// Global Declarations

sf::RectangleShape snake_part::shape;

//=============================


int main(){
  game_window g_window;
  g_window.width = 700;
  g_window.height = 500;
  g_window.title = "Nigga Snaaakeeee";

  snake snk;
  snk.size = 20;
  snk.transform(50); // creates snake with its parts  
  snake_part::shape = sf::RectangleShape(sf::Vector2f(20.f, 20.f));
  snake_part::shape.setFillColor(sf::Color::Green);
  snk.is_right=false;


  for (int i = 0; i < snk.size; i++){
    snk.part[i].position.x = 20 * (snk.size - i);
    snk.part[i].position.y = 0.f;
  }
  
  for (int i = 1; i < snk.size; i++){
    snk.part[i].followup = snk.part[i].position;
  }
  
  
  
  

  sf::RenderWindow
    window 
    (sf::VideoMode(g_window.width, g_window.height),g_window.title)
  ;
  //window.setFramerateLimit(60);
  sf::Event window_event;
  
  sf::RectangleShape rect(sf::Vector2f(200.f, 200.f));
  
  sf::Vector2f pos;
  rect.setFillColor(sf::Color::Green);
  
  sf::Clock clock;
  
  float snake_speed = 20.0f;
  float move_interval= 1.0f / snake_speed;
  float move_timer = 0.0f;
  
  while(window.isOpen()){
    
    while (window.pollEvent(window_event)){
      if (window_event.type == sf::Event::Closed) window.close();
    }
    
    sf::Time delta_time = clock.restart();
    move_timer += delta_time.asSeconds();
    
  
    
    if (window_event.type == sf::Event::KeyPressed){
      
      if (window_event.key.code == sf::Keyboard::Down && window_event.key.code != 0){
        cout<<endl<<"Key pressed: Down"<<endl;
        snk.is_down = true;
        snk.is_up = false;
        snk.is_left = false;
        snk.is_right = false;
      }
      
      if (window_event.key.code == sf::Keyboard::Up){
        cout<<endl<<"Key pressed: Up"<<endl;
        snk.is_down = false;
        snk.is_up = true;
        snk.is_left = false;
        snk.is_right = false;
      }
      
      if (window_event.key.code == sf::Keyboard::Left){
        cout<<endl<<"Key pressed: Left"<<endl;
        snk.is_down = false;
        snk.is_up = false;
        snk.is_left = true;
        snk.is_right = false;
      }
      
      if (window_event.key.code == sf::Keyboard::Right){
        cout<<endl<<"Key pressed: Right"<<endl;
        snk.is_down = false;
        snk.is_up = false;
        snk.is_left = false;
        snk.is_right = true;
      }
    }
    
    if (move_timer >= move_interval){
      cout<<endl<<move_timer<<endl;
      move_timer -= move_interval;
      
      if (snk.is_down){
        snk.part[0].position.y += 20.f;
        for (int i = 1; i <snk.size; i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
      else if (snk.is_up){
        snk.part[0].position.y -= 20.f;
        for (int i = 1; i <snk.size; i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
      else if (snk.is_left){
        snk.part[0].position.x -= 20.f;
        for (int i = 1; i <snk.size; i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
      else if (snk.is_right){
        snk.part[0].position.x += 20.f;
        for (int i = 1; i <snk.size; i++){
          snk.part[i].position = snk.part[i - 1].followup;
        }
      }
    }
    // Update Followup
    for (int i = 0; i <snk.size; i++){
      snk.part[i].followup = snk.part[i].position;
    }
    //==================
    cout<<"[0].x = "<<snk.part[0].position.x<<"  |  "<<"[1].x = "<<snk.part[1].position.x<<endl;
    cout<<"[0].y = "<<snk.part[0].position.y<<"  |  "<<"[1].y = "<<snk.part[1].position.y<<endl;
    //rect.setPosition(pos);
    window.clear();
    snk.draw_snake(window);
    //window.draw(rect);
    window.display();
  
  }
  return 0;
}
