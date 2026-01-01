#ifndef VOXL_TRANSFORM_H
#define VOXL_TRANSFORM_H


#include <entt/core/type_info.hpp>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
using namespace entt::literals;
#include <imgui/imgui.h>

#include "components/editor_component.h"
#include "utils/draw_component_header.h"


struct Transform
{
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
};


template<>
struct EditorComponent<Transform>
{
  static void Display(Transform& t, entt::registry* registry)
  {
    ImGui::PushID(&t);

    if (DrawComponentHeader("\tTransform"))
    {
      ImGui::DragFloat3("Position", &t.position.x, 0.1f);
      ImGui::DragFloat3("Rotation", &t.rotation.x, 0.1f);
      ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
    }
    ImGui::PopID();
  }

  static void Register()
  {
    entt::meta_factory<Transform>{}
      .type(entt::type_id<Transform>().hash())
      .data<&Transform::position>("position"_hs)
      .data<&Transform::rotation>("rotation"_hs)
      .data<&Transform::scale>("scale"_hs)
      .func<&EditorComponent<Transform>::Display>("display"_hs);
  }
};


#endif // !VOXL_TRANSFORM_H