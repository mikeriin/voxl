#include "core/engine.h"
#include "components/font_mesh.h"
#define STB_IMAGE_IMPLEMENTATION

#include <SDL3/SDL_keycode.h>
#include <iostream>
#include <memory>

#include "components/font.h"
#include "core/game_context.h"
#include "events/close_event.h"
#include "graphics/renderer.h"
#include "platform/window.h"
#include "utils/create_font.h"
#include "utils/generate_font_mesh.h"


Engine::Engine() : _isRunning(true) {
  _pRegistry = std::make_unique<entt::registry>();

  auto &gameCtx = _pRegistry->ctx().emplace<GameContext>();
  gameCtx.dispatcher.sink<CloseEvent>().connect<&Engine::onClose>(this);

  _pWindow = std::make_unique<Window>(_pRegistry.get());
  _pRenderer = std::make_unique<Renderer>(_pRegistry.get(), _pWindow.get());
}

Engine::~Engine() = default;

void Engine::Run() {
  auto &gameCtx = _pRegistry->ctx().get<GameContext>();

  if (!init()) {
    std::cerr << "[Engine] Failed to init engine\n";
    _isRunning = false;
  }

  while (_isRunning) {
    _pWindow->PollEvent();
    gameCtx.dispatcher.update();

    _pRenderer->BeginFrame();
    _pRenderer->Render();
    _pRenderer->EndFrame();
  }
}

bool Engine::init() {
  if (!_pWindow->Init())
    return false;
  if (!_pRenderer->Init())
    return false;

  Font font = CreateFont("roboto");
  FontMesh mesh = GenerateFontMesh("Bonjour\n Je m'appelle Gael.", font, 100.0f, 100.0f, 10.0f);
  auto e = _pRegistry->create();
  _pRegistry->emplace<Font>(e, font);
  _pRegistry->emplace<FontMesh>(e, mesh);

  _pRenderer->UpdateFont();

  return true;
}

void Engine::onClose(const CloseEvent &e) {
  std::cout << "[Engine] " << e.name << " called\n";
  _isRunning = !e.shoulClose;
}