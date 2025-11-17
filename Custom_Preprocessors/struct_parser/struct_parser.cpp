#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>

// a very temporary code, ill refactor later but i need this for my ecs
// so i am making it somehow work, no optimizations, only shortcuts for now
// i am on a hurry for now

bool if_struct_exists(std::string& str) {

  if (str.find("struct") != std::string::npos) return true;
  return false;

}

int main() {
  const std::string START_PARSE = "// ##-START_PARSE-##";
  const std::string STOP_PARSE = "// ##-STOP_PARSE-##";

  bool start = false;
  bool stop = false;
  
  std::string struct_name = "";

  std::vector<std::string> element_token;// i know its very bad, ill fix it later

  std::ifstream target_file("test.cpp");
  std::string line = "";
  
  if (!target_file.is_open()) std::cout << "Error opening file" << std::endl;

  while (std::getline(target_file, line)) {
  
    if (start) {
      if (if_struct_exists(line)) {
        std::cout << "struct exists\n";
        int struct_start = line.find("struct");
        int ix = struct_start + 6;
        struct_name = "";

        for (int i = ix; i < line.size(); i++) {
          if (line[i] != ' ' && line[i] != '{') {
            struct_name = struct_name + line[i];
          }
          else if (line[i] == '{') {
            break;
          }
        }
        std::cout << "struct name :" << struct_name << std::endl;
      }
      else if (struct_name != "") {
        std::string temp_element = "";
        bool element_start = false;

        for (int i = line.size() - 1; i >= 0; i--) {
          if (!element_start && line[i] == ';') element_start = true;

          if (element_start) {
            if (line[i] != ' ') temp_element = std::string(1, line[i]) + temp_element;
            if (line[i] == ' ') {
              element_token.push_back(struct_name + "::" + temp_element + ".emplace_back()");

              break;
            }
          }
          
        }
      }

      
    }
    
    if (line.find(START_PARSE) != std::string::npos) start = true;
    if (line.find(STOP_PARSE) != std::string::npos) break;
   
  }

  for (int i = 0; i < element_token.size(); i++) {
    std::cout << element_token[i] << std::endl;
  }
  return 0;
}