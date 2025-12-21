#include "core/command_manager.h"


void CommandManager::Register(const Command& cmd)
{
  _commands.insert_or_assign(cmd.name, cmd);
}


bool CommandManager::Execute(const std::string& name, std::vector<std::string>& args) const
{
  auto it = _commands.find(name);
  if (it != _commands.end())
  {
    it->second.func(args);
    return true;
  }
  return false;
}