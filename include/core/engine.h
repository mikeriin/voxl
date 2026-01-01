#ifndef VOXL_ENGINE_H
#define VOXL_ENGINE_H


#include <memory>

#include <entt/fwd.hpp>

#include "resources/font.h"


class Window;
class Renderer;
class DevConsole;
class Scene;

struct GameContext;
struct CloseEvent;
struct ResizeEvent;
struct GameStateChangeEvent;


class Engine
{
public:
  Engine();
  ~Engine();

  void Run();

private:
  bool _isRunning;

  std::unique_ptr<entt::registry> _pRegistry;
  std::unique_ptr<Window> _pWindow;
  std::unique_ptr<Renderer> _pRenderer;
  std::unique_ptr<DevConsole> _pDevConsole;
  std::unique_ptr<Scene> _pScene;

  Font _font;

  bool init();
  void registerCommands();
  void registerHelpCommand();
  void registerComponents();

  void onClose(const CloseEvent& e);
  void onGameStateChange(const GameStateChangeEvent& e);
};


#endif // !VOXL_ENGINE_H