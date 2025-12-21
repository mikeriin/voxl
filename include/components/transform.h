#ifndef VOXL_TRANSFORM_H
#define VOXL_TRANSFORM_H


#include <glm/glm.hpp>


struct Transform
{
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};


#endif // !VOXL_TRANSFORM_H