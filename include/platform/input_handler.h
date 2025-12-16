#ifndef VOXL_INPUT_HANDLER_H
#define VOXL_INPUT_HANDLER_H


#include <cstdint>
#include <vector>

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_events.h>


class InputHandler
{
public:
  InputHandler();

  void BeginFrame();
  void ProcessEvent(const SDL_Event& e);

  bool IsKeyPressed(SDL_Keycode key) const;
  bool IsKeyHeld(SDL_Keycode key) const;
  bool IsKeyReleased(SDL_Keycode key) const;

  bool IsButtonPressed(uint8_t button) const;
  bool IsButtonHeld(uint8_t button) const;
  bool IsButtonReleased(uint8_t button) const;

private:
  std::vector<uint8_t> _currKeyState;
  std::vector<uint8_t> _prevKayState;

  std::vector<uint8_t> _currButtonState;
  std::vector<uint8_t> _prevButtonState;
};


#endif // !VOXL_INPUT_HANDLER_H