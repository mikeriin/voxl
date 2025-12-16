#ifndef VOXL_FONT_MESH_H
#define VOXL_FONT_MESH_H


#include <vector>

#include "utils/font_vertex.h"


struct FontMesh
{
  std::vector<FontVertex> vertices;
  std::vector<unsigned int> indices;

  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;
};


#endif // !VOXL_FONT_MESH_H