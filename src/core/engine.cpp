#include "core/engine.h"
#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include <memory>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <string>

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_video.h>
#include <entt/entt.hpp>
using namespace entt::literals;
#include <stb_image.h>

#include "core/engine_context.h"
#include "core/dev_console.h"
#include "core/command.h"
#include "core/command_manager.h"
#include "core/command_manager.h"
#include "core/resource_manager.h"
#include "core/scene.h"
#include "platform/window.h"
#include "platform/input_handler.h"
#include "graphics/renderer.h"
#include "loaders/font_loader.h"
#include "events/close_event.h"
#include "events/game_state_change_event.h"
#include "events/dev_console_message_event.h"
#include "systems/user_control_system.h"
#include "systems/timer_system.h"
#include "components/transform.h"
#include "components/rect_transform.h"
#include "components/text.h"
#include "components/ui_node.h"
#include "components/text_mesh.h"
#include "resources/font.h"
#include "utils/game_state.h"


Engine::Engine() : _isRunning(true) {
  _pRegistry = std::make_unique<entt::registry>();

  registerComponents();

  _pRegistry->ctx().emplace<ResourceManager>();
  _pRegistry->ctx().emplace<InputHandler>();
  _pRegistry->ctx().emplace<CommandManager>();
  
  auto& dispatcher = _pRegistry->ctx().emplace<entt::dispatcher>();
  auto &engine_context = _pRegistry->ctx().emplace<EngineContext>();
  dispatcher.sink<CloseEvent>().connect<&Engine::onClose>(this);
  dispatcher.sink<GameStateChangeEvent>().connect<&Engine::onGameStateChange>(this);

  // TODO être en mode editor par défaut en debug et in_game en release
  dispatcher.enqueue<GameStateChangeEvent>(GameStateChangeEvent{
    .newState = GameState::EDITOR
  });

  _pWindow = std::make_unique<Window>(_pRegistry.get());
  _pRenderer = std::make_unique<Renderer>(_pRegistry.get(), _pWindow.get());
  _pDevConsole = std::make_unique<DevConsole>(_pRegistry.get());
  _pScene = std::make_unique<Scene>(_pRegistry.get());
}

Engine::~Engine() 
{
  glDeleteTextures(1, &_font.textureHandle);
}

void Engine::Run() {
  auto& engine_context = _pRegistry->ctx().get<EngineContext>();
  auto& dispatcher = _pRegistry->ctx().emplace<entt::dispatcher>();

  if (!init()) {
    std::cerr << "[Engine] Failed to init engine\n";
    _isRunning = false;
  }


  UserControlSystem user_control_sys;
  TimerSystem timer_sys;


  auto last_frame_time = std::chrono::steady_clock::now();
  double delta_time;

  registerHelpCommand(); // maintenant on peut faire $help et afficher tous les helper !

  dispatcher.enqueue(DevConsoleMessageEvent{
    .level = DebugLevel::INFO,
    .buffer = "Engine ready"
  });

  while (_isRunning) {
    auto current_frame_time = std::chrono::steady_clock::now();
    delta_time = std::chrono::duration<double>(current_frame_time - last_frame_time).count();
    last_frame_time = current_frame_time;

    _pWindow->PollEvent();

    user_control_sys.Update(*_pRegistry);
    timer_sys.Update(*_pRegistry, delta_time);
    
    _pRenderer->BeginFrame();

    // affichage avec imgui
    // toujours après NewFrame et avant Render !
    bool is_scene_graph_open = (engine_context.currentState == GameState::EDITOR);
    if (is_scene_graph_open) _pScene->DisplayGraph(&is_scene_graph_open);
    bool is_console_open = (engine_context.currentState == GameState::CONSOLE);
    if (is_console_open) _pDevConsole->OpenDevConsole(&is_console_open);

    _pRenderer->Render();
    _pRenderer->EndFrame();

    dispatcher.update();

    // TODO Supprimer propement les entités
    if (!engine_context.entitiesToDelete.empty())
    {
      for (auto entity: engine_context.entitiesToDelete) _pRegistry->destroy(entity);
      engine_context.entitiesToDelete.clear();
    }
  }
}

bool Engine::init() {
  if (!_pWindow->Init())
    return false;
  if (!_pRenderer->Init())
    return false;

  auto& resource_manager = _pRegistry->ctx().get<ResourceManager>();
  resource_manager.Load<Font>("Roboto Mono", "roboto_mono");
  // resource_manager.Load<Font>("Google Sans Code", "google_sans_code");
  // resource_manager.Load<Font>("Roboto", "roboto");
  
  if (!_pDevConsole->Init())
    return false;
  
  registerCommands();

  return true;
}


void Engine::registerCommands()
{
  auto& command_manager = _pRegistry->ctx().get<CommandManager>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  // on enregistre la commande exit qui permettra de quitter le jeu
  // chaque commande commence par le symbole $
  // interprétation des commandes dans DevConsole
  // ça sera toujours la même boucle
  std::string helper = "$exit --> doesn't need args";
  command_manager.Register(Command{
    .name = "exit",
    .helper = helper,
    .func = [this, &dispatcher, helper](auto& args)
    {
      try 
      {
        // s'il y a des arguments alors on renvoit une erreur sinon on quitte le program
        if (!args.empty()) 
          throw std::out_of_range("[Engine] $exit doesn't accept args");
        this->_isRunning = false;
      } 
      // la dite erreur
      catch (const std::out_of_range &e) 
      {
        dispatcher.enqueue(DevConsoleMessageEvent{
          .level = DebugLevel::WARNING,
          .buffer = helper,
        }); // affiche le helper dans la console développeur
        std::cerr << e.what() << "\n"; // afficher la raison pour laquelle la commande n'a pas été exécuté, coté code
      }
    }
  });


  // activer/désactiver le fullscreen
  helper = "$fullscreen <toggle> --> 'toggle' must be 0 or 1";
  command_manager.Register(Command{
    .name = "fullscreen",
    .helper = helper,
    .func = [this, &dispatcher, helper](auto& args)
    {
      try {
        if (args.size() != 1) throw std::out_of_range("[Engine] $fullscreen needs only 1 arg");
        
        size_t last_valid_index;
        int toggle_fullscreen = std::stoi(args[0], &last_valid_index);
        if (last_valid_index != args[0].size() || (toggle_fullscreen != 0 && toggle_fullscreen != 1)) throw std::invalid_argument("[Engine] args[0] must be 1 or 0");

        SDL_SetWindowFullscreen(_pWindow->GetNativeWindow(), (bool) toggle_fullscreen);
      } 
      // la dite erreur
      catch (const std::out_of_range& e) 
      {
        dispatcher.enqueue(DevConsoleMessageEvent{
          .level = DebugLevel::WARNING,
          .buffer = helper,
        });
        std::cerr << e.what() << "\n";
      }
      catch (const std::invalid_argument& e)
      {
        dispatcher.enqueue(DevConsoleMessageEvent{
          .level = DebugLevel::WARNING,
          .buffer = helper,
        });
        std::cerr << e.what() << "\n";
      }
    }
  });
}


// appel de cette méthode juste avant la boucle pour être sur d'avoir la liste de toutes les commandes
void Engine::registerHelpCommand()
{
  auto& command_manager = _pRegistry->ctx().get<CommandManager>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  std::string helper = "$help --> doesn't need args";
  command_manager.Register(Command{
    .name = "help",
    .helper = helper,
    .func = [&command_manager, &dispatcher, helper](auto& args)
    {
      try {
        // s'il y a des arguments alors on renvoit une erreur sinon on quitte le program
        if (!args.empty()) throw std::out_of_range("[Engine] $help doesn't accept args");
        
        for (const auto& cmd: command_manager.GetCommands()) 
        {
          dispatcher.enqueue(DevConsoleMessageEvent{
            .level = DebugLevel::NONE,
            .buffer = cmd.second.helper,
          });
        }        
      } 
      // la dite erreur
      catch (const std::out_of_range &e) 
      {
        dispatcher.enqueue(DevConsoleMessageEvent{
          .level = DebugLevel::WARNING,
          .buffer = helper,
        }); // affiche le helper dans la console développeur
        std::cerr << e.what() << "\n"; // afficher la raison pour laquelle la commande n'a pas été exécuté, coté code
      }
    }
  });
}


void Engine::registerComponents()
{
  //! obligatoire pour lire registry comme un argument dans invoke, le type registry doit être connu, les pointeurs sont mieux géré par meta_any
  entt::meta_factory<entt::registry*>{}.type("registry_ptr"_hs);

  EditorComponent<Transform>::Register();
  EditorComponent<RectTransform>::Register();
  EditorComponent<Text>::Register();
  EditorComponent<UINode>::Register();
  EditorComponent<TextMesh>::Register();
}


void Engine::onClose(const CloseEvent &e) {
  std::cout << "[Engine] " << e.name << " called\n";
  _isRunning = !e.shoulClose;
}


void Engine::onGameStateChange(const GameStateChangeEvent& e)
{
  auto& engine_context = _pRegistry->ctx().get<EngineContext>();
  engine_context.lastState = engine_context.currentState;
  engine_context.currentState = e.newState;

  std::cout << "[Engine] " << e.name << " called => new state: ";
  switch (e.newState) 
  {
  case GameState::IN_GAME:
    std::cout << "IN_GAME\n";
    SDL_StopTextInput(_pWindow->GetNativeWindow());
  break;

  case GameState::EDITOR:
    std::cout << "EDITOR\n";
  break;

  case GameState::CONSOLE:
    std::cout << "CONSOLE\n";
    SDL_StartTextInput(_pWindow->GetNativeWindow());
  break;
  }
}