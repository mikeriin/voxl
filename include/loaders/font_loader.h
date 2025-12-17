#ifndef VOXL_FONT_LOADER_H
#define VOXL_FONT_LOADER_H


#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h>

#include "resources/font.h"
#include "utils/glyph.h"


inline Font LoadFont(const std::string& fontName)
{
  Font font;

  std::string fontMetricsPath = std::string("assets/fonts/") + fontName + std::string("/metrics.json");

  std::fstream f(fontMetricsPath);
  if (!f.is_open())
  {
    std::cerr << "Failed to open '" << fontMetricsPath << "'\n";
    return Font{};
  }
  nlohmann::json j = nlohmann::json::parse(f);

  font.pixelRange = (float)j["atlas"].value("distanceRange", 4.0);
  float atlasWidth = (float)j["atlas"]["width"];
  float atlasHeight = (float)j["atlas"]["height"];

  for (const auto& glyphData: j["glyphs"])
  {
    uint32_t unicode = glyphData["unicode"];

    Glyph g;
    g.advance = glyphData["advance"];

    if (glyphData.contains("planeBounds"))
    {
      auto planeBounds = glyphData["planeBounds"];
      g.planeBounds = glm::vec4(planeBounds["left"], planeBounds["bottom"], planeBounds["right"], planeBounds["top"]);
      
      auto atlasBounds = glyphData["atlasBounds"];
      g.atlasBounds = glm::vec4(
        (float)atlasBounds["left"] / atlasWidth, 
        (float)atlasBounds["bottom"] / atlasHeight, 
        (float)atlasBounds["right"] / atlasWidth, 
        (float)atlasBounds["top"] / atlasHeight
      );
    }
    else 
    {
      g.planeBounds = glm::vec4(0.0f);
      g.atlasBounds = glm::vec4(0.0f);
    }

    font.glyphs.insert_or_assign(unicode, g);
  }

  int width;
  int height;
  int channels;
  std::string fontAtlasPath = std::string("assets/fonts/") + fontName + std::string("/atlas.png");
  const char* fontAtlasPathC = fontAtlasPath.c_str();

  stbi_set_flip_vertically_on_load(true);
  unsigned char* pixels = stbi_load(fontAtlasPathC, &width, &height, &channels, 3); // on veut du RGB
  if (!pixels)
  {
    std::cerr << "Failed to load '" << fontAtlasPathC << "': " << stbi_failure_reason() << "\n";
    return Font{};
  }

  glCreateTextures(GL_TEXTURE_2D, 1, &font.textureHandle);
  glTextureStorage2D(font.textureHandle, 1, GL_RGB8, width, height); // pas de mipmap
  glTextureSubImage2D(font.textureHandle, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  glTextureParameteri(font.textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(font.textureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(font.textureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(font.textureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  stbi_image_free(pixels);

  if (!font.textureHandle)
  {
    std::cerr << "Failed to create GL Texture\n";
    return Font{};
  }

  return font;
}


#endif // !VOXL_FONT_LOADER_H