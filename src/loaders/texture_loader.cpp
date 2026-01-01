#include "loaders/texture_loader.h"


#include <iostream>

#include <glad/glad.h>
#include <stb_image.h>


TextureLoader::result_type TextureLoader::operator()(const std::string& texPath)
{
    Texture tex;

    int width;
    int height;
    int channels;
    std::string tex_final_path = std::string("assets/textures/") + texPath;
    const char* tex_final_path_c = tex_final_path.c_str();

    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load(tex_final_path_c, &width, &height, &channels, 0);
    if (!pixels)
    {
      std::cerr << "Failed to load '" << tex_final_path_c << "': " << stbi_failure_reason() << "\n";
      return nullptr;
    }

    unsigned int internal_format = 0;
    unsigned int format = 0;
    switch (channels) 
    {
      case 1:
        internal_format = GL_R8;
        format = GL_RED;
      break;

      case 2:
        internal_format = GL_RG8;
        format = GL_RG;
      break;

      case 3:
        internal_format = GL_RGB8;
        format = GL_RGB;
      break;

      case 4:
        internal_format = GL_RGBA8;
        format = GL_RGBA;
      break;

      default:
        internal_format = GL_RGB8;
        format = GL_RGB;
      break;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &tex.handle);
    glTextureStorage2D(tex.handle, 1, internal_format, width, height); // pas de mipmap
    glTextureSubImage2D(tex.handle, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);

    glTextureParameteri(tex.handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex.handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(pixels);

    if (!tex.handle)
    {
      std::cerr << "Failed to create GL Texture\n";
      return nullptr;
    }

    return std::make_shared<Texture>(tex);
  }