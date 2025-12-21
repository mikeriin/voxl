#ifndef VOXL_GAME_CONTEXT_H
#define VOXL_GAME_CONTEXT_H


#include <entt/entt.hpp>

#include "utils/screen_info.h"
#include "utils/game_state.h"

struct GameContext
{
  ScreenInfo screenInfo;
  GameState currentState;
};


#endif // !VOXL_GAME_CONTEXT_H