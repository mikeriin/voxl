#include "core/engine.h"
#define STB_IMAGE_IMPLEMENTATION

#include <SDL3/SDL_keycode.h>
#include <iostream>
#include <memory>

#include "core/game_context.h"
#include "events/close_event.h"
#include "graphics/renderer.h"
#include "platform/window.h"
#include "loaders/font_loader.h"
#include "components/text.h"
#include "components/text_mesh.h"
#include "utils/create_text_mesh.h"


Engine::Engine() : _isRunning(true) {
  _pRegistry = std::make_unique<entt::registry>();

  auto &gameCtx = _pRegistry->ctx().emplace<GameContext>();
  gameCtx.dispatcher.sink<CloseEvent>().connect<&Engine::onClose>(this);

  _pWindow = std::make_unique<Window>(_pRegistry.get());
  _pRenderer = std::make_unique<Renderer>(_pRegistry.get(), _pWindow.get());
}

Engine::~Engine() 
{
  glDeleteTextures(1, &_font.textureHandle);
}

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


  _font = LoadFont("roboto_mono");

  Text t{
    .text = "Hello, World!\n@#&!$*%^§+=?[]{}<>~€",
    .pFont = &_font,
    .fontSize = 50,
    .position = {100.0f, 200.0f, 0.0f},
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