#ifndef VOXL_SCREEN_INFO_H
#define VOXL_SCREEN_INFO_H


#include "utils/cursor_mode.h"


struct ScreenInfo
{
  int width;
  int height;
  float aspectRatio;
  float halfWidth;
  float halfHeight;
  bool isFocused;
  bool isFullScreen;
  bool isMinimized;
  CursorMode cursorMode;
};


#endif // !VOXL_SCREEN_INFO_H