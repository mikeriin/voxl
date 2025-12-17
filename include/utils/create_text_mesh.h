#ifndef VOXL_CREATE_TEXT_MESH_H
#define VOXL_CREATE_TEXT_MESH_H


#include <cstddef>
#include <cstdint>
#include <iostream>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "components/text.h"
#include "components/text_mesh.h"
#include "resources/font.h"
#include "utils/glyph.h"


inline TextMesh CreateTextMesh(const Text& text)
{
  TextMesh mesh;

  uint32_t indexStart = (uint32_t)mesh.vertices.size();

  float cursorX = text.position.x;
  float cursorY = text.position.y;

  for (uint32_t c: text.text)
  {
    auto it = text.pFont->glyphs.find(c);
    if (it == text.pFont->glyphs.end())
    {
      it = text.pFont->glyphs.find((uint32_t)'?');
    }

    if (it == text.pFont->glyphs.end()) continue;

    if (c == (uint32_t)' ') 
    {
      auto it = text.pFont->glyphs.find((uint32_t)' ');
      if (it == text.pFont->glyphs.end()) 
      {
        cursorX += text.fontSize;
        continue;
      }

      cursorX += it->second.advance * text.fontSize;
      continue;
    }

    if (c == (uint32_t)'\n')
    {
      cursorY -= text.fontSize;
      cursorX = text.position.x;
      continue;
    }

    Glyph g = it->second;

    glm::vec3 botleft{ cursorX + g.planeBounds.x * text.fontSize, cursorY + g.planeBounds.y * text.fontSize, 0.0f};
    glm::vec3 botright{ cursorX + g.planeBounds.z * text.fontSize, cursorY + g.planeBounds.y * text.fontSize, 0.0f};
    glm::vec3 topright{ cursorX + g.planeBounds.z * text.fontSize, cursorY + g.planeBounds.w * text.fontSize, 0.0f};
    glm::vec3 topleft{ cursorX + g.planeBounds.x * text.fontSize, cursorY + g.planeBounds.w * text.fontSize, 0.0f};

    glm::vec2 uvbl{g.atlasBounds.x, g.atlasBounds.y};
    glm::vec2 uvbr{g.atlasBounds.z, g.atlasBounds.y};
    glm::vec2 uvtr{g.atlasBounds.z, g.atlasBounds.w};
    glm::vec2 uvtl{g.atlasBounds.x, g.atlasBounds.w};

    mesh.vertices.insert(mesh.vertices.end(), {
      {botleft, uvbl},
      {botright, uvbr},
      {topright, uvtr},
      {topleft, uvtl}
    });

    mesh.indices.insert(mesh.indices.end(), {
      indexStart, indexStart + 1, indexStart + 2,
      indexStart + 2, indexStart + 3, indexStart
    });

    indexStart = (uint32_t)mesh.vertices.size();
    cursorX += g.advance * text.fontSize;
  }

  glCreateVertexArrays(1, &mesh.vao);
  glCreateBuffers(1, &mesh.vbo);
  glCreateBuffers(1, &mesh.ebo);

  glNamedBufferStorage(mesh.vbo, sizeof(TextVertex) * mesh.vertices.size(), mesh.vertices.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(TextVertex));

  glNamedBufferStorage(mesh.ebo, sizeof(unsigned int) * mesh.indices.size(), mesh.indices.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

  glEnableVertexArrayAttrib(mesh.vao, 0); // position => 0
  glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(TextVertex, position));
  glVertexArrayAttribBinding(mesh.vao, 0, 0);

  glEnableVertexArrayAttrib(mesh.vao, 1); // texCoord => 1
  glVertexArrayAttribFormat(mesh.vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(TextVertex, texCoord));
  glVertexArrayAttribBinding(mesh.vao, 1, 0);

  // TODO créer un buffer pour 10000 caractères et map le text dans le buffer => plus rapide !

  if (!mesh.vao && !mesh.vbo && !mesh.ebo) 
  {
    std::cerr << "Failed to create vao, vbo or ebo\n";
    return TextMesh{};
  }

  return mesh;
}


#endif // !VOXL_CREATE_TEXT_MESH_H