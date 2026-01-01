#include "loaders/font_loader.h"


#include <cstdint>
#include <iostream>
#include <fstream>

#include <stb_image.h>

#include "utils/glyph.h"


FontLoader::result_type FontLoader::operator()(const std::string& fontName)
{
  Font font;

  std::string font_metrics_path = std::string("assets/fonts/") + fontName + std::string("/metrics.json");

  std::fstream f(font_metrics_path);
  if (!f.is_open())
  {
    std::cerr << "Failed to open '" << font_metrics_path << "'\n";
    return nullptr;
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
      auto plane_bounds = glyphData["planeBounds"];
      g.planeBounds = glm::vec4(plane_bounds["left"], plane_bounds["bottom"], plane_bounds["right"], plane_bounds["top"]);
      
      auto atlas_bounds = glyphData["atlasBounds"];
      g.atlasBounds = glm::vec4(
        (float)atlas_bounds["left"] / atlasWidth, 
        (float)atlas_bounds["bottom"] / atlasHeight, 
        (float)atlas_bounds["right"] / atlasWidth, 
        (float)atlas_bounds["top"] / atlasHeight
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
  std::string font_atlas_path = std::string("assets/fonts/") + fontName + std::string("/atlas.png");
  const char* font_atlas_path_c = font_atlas_path.c_str();

  stbi_set_flip_vertically_on_load(true);
  unsigned char* pixels = stbi_load(font_atlas_path_c, &width, &height, &channels, 3); // on veut du RGB
  if (!pixels)
  {
    std::cerr << "Failed to load '" << font_atlas_path_c << "': " << stbi_failure_reason() << "\n";
    return nullptr;
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
    return nullptr;
  }

  return std::make_shared<Font>(font);
}