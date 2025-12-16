#ifndef VOXL_GENERATE_FONT_MESH_H
#define VOXL_GENERATE_FONT_MESH_H


#include <string>

#include "components/font_mesh.h"
#include "components/font.h"
#include "font_vertex.h"


inline FontMesh GenerateFontMesh(const std::string& text, const Font& font, float startX, float startY, float fontSize)
{
  FontMesh mesh;
  unsigned int indexStart = static_cast<unsigned int>(mesh.vertices.size());

  float cursorX = startX;
  float cursorY = startY;

  for (char c: text)
  {
    unsigned char uc = (unsigned char)c;
    if (uc > 127)
    {
      if ((uc & 0xC0) == 0xC0) cursorX += fontSize * 0.5f;
      continue;
    }

    const auto& glyph = font.metrics[c];

    // gestion des sauts de ligne
    if (c == '\n')
    {
      cursorX = startX;
      cursorY -= fontSize;
    }

    // si la lettre n'est pas un espace
    if (glyph.planeRight > 0.0)
    {
      // coordonnées écran (screen space)
      // on * par fontSize parce que planeBounds est normalisé
      float x0 = cursorX + (glyph.planeLeft * fontSize);
      float y0 = cursorY + (glyph.planeBottom * fontSize);
      float x1 = cursorX + (glyph.planeRight * fontSize);
      float y1 = cursorY + (glyph.planeTop * fontSize);

      // uv déjà calculés lors du chargement
      float u0 = glyph.uvLeft;
      float v0 = glyph.uvBottom;
      float u1 = glyph.uvRight;
      float v1 = glyph.uvTop;

      mesh.vertices.push_back(FontVertex{.position = {x0, y0, 0.0f}, .uv = {u0, v0}}); // bas-gauche
      mesh.vertices.push_back(FontVertex{.position = {x1, y0, 0.0f}, .uv = {u1, v0}}); // bas-droite
      mesh.vertices.push_back(FontVertex{.position = {x1, y1, 0.0f}, .uv = {u1, v1}}); // haut-droite
      mesh.vertices.push_back(FontVertex{.position = {x0, y1, 0.0f}, .uv = {u0, v1}}); // haut-gauche

      mesh.indices.insert(mesh.indices.end(), {
        indexStart + 0, indexStart + 1, indexStart + 2, // triangle bas-droite
        indexStart + 0, indexStart + 2, indexStart + 3  // triangle haut_gauche
      });

      indexStart = mesh.vertices.size();

      cursorX += glyph.advance * fontSize;
    }
  }

  return mesh;
}


#endif // !VOXL_GENERATE_FONT_MESH_H