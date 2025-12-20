#include "core/engine.h"
#define STB_IMAGE_IMPLEMENTATION

#include <memory>
#include <iostream>
#include <chrono>

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>

#include "core/game_context.h"
#include "core/dev_console.h"
#include "platform/window.h"
#include "graphics/renderer.h"
#include "loaders/font_loader.h"
#include "events/resize_event.h"
#include "events/close_event.h"
#include "events/game_state_change_event.h"
#include "components/text.h"
#include "components/text_mesh.h"
#include "utils/create_text_mesh.h"
#include "systems/user_control_system.h"
#include "systems/timer_system.h"


Engine::Engine() : _isRunning(true) {
  _pRegistry = std::make_unique<entt::registry>();

  auto &game_ctx = _pRegistry->ctx().emplace<GameContext>();
  game_ctx.dispatcher.sink<CloseEvent>().connect<&Engine::onClose>(this);
  game_ctx.dispatcher.sink<GameStateChangeEvent>().connect<&Engine::onGameStateChange>(this);

  _pWindow = std::make_unique<Window>(_pRegistry.get());
  _pRenderer = std::make_unique<Renderer>(_pRegistry.get(), _pWindow.get());
  _pDevConsole = std::make_unique<DevConsole>(_pRegistry.get());

  game_ctx.dispatcher.sink<ResizeEvent>().connect<&DevConsole::OnResize>(_pDevConsole);
}

Engine::~Engine() 
{
  glDeleteTextures(1, &_font.textureHandle);
}

void Engine::Run() {
  auto &game_ctx = _pRegistry->ctx().get<GameContext>();

  if (!init()) {
    std::cerr << "[Engine] Failed to init engine\n";
    _isRunning = false;
  }


  UserControlSystem user_control_sys;
  TimerSystem timer_sys;


  auto last_frame_time = std::chrono::steady_clock::now();
  double delta_time;

  while (_isRunning) {
    auto current_frame_time = std::chrono::steady_clock::now();
    delta_time = std::chrono::duration<double>(current_frame_time - last_frame_time).count();
    last_frame_time = current_frame_time;

    _pWindow->PollEvent();

    user_control_sys.Update(*_pRegistry);
    timer_sys.Update(*_pRegistry, delta_time);

    _pDevConsole->Update();
    
    _pRenderer->BeginFrame();
    _pRenderer->Render();
    _pRenderer->EndFrame();

    game_ctx.dispatcher.update();
  }
}

bool Engine::init() {
  if (!_pWindow->Init())
    return false;
  if (!_pRenderer->Init())
    return false;
  if (!_pDevConsole->Init())
    return false;

  _font = LoadFont("roboto");

  Text t{
    .text = "Hello, World!\n@#&!$*%^§+=?[]{}<>~€†",
    .pFont = &_font,
    .fontSize = 50,
    .position = {50.0f, 450.0f, 0.0f},
    .color = {1.0f, 1.0f, 1.0f, 1.0f}
  };
  auto mesh = CreateTextMesh(t);

  auto e = _pRegistry->create();
  _pRegistry->emplace<Text>(e, t);
  _pRegistry->emplace<TextMesh>(e, mesh);


  _pRenderer->UpdateFont();

  return true;
}

void Engine::onClose(const CloseEvent &e) {
  std::cout << "[Engine] " << e.name << " called\n";
  _isRunning = !e.shoulClose;
}

void Engine::onGameStateChange(const GameStateChangeEvent& e)
{
  std::cout << "[Engine] " << e.name << " called => new state: ";

  switch (e.newState) 
  {
  case GameState::IN_GAME:
    std::cout << "IN_GAME\n";
    SDL_StopTextInput(_pWindow->GetNativeWindow());
  break;

  case GameState::CONSOLE:
    std::cout << "CONSOLE\n";
    SDL_StartTextInput(_pWindow->GetNativeWindow());
  break;
  }

  _pRegistry->ctx().get<GameContext>().currentState = e.newState;
}