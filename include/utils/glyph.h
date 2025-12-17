#ifndef VOXL_GLYPH_H
#define VOXL_GLYPH_H


#include <ostream>

#include <glm/glm.hpp>

struct Glyph
{
  float advance;
  // toujours dans l'ordre left, bottom, right, top
  glm::vec4 planeBounds;
  glm::vec4 atlasBounds;

  friend std::ostream& operator<<(std::ostream& stream, const Glyph& g)
  {
    return stream << "glyph{\n" <<
      "\t" << g.advance << "\n" <<
      "\t" << g.planeBounds.x << ", " << g.planeBounds.y << ", " << g.planeBounds.z << ", " << g.planeBounds.w << "\n" <<
      "\t" << g.atlasBounds.x << ", " << g.atlasBounds.y << ", " << g.atlasBounds.z << ", " << g.atlasBounds.w << "\n}\n";
  }
};





#endif // !VOXL_GLYPH_H