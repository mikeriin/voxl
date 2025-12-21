#ifndef VOXL_COMMAND_H
#define VOXL_COMMAND_H


#include <string>
#include <functional>
#include <vector>


struct Command
{
  std::string name;
  std::string helper;
  std::function<void(const std::vector<std::string>&)> func;
};


#endif // !VOXL_COMMAND_H