#ifndef VOXL_ENGINE_CONTEXT_H
#define VOXL_ENGINE_CONTEXT_H


#include <vector>

#include <entt/entt.hpp>

#include "utils/screen_info.h"
#include "utils/game_state.h"

struct EngineContext
{
  ScreenInfo screenInfo;
  GameState lastState;
  GameState currentState;
  std::vector<entt::entity> entitiesToDelete;
};


#endif // !VOXL_ENGINE_CONTEXT_H