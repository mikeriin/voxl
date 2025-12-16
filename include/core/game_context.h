#ifndef VOXL_GAME_CONTEXT_H
#define VOXL_GAME_CONTEXT_H


#include <entt/entt.hpp>

#include "platform/input_handler.h"
#include "utils/screen_info.h"


struct GameContext
{
  entt::dispatcher dispatcher;
  InputHandler inputHandler;
  ScreenInfo screenInfo;
};


#endif // !VOXL_GAME_CONTEXT_H