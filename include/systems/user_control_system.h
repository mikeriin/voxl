#ifndef VOXL_USER_CONTROL_SYSTEM_H
#define VOXL_USER_CONTROL_SYSTEM_H


#include <SDL3/SDL_keycode.h>
#include <entt/entt.hpp>

#include "core/game_context.h"
#include "events/game_state_change_event.h"
#include "platform/input_handler.h"
#include "utils/game_state.h"


struct UserControlSystem
{
  void Update(entt::registry& registry)
  {
    auto& game_ctx = registry.ctx().get<GameContext>();

    if (game_ctx.currentState == GameState::IN_GAME)
    {
      if (game_ctx.inputHandler.IsKeyPressed(SDLK_SEMICOLON) || game_ctx.inputHandler.IsKeyPressed(SDLK_SLASH)) 
        game_ctx.dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::CONSOLE});
    }
  }
};


#endif // !VOXL_USER_CONTROL_SYSTEM_H