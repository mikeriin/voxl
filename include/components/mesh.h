#ifndef VOXL_MESH_H
#define VOXL_MESH_H


#include <vector>

#include <glm/glm.hpp>


struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 textureCoordinates;
  glm::vec4 color;
};


struct Mesh
{
  glm::vec4 color{1.0f};
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  int indiceCount;

  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;

  void* pBuffer; // utile pour le 'persistent mapping', on stocke le pointeur du 'mapping' pour pouvoir copier les données dessus à la volée
};


#endif // !VOXL_MESH_H