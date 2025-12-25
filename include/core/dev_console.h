#ifndef VOXL_DEV_CONSOLE_H
#define VOXL_DEV_CONSOLE_H


#include <string>

#include <entt/entt.hpp>
#include <vector>


static constexpr int DEV_CONSOLE_WIDTH = 640;
static constexpr int DEV_CONSOLE_HEIGHT = 360;


struct DevConsoleMessageEvent;


class DevConsole
{
public:
  DevConsole(entt::registry* registry);
  ~DevConsole();

  bool Init();

  void Update();
  void OpenDevConsole(bool* pOpen);

private:
  entt::registry* _pRegistry;

  char _inputBuffer[512];
  std::vector<std::string> _history;
  bool _scrollToBottom = false;

  void onDevConsoleMessage(const DevConsoleMessageEvent& e);

  void registerCommands();
};


#endif // !VOXL_DEV_CONSOLE_H