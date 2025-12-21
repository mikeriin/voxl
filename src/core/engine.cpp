#include "core/engine.h"
#include <SDL3/SDL_video.h>
#define STB_IMAGE_IMPLEMENTATION

#include <memory>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <string>

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>

#include "core/game_context.h"
#include "core/dev_console.h"
#include "core/command.h"
#include "core/command_manager.h"
#include "core/command_manager.h"
#include "platform/window.h"
#include "platform/input_handler.h"
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

  _pRegistry->ctx().emplace<InputHandler>();
  _pRegistry->ctx().emplace<CommandManager>();
  
  auto& dispatcher = _pRegistry->ctx().emplace<entt::dispatcher>();
  auto &game_context = _pRegistry->ctx().emplace<GameContext>();
  dispatcher.sink<CloseEvent>().connect<&Engine::onClose>(this);
  dispatcher.sink<GameStateChangeEvent>().connect<&Engine::onGameStateChange>(this);

  _pWindow = std::make_unique<Window>(_pRegistry.get());
  _pRenderer = std::make_unique<Renderer>(_pRegistry.get(), _pWindow.get());
  _pDevConsole = std::make_unique<DevConsole>(_pRegistry.get());

  dispatcher.sink<ResizeEvent>().connect<&DevConsole::OnResize>(_pDevConsole);
}

Engine::~Engine() 
{
  glDeleteTextures(1, &_font.textureHandle);
}

void Engine::Run() {
  auto& game_context = _pRegistry->ctx().get<GameContext>();
  auto& dispatcher = _pRegistry->ctx().emplace<entt::dispatcher>();

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

    dispatcher.update();
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


  _pRenderer->UpdateFont(); // TODO! créer un gestionnaire de resources, pour le moment cette fonction créée un shader pour afficher du text
  
  registerCommands();
  return true;
}


void Engine::registerCommands()
{
  auto& command_manager = _pRegistry->ctx().get<CommandManager>();

  // on enregistre la commande exit qui permettra de quitter le jeu
  // chaque commande commence par le symbole $
  // interprétation des commandes dans DevConsole
  // ça sera toujours la même boucle
  std::string helper = "?> $exit";
  command_manager.Register(Command{
    .name = "exit",
    .helper = helper,
    .func = [this, helper](auto& args)
    {
      try {
        // s'il y a des arguments alors on renvoit une erreur sinon on quitte le program
        if (!args.empty()) throw std::out_of_range("[Engine] $exit doesn't accept args");
        this->_isRunning = false;
      } 
      // la dite erreur
      catch (const std::out_of_range &e) 
      {
        _pDevConsole->UpdateHistory(helper); // affiche le helper dans la console développeur
        std::cerr << e.what() << "\n"; // afficher la raison pour laquelle la commande n'a pas été exécuté, coté code
      }
    }
  });

  // activer/désactiver le fullscreen
  helper = "?> $fullscreen <0/1>";
  command_manager.Register(Command{
    .name = "fullscreen",
    .helper = helper,
    .func = [this, helper](auto& args)
    {
      try {
        if (args.size() > 1) throw std::out_of_range("[Engine] $fullscreen accepts only 1 arg");
        
        size_t last_valid_index;
        int toggle_fullscreen = std::stoi(args[0], &last_valid_index);
        if (last_valid_index != args[0].size() || (toggle_fullscreen != 0 && toggle_fullscreen != 1)) throw std::invalid_argument("[Engine] args[0] must be 1 or 0");

        SDL_SetWindowFullscreen(_pWindow->GetNativeWindow(), (bool) toggle_fullscreen);
      } 
      // la dite erreur
      catch (const std::out_of_range& e) 
      {
        _pDevConsole->UpdateHistory(helper);
        std::cerr << e.what() << "\n";
      }
      catch (const std::invalid_argument& e)
      {
        _pDevConsole->UpdateHistory(helper);
        std::cerr << e.what() << "\n";
      }
    }
  });
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