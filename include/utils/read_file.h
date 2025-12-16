#ifndef VOXL_READ_FILE_H
#define VOXL_READ_FILE_H


#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

inline std::string ReadFile(const char *path) 
{
  std::ifstream file(path);

  if (!file.is_open()) {
    std::cerr << "Failed to open '" << path << "\n";
    return "";
  }
  
  std::stringstream buffer;
  buffer << file.rdbuf();

  return buffer.str();
}


#endif // !VOXL_READ_FILE_H