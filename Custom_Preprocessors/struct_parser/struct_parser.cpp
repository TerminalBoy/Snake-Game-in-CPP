#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <unordered_map>
// a very temporary code, ill refactor later but i need this for my ecs
// so i am making it somehow work, no optimizations, only shortcuts for now
// i am on a hurry for now

//this program will scan a cpp file and look for ##-START_PARSE-## to start parsing and ##-STOP_PARSE-## to stop parsing
// this program will look at component structs  and generate all its element's emplace_back() 
// like <struct_name>::element_name.emplace_back()

void remove_char(std::string& str, char ch) {
  std::string res = "";
  for (int i = 0; i < str.size(); i++) {
    if (str[i] != ch) res = res + str[i];
  }
  str = res;
}

std::string strip_struct_name(std::string str){
  std::size_t pos = str.find("struct");
  str.erase(pos, pos + 6);
  
  pos = str.find("//");
  if (pos != std::string::npos) str.erase(pos, str.size() - 1);
  remove_char(str, ' ');
  remove_char(str, '{');

  return str;
}

int main() {
  const std::string START_PARSE = "##-START_PARSE-##";
  const std::string STOP_PARSE = "##-STOP_PARSE-##";
  
  enum class state { on, off };
  state parsing_state = state::off;

  std::string current_line = "";

  std::ifstream target_file("test.cpp");

  std::vector<std::string> struct_token; //elements and structs
  std::vector<std::string> element_token;
  std::unordered_map<std::size_t, std::size_t> token_link;

  std::size_t struct_counter = 0;
  std::size_t element_counter = 0;

  while (std::getline(target_file, current_line)) {
     
    if (parsing_state == state::on) {
      if (current_line.find("struct") != std::string::npos) {
        struct_token.push_back(strip_struct_name(current_line));
        struct_counter++;
      }


    }
    
    
    
    if (current_line.find(START_PARSE)) parsing_state = state::on;
    if (current_line.find(STOP_PARSE)) parsing_state = state::off;
   
  }
  
  return 0;
}