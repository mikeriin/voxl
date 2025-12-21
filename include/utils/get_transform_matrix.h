#ifndef VOXL_GET_TRANSFORM_MATRIX_H
#define VOXL_GET_TRANSFORM_MATRIX_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "components/transform.h"


inline glm::mat4 GetTransformMatrix(const Transform& t)
{
  glm::mat4 model = glm::mat4(1.0f);

  model = glm::translate(model, t.position);

  model = glm::rotate(model, glm::radians(t.rotation.x), glm::vec3(1, 0, 0));
  model = glm::rotate(model, glm::radians(t.rotation.y), glm::vec3(0, 1, 0));
  model = glm::rotate(model, glm::radians(t.rotation.z), glm::vec3(0, 0, 1));

  model = glm::scale(model, t.scale);

  return model;
}


#endif // !VOXL_GET_TRANSFORM_MATRIX_H