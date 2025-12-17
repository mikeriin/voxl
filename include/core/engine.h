#ifndef VOXL_ENGINE_H
#define VOXL_ENGINE_H


#include <memory>

#include <entt/fwd.hpp>

#include "resources/font.h"


class Window;
class Renderer;

struct GameContext;
struct CloseEvent;
struct ResizeEvent;


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

  Font _font;

  bool init();

  void onClose(const CloseEvent& e);
};


#endif // !VOXL_ENGINE_H