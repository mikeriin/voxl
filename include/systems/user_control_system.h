#ifndef VOXL_USER_CONTROL_SYSTEM_H
#define VOXL_USER_CONTROL_SYSTEM_H


#include <SDL3/SDL_keycode.h>
#include <entt/entt.hpp>

#include "core/engine_context.h"
#include "events/game_state_change_event.h"
#include "platform/input_handler.h"
#include "utils/game_state.h"


struct UserControlSystem
{
  void Update(entt::registry& registry)
  {
    auto& engine_context = registry.ctx().get<EngineContext>();
    auto& input_handler = registry.ctx().get<InputHandler>();
    auto& dispatcher = registry.ctx().get<entt::dispatcher>();

    if (engine_context.currentState == GameState::IN_GAME)
    {
      if (input_handler.IsKeyHeld(SDLK_LALT) && input_handler.IsKeyPressed(SDLK_H))
      {
        dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::EDITOR});
      }
    }

    if (input_handler.IsKeyPressed(SDLK_DOLLAR) && !(engine_context.currentState == GameState::CONSOLE)) 
    {
      dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::CONSOLE});
    }

    if (input_handler.IsKeyPressed(SDLK_ESCAPE))
    {
      if (engine_context.currentState == GameState::CONSOLE) 
      {
        if (engine_context.lastState == GameState::EDITOR) dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::EDITOR});
        else dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
      }
      if (engine_context.currentState == GameState::EDITOR) dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
    }
  }
};


#endif // !VOXL_USER_CONTROL_SYSTEM_H