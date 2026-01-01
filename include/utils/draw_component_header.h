#ifndef VOXL_DRAW_COMPONENT_HEADER_H
#define VOXL_DRAW_COMPONENT_HEADER_H


#include <imgui/imgui.h>


inline bool DrawComponentHeader(const char* label, void* id_ptr = nullptr)
{
  if (id_ptr) ImGui::PushID(id_ptr);
  else ImGui::PushID(label);

  bool* p_open = ImGui::GetStateStorage()->GetBoolRef(ImGui::GetID("IsOpen"), true);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 3.0f));
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.31f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.80f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));

  if (ImGui::Button(label, ImVec2(-1, 0))) *p_open = !(*p_open);
  
  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar(1);
  ImGui::PopID();

  return *p_open;
}


#endif // !VOXL_DRAW_COMPONENT_HEADER_H