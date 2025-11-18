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
  str.erase(pos, pos + 5);
  
  pos = str.find("//");
  if (pos != std::string::npos) str.erase(pos, str.size() - 1);
  remove_char(str, ' ');
  remove_char(str, '{');

  return str;
}

std::string strip_element_name(std::string str) {
  std::string res = "";
  std::size_t endpos = str.find(";");

  if (endpos == std::string::npos) return str;

  remove_char(str, ';');

  for (int i = str.size() - 1; i >= 0; i--) {
    if (str[i] == ' ') break;

    res = std::string(1, str[i]) + res;
  }

  return res;

}

std::vector<std::string> make_emplace_backs(std::vector<std::string> struct_token, std::vector<std::string> element_token, std::unordered_map<std::size_t, std::size_t> link) {
  std::vector<std::string> res;

  //res.push_back(struct_token[i] + token_link)
  for (int i = 0; i < element_token.size(); i++) {
    res.push_back(struct_token[link[i]] + "::" + element_token[i] + ".emplace_back();");
  }
  return res;
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
  std::unordered_map<std::size_t, std::size_t> token_link; // liked like <element, struct>

  std::size_t struct_counter = 0;
  std::size_t element_counter = 0;

  while (std::getline(target_file, current_line)) {

    if (parsing_state == state::on) {
      if (current_line.find("struct") != std::string::npos) {
        struct_token.push_back(strip_struct_name(current_line));
        struct_counter++;
      }
      else if (current_line.find(";") != std::string::npos && current_line.find("}") == std::string::npos) {
        element_token.push_back(strip_element_name(current_line));
        element_counter++;
        token_link[element_counter - 1] = struct_counter - 1;

      }

    }
    
    
    
    if (current_line.find(START_PARSE) != std::string::npos && parsing_state == state::off) parsing_state = state::on;
    if (current_line.find(STOP_PARSE) != std::string::npos && parsing_state == state::on) parsing_state = state::off;
   
  }
  
  std::vector<std::string> res = make_emplace_backs(struct_token, element_token, token_link);

  for (int i = 0; i < res.size(); i++) {
    std::cout << res[i] << '\n';
  }

  return 0;
}