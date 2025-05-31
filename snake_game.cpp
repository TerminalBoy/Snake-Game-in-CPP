#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std::cout;
using namespace std::endl;
using namespace std::cin;
  
class game_window{
  public:
 
    int width = 0; // Width and Height of game screen 
    int height = 0;
    int max_x = 0; // (Usually width - 1)
    int max_y = 0; // (Usually height - 1)
    string title = "";
    game_window(){
          
    }
};
  
  


class snake { // Will hold Snake information  
  public:

    static int size;
    static const int max_size = 100;
  
    int x = 0; // Coordinates of current blocks
    int y = 0;
    int followup_x = 0;  //Coordinates of previous blocks
    int followup_y = 0;
};

int main(){
  game_window g_window;
  g_window.width = 700;
  g_window.height = 500;
  g_window.title = "Nigga Snaaakeeee aaaahhhhhhooo";
  
  sf::RenderWindow
    window 
    (sf::VideoMode(g_window.width, g_window.height),g_window.title)
  ;
  
  
  
  
  return 0;
}
