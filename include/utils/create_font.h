#ifndef VOXL_CREATE_FONT_H
#define VOXL_CREATE_FONT_H


#include "utils/glyph.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json_fwd.hpp>
#include <string>

#include <glad/glad.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "components/font.h"
#include "utils/stb_image.h"


inline Font CreateFont(const std::string& fontName)
{
  std::string fontTexPath = "assets/textures/fonts/" + fontName + std::string(".png");
  std::string fontMetaPath = "assets/fonts/" + fontName + std::string(".json");

  Font font;

  int width;
  int height;
  int channels;

  const char* fontTexPathC = fontTexPath.c_str();
  unsigned char* pixels = stbi_load(fontTexPathC, &width, &height, &channels, 3); // 3 => force le RGB
  if (!pixels)
  {
    std::cerr << "[CreateFont] Failed to load '" << fontTexPath << "' : " << stbi_failure_reason() << "\n";
    return Font{};
  }

  glCreateTextures(GL_TEXTURE_2D, 1, &font.texID);
  glTextureStorage2D(font.texID, 1, GL_RGB8, width, height); // alloc mémoire GPU
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
  glTextureSubImage2D(font.texID, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels); // upload des données

  glTextureParameteri(font.texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(font.texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(font.texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(font.texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  stbi_image_free(pixels);

  if (!font.texID)
  {
    std::cerr << "[CreateFont] Failed to create texture\n";
    return font;
  }

  font.metrics.resize(128);

  std::ifstream f(fontMetaPath);
  json meta = json::parse(f);

  float atlasWidth = meta["atlas"]["width"];
  float atlasHeight = meta["atlas"]["height"];

  for (const auto& glyph: meta["glyphs"])
  {
    int unicode = glyph["unicode"];
    if (unicode > 127) continue; // on skip les caractères spéciaux pour le moment

    Glyph& data = font.metrics[unicode];
    data.advance = glyph["advance"];

    // espace unicode(32) n'a pas de 'bound'
    if (glyph.contains("planeBounds"))
    {
      data.planeLeft    = glyph["planeBounds"]["left"];
      data.planeBottom  = glyph["planeBounds"]["bottom"];
      data.planeRight   = glyph["planeBounds"]["right"];
      data.planeTop     = glyph["planeBounds"]["top"];

      data.uvLeft   = (float)glyph["atlasBounds"]["left"]   / atlasWidth;
      data.uvBottom = (float)glyph["atlasBounds"]["bottom"] / atlasHeight;
      data.uvRight  = (float)glyph["atlasBounds"]["right"]  / atlasWidth;
      data.uvTop    = (float)glyph["atlasBounds"]["top"]    / atlasHeight;
    }
    else
    {
      data.planeLeft = data.planeBottom = data.planeRight = data.planeTop = 0.0f;
      data.uvLeft = data.uvBottom = data.uvRight = data.uvTop = 0.0f;
    }
  }

  return font;
}


#endif // ! VOXL_CREATE_FONT_H