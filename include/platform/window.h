#ifndef VOXL_WINDOW_H
#define VOXL_WINDOW_H


#include <memory>

#include <SDL3/SDL_video.h>
#include <entt/fwd.hpp>


struct ScreenInfo;


class Window
{
public:
  Window(entt::registry* registry);
  ~Window();

  bool Init();

  void PollEvent();
  void SwapBuffers();
  
  SDL_Window* GetNativeWindow();

private:
  entt::registry* _pRegistry;

  std::shared_ptr<SDL_Window> _pNativeWindow;

  void resize(int width, int height);
};


#endif // !VOXL_WINDOW_H