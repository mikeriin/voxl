#ifndef VOXL_RECT_TRANSFORM_H
#define VOXL_RECT_TRANSFORM_H


#include <glm/glm.hpp>
#include <entt/entt.hpp>
using namespace entt::literals;
#include <imgui/imgui.h>

#include "components/editor_component.h"
#include "utils/draw_component_header.h"


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
  float width;
  float height;
  Anchor anchor;
  glm::vec2 pivot;
};


inline void DrawAnchorSelectorGrid(RectTransform& transform)
{
  ImGui::Text("Anchor Presets");
  
  ImVec2 boutton_size(25, 25);
  int columns = 3;
  
  // Mapping visuel : On dessine une grille 3x3, mais on doit mapper 
  // chaque case (0 à 8) vers votre valeur d'enum spécifique.
  // Ordre visuel : 
  // TL(0) | MT(1) | TR(2)
  // ML(3) | C (4) | MR(5)
  // BL(6) | MB(7) | BR(8)
  
  Anchor visualToEnumMap[9] = {
    Anchor::TOP_LEFT,    Anchor::MIDDLE_TOP,    Anchor::TOP_RIGHT,
    Anchor::MIDDLE_LEFT, Anchor::CENTER,        Anchor::MIDDLE_RIGHT,
    Anchor::BOTTOM_LEFT, Anchor::MIDDLE_BOTTOM, Anchor::BOTTOM_RIGHT
  };

  ImGui::BeginGroup();
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3, 3));

  for (int i = 0; i < 9; i++)
  {
    Anchor btnAnchor = visualToEnumMap[i];
    
    bool is_selected = (transform.anchor == btnAnchor);
    if (is_selected)
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f)); // Bleu
    
    ImGui::PushID(i);
    const char* label = (btnAnchor == Anchor::CENTER) ? "+" : " "; 
    if (ImGui::Button(label, boutton_size))
    {
      transform.anchor = btnAnchor;
      
      // TODO mettre à jour le pivot automatiquement 
      // UpdatePivotFromAnchor(transform); 
    }
    ImGui::PopID();
    
    if (is_selected)
      ImGui::PopStyleColor();

    if ((i + 1) % 3 != 0)
      ImGui::SameLine();
  }

  ImGui::PopStyleVar();
  ImGui::EndGroup();
  
  ImGui::SameLine();
  ImGui::TextDisabled("(%s)", 
    (transform.anchor == Anchor::CENTER) ? "Center" : "Custom");
}


template<>
struct EditorComponent<RectTransform>
{
  static void Display(RectTransform& t, entt::registry* registry)
  {
    ImGui::PushID(&t);

    if (DrawComponentHeader("\tRect Transform"))
    {
      ImGui::DragFloat3("Position", &t.position.x, 0.1f);
      ImGui::DragFloat("Rotation", &t.rotation, 0.1f);
      ImGui::DragFloat("Width", &t.width, 0.1f);
      ImGui::DragFloat("Height", &t.height, 0.1f);
      ImGui::Separator();
      DrawAnchorSelectorGrid(t);
      ImGui::Separator();
      ImGui::DragFloat2("Pivot", &t.pivot.x, 0.1f);
    }
    ImGui::PopID();
  }

  static void Register()
  {
    entt::meta_factory<RectTransform>{}
      .type(entt::type_id<RectTransform>().hash())
      .data<&RectTransform::position>("position"_hs)
      .data<&RectTransform::rotation>("rotation"_hs)
      .data<&RectTransform::width>("width"_hs)
      .data<&RectTransform::height>("height"_hs)
      .data<&RectTransform::anchor>("anchor"_hs)
      .data<&RectTransform::pivot>("pivot"_hs)
      .func<&EditorComponent<RectTransform>::Display>("display"_hs);
  }
};


#endif // !VOXL_RECT_TRANSFORM_H