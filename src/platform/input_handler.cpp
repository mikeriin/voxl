#include "platform/input_handler.h"


#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <cstdint>


InputHandler::InputHandler()
{
  _currKeyState.resize(SDL_SCANCODE_COUNT, 0);
  _prevKayState.resize(SDL_SCANCODE_COUNT, 0);

  _currButtonState.resize(5, 0);
  _prevButtonState.resize(5, 0);
}

void InputHandler::BeginFrame()
{
  _prevKayState = _currKeyState;
  _prevButtonState = _currButtonState;
}

void InputHandler::ProcessEvent(const SDL_Event& e)
{
  switch (e.type) 
  {
    case SDL_EVENT_KEY_DOWN:
      if (e.key.repeat) break;
      _currKeyState[e.key.scancode] = 1;
    break;

    case SDL_EVENT_KEY_UP:
      _currKeyState[e.key.scancode] = 0;
    break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      _currButtonState[e.button.button] = 1;
    break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      _currButtonState[e.button.button] = 0;
    break;
  }
}

bool InputHandler::IsKeyPressed(SDL_Keycode key) const
{
  int scancode = SDL_GetScancodeFromKey(key, nullptr);
  return _currKeyState[scancode] && !_prevKayState[scancode];
}

bool InputHandler::IsKeyHeld(SDL_Keycode key) const
{
  int scancode = SDL_GetScancodeFromKey(key, nullptr);
  return _currKeyState[scancode] && _prevKayState[scancode];
}

bool InputHandler::IsKeyReleased(SDL_Keycode key) const
{
  int scancode = SDL_GetScancodeFromKey(key, nullptr);
  return !_currKeyState[scancode] && _prevKayState[scancode];
}

bool InputHandler::IsButtonPressed(uint8_t button) const
{
  if (button > 5) return false;
  return _currButtonState[button] && !_prevButtonState[button];
}

bool InputHandler::IsButtonHeld(uint8_t button) const
{
  if (button > 5) return false;
  return _currButtonState[button] && _prevButtonState[button];
}

bool InputHandler::IsButtonReleased(uint8_t button) const
{
  if (button > 5) return false;
  return !_currButtonState[button] && _prevButtonState[button];
}