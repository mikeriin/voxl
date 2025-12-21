#include "platform/window.h"


#include <SDL3/SDL_keycode.h>
#include <entt/signal/fwd.hpp>
#include <iostream>
#include <memory>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_error.h>

#include "core/engine_context.h"
#include "events/close_event.h"
#include "events/resize_event.h"
#include "platform/input_handler.h"


Window::Window(entt::registry* registry)
  : _pRegistry(registry)
{}


Window::~Window()
{
  SDL_Quit();
}


bool Window::Init()
{
  auto& engine_context = _pRegistry->ctx().get<EngineContext>();

  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    std::cerr << "[Window] Failed to init SDL3: " << SDL_GetError() << "\n";
    return false;
  }

  const char* WINDOW_NAME = "widnow";
  const int WINDOW_WIDTH = 1280;
  const int WINDOW_HEIGHT = 720;
  const unsigned int WINDOW_FLAGS = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

  _pNativeWindow = std::shared_ptr<SDL_Window>(
    SDL_CreateWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS),
    SDL_DestroyWindow
  );
  if (!_pNativeWindow)
  {
    std::cout << "[Window] Failed to create SDL_Window: " << SDL_GetError() << "\n";
    return false;
  }

  resize(WINDOW_WIDTH, WINDOW_HEIGHT);
  auto& screenInfo = engine_context.screenInfo;
  screenInfo.isFocused = false;
  screenInfo.isFullScreen = false;
  screenInfo.isMinimized = false;
  screenInfo.cursorMode = CursorMode::NORMAL;

  return true;
}

void Window::PollEvent()
{
  auto& engine_context = _pRegistry->ctx().get<EngineContext>();
  auto& inputH = _pRegistry->ctx().get<InputHandler>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  inputH.BeginFrame();

  SDL_Event e;
  while (SDL_PollEvent(&e)) 
  {
    inputH.ProcessEvent(e);

    switch (e.type)
    {
      case SDL_EVENT_QUIT:
        dispatcher.enqueue<CloseEvent>({.shoulClose = true});
      break;
        
      case SDL_EVENT_WINDOW_RESIZED:
      {
        int width = e.window.data1;
        int height = e.window.data2;
        resize(width, height);
        dispatcher.enqueue<ResizeEvent>({.width = width, .height = height});
      break;
      }

      case SDL_EVENT_WINDOW_FOCUS_LOST:
        engine_context.screenInfo.isFocused = false;
      break;

      case SDL_EVENT_WINDOW_FOCUS_GAINED:
        engine_context.screenInfo.isFocused = true;
      break;

      case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        engine_context.screenInfo.isFullScreen = false;
      break;

      case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        engine_context.screenInfo.isFullScreen = true;
      break;

      case SDL_EVENT_WINDOW_MINIMIZED:
        engine_context.screenInfo.isMinimized = true;
      break;

      case SDL_EVENT_WINDOW_RESTORED:
        engine_context.screenInfo.isMinimized = false;
      break;
    }
  }

  if (inputH.IsKeyPressed(SDLK_F11))
  {
    SDL_SetWindowFullscreen(_pNativeWindow.get(), !engine_context.screenInfo.isFullScreen);
  }
}


void Window::SwapBuffers()
{
  SDL_GL_SwapWindow(GetNativeWindow());
}


SDL_Window* Window::GetNativeWindow()
{
  return _pNativeWindow.get();
}


void Window::resize(int width, int height)
{
  auto& screenInfo = _pRegistry->ctx().get<EngineContext>().screenInfo;
  screenInfo.width = width;
  screenInfo.height = height;
  screenInfo.aspectRatio = (height > 0) ? (float)width / (float)height : 1.0f;
  screenInfo.halfWidth = (float)width * 0.5f;
  screenInfo.halfHeight = (float)height * 0.5f;
}