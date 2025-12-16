#ifndef VOXL_FONT_H
#define VOXL_FONT_H


#include <vector>

#include <glm/glm.hpp>

#include "utils/glyph.h"


static constexpr int MAX_TEXT_CHARACTERS = 512;
static constexpr unsigned int FONT_BINDING = 0; 
static constexpr unsigned int FONT_POSITION_LOC = 0; 
static constexpr unsigned int FONT_UV_LOC = 1;


struct Font
{
  unsigned int texID;
  std::vector<Glyph> metrics;
  glm::vec4 color{1.0};
};


#endif // !VOXL_FONT_H