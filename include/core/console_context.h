#ifndef VOXL_CONSOLE_CONTEXT_H
#define VOXL_CONSOLE_CONTEXT_H


#include <unordered_map>
#include <string>


struct ConsoleContext
{
  std::unordered_map<std::string, std::string> historyBuffer;
};


#endif // !VOXL_CONSOLE_CONTEXT_H