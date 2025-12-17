#ifndef VOXL_TEXT_H
#define VOXL_TEXT_H


#include <string>

#include <glm/fwd.hpp>

#include "resources/font.h"


struct Text
{
  std::string text;
  Font* pFont;
  float fontSize;
  glm::vec3 position;
  glm::vec4 color;
};


#endif // !VOXL_TEXT_H