#ifndef VOXL_DEV_CONSOLE_H
#define VOXL_DEV_CONSOLE_H


#include <string>
#include <deque>

#include <entt/entt.hpp>
#include <vector>

#include "events/resize_event.h"


static constexpr float CONSOLE_FONT_SIZE = 12.0f;
static constexpr float BUFFER_Y_OFFSET = 15.0f;
static constexpr float HISTORY_Y_OFFSET = 40.0f;
static constexpr size_t NUM_COMMAND_HISTORY = 20;
static constexpr double HISTORY_TIMER_TIME = 10.0;

static constexpr int DEV_CONSOLE_WIDTH = 500;
static constexpr int DEV_CONSOLE_HEIGHT = 375;


class DevConsole
{
public:
  DevConsole(entt::registry* registry);
  ~DevConsole();

  bool Init();

  void Update();
  void OpenDevConsole(bool* pOpen);
  void UpdateHistory(const std::string& buffer);

  void OnResize(const ResizeEvent& e);

private:
  entt::registry* _pRegistry;

  char _inputBuffer[256];
  std::vector<std::string> _items;
  bool _scrollToBottom = false;

  std::string _buffer;
  entt::entity _historyEntity;
  std::deque<std::string> _history;
  
  std::string getHistoryAsText() const;
  void resetHistoryTimer(double time, bool isActive);
};


#endif // !VOXL_DEV_CONSOLE_H