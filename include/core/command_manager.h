#ifndef VOXL_COMMAND_MANAGER_H
#define VOXL_COMMAND_MANAGER_H


#include <unordered_map>
#include <string>
#include <vector>

#include "core/command.h"


class CommandManager
{
public:
  CommandManager() = default;
  ~CommandManager() = default;

  void Register(const Command& cmd);
  bool Execute(const std::string& name, std::vector<std::string>& args) const;

private:
  std::unordered_map<std::string, Command> _commands;
};


#endif // !VOXL_COMMAND_MANAGER_H