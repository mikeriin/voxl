#ifndef VOXL_FONT_H
#define VOXL_FONT_H


#include <cstdint>
#include <unordered_map>

#include "utils/glyph.h"


struct Font
{
  unsigned int textureHandle;
  float pixelRange; // pxrange => 4.0 pour roboto
  std::unordered_map<uint32_t, Glyph> glyphs;
};


#endif // !VOXL_FONT_H