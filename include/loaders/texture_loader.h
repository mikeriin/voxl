#ifndef VOXL_TEXTURE_LOADER_H
#define VOXL_TEXTURE_LOADER_H


#include <memory>
#include <string>

#include "resources/texture.h"


struct TextureLoader
{
  using result_type = std::shared_ptr<Texture>;

  result_type operator()(const std::string& texPath);
};


#endif // !VOXL_TEXTURE_LOADER_H