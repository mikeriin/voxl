#ifndef VOXL_FONT_LOADER_H
#define VOXL_FONT_LOADER_H


#include <string>
#include <memory>

#include <nlohmann/json.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "resources/font.h"

struct FontLoader
{
  using result_type = std::shared_ptr<Font>;

  result_type operator()(const std::string& fontName);
};


#endif // !VOXL_FONT_LOADER_H