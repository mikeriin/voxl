#ifndef VOXL_TEXT_MESH_H
#define VOXL_TEXT_MESH_H


#include <vector>

#include <glm/glm.hpp>
#include <entt/entt.hpp>
using namespace entt::literals;
#include <imgui/imgui.h>

#include "components/editor_component.h"
#include "utils/draw_component_header.h"


static constexpr int MAX_TEXT_LENGTH = 2048;


struct TextVertex
{
  glm::vec3 position;
  glm::vec2 textureCoordinates;
};


struct TextMesh
{
  std::vector<TextVertex> vertices;
  std::vector<unsigned int> indices;

  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;

  void* pBuffer;
};


template<>
struct EditorComponent<TextMesh>
{
  static void Display(TextMesh& mesh, entt::registry* registry)
  {
    ImGui::PushID(&mesh);

    DrawComponentHeader("\tText Mesh");

    ImGui::PopID();
  }

  static void Register()
  {
    entt::meta_factory<TextMesh>{}
      .type(entt::type_id<TextMesh>().hash())
      .data<&TextMesh::vertices>("vertices"_hs)
      .data<&TextMesh::indices>("indices"_hs)
      .data<&TextMesh::vao>("vao"_hs)
      .data<&TextMesh::vbo>("vbo"_hs)
      .data<&TextMesh::ebo>("ebo"_hs)
      .data<&TextMesh::pBuffer>("p_buffer"_hs)
      .func<&EditorComponent<TextMesh>::Display>("display"_hs);
  }
};


#endif // !VOXL_TEXT_MESH_H