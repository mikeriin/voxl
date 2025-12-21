#ifndef VOXL_RECT_TRANSFORM_H
#define VOXL_RECT_TRANSFORM_H


#include <glm/glm.hpp>


enum Anchor
{
  CENTER,
  TOP_LEFT,
  MIDDLE_TOP,
  TOP_RIGHT,
  MIDDLE_RIGHT,
  BOTTOM_RIGHT,
  MIDDLE_BOTTOM,
  BOTTOM_LEFT,
  MIDDLE_LEFT
};


struct RectTransform
{
  glm::vec3 position;
  float rotation;
  int width;
  int height;
  glm::vec2 pivot;
  Anchor anchor;
};


#endif // !VOXL_RECT_TRANSFORM_H