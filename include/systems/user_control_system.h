#ifndef VOXL_USER_CONTROL_SYSTEM_H
#define VOXL_USER_CONTROL_SYSTEM_H


#include <SDL3/SDL_keycode.h>
#include <entt/entt.hpp>

#include "core/console_context.h"
#include "core/engine_context.h"
#include "events/game_state_change_event.h"
#include "platform/input_handler.h"
#include "utils/game_state.h"


struct UserControlSystem
{
  void Update(entt::registry& registry)
  {
    auto& engine_context = registry.ctx().get<EngineContext>();
    auto& console_context = registry.ctx().get<ConsoleContext>();
    auto& input_handler = registry.ctx().get<InputHandler>();
    auto& dispatcher = registry.ctx().get<entt::dispatcher>();

    if (engine_context.currentState == GameState::IN_GAME)
    {
      if (input_handler.IsKeyPressed(SDLK_SEMICOLON) || input_handler.IsKeyPressed(SDLK_SLASH)) 
        dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::CONSOLE});

      if (input_handler.IsKeyPressed(SDLK_DOLLAR)) 
      {
        console_context.isOpen = true;
        dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::CONSOLE});
      }
    }

    if (input_handler.IsKeyPressed(SDLK_ESCAPE))
    {
      if (engine_context.currentState == GameState::CONSOLE) 
      {
        console_context.isOpen = false;
        dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
      }
    }
  }
};


#endif // !VOXL_USER_CONTROL_SYSTEM_H