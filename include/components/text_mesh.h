#ifndef VOXL_TEXT_MESH_H
#define VOXL_TEXT_MESH_H


#include <vector>

#include <glm/glm.hpp>


struct TextVertex
{
  glm::vec3 position;
  glm::vec2 texCoord;
};


struct TextMesh
{
  std::vector<TextVertex> vertices;
  std::vector<unsigned int> indices;

  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;
};


#endif // !VOXL_TEXT_MESH_H