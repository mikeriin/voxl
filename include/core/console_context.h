#ifndef VOXL_CONSOLE_CONTEXT_H
#define VOXL_CONSOLE_CONTEXT_H


#include <unordered_map>
#include <string>


struct ConsoleContext
{
  std::unordered_map<std::string, std::string> helperBuffer;
  bool isOpen = false;
};


#endif // !VOXL_CONSOLE_CONTEXT_H