#ifndef VOXL_UI_NODE_H
#define VOXL_UI_NODE_H


#include <cstdint>
#include <string>
#include <vector>

#include <entt/entt.hpp>
using namespace entt::literals;
#include <imgui/imgui.h>

#include "components/editor_component.h"
#include "components/name.h"
#include "resources/texture.h"
#include "core/resource_manager.h"
#include "utils/draw_component_header.h"


struct UINode
{
  bool isDirty;
  entt::entity parent;
  std::vector<entt::entity> children;
};


template<>
struct EditorComponent<UINode>
{
  static void Display(UINode& node, entt::registry* registry)
  {
    ImGui::PushID(&node);
    
    if (DrawComponentHeader("\tUI Node"))
    {
      ImGui::Checkbox("Is Dirty", &node.isDirty);
      std::string parent_name = (node.parent == entt::null) ? "None" : std::to_string((uint32_t)node.parent);
      ImGui::Text("Parent ID: %s", parent_name.c_str());

      if (node.children.empty())
      {
        ImGui::TextDisabled("No children attached");
      }
      else
      {
        auto& resource_manager = registry->ctx().get<ResourceManager>();

        auto icon_res = resource_manager.GetByID<Texture>("tex_icon"_hs);
        ImTextureID icon_id = 0;

        if (icon_res)
          icon_id = (ImTextureID)(uintptr_t)icon_res.handle().get()->handle;

        // itérer à l'envers ou avec un index pour permettre la suppression sans casser l'itérateur
        // merci gemini pour l'aide
        for (int i = 0; i < node.children.size(); ++i)
        {
          entt::entity child_entity = node.children[i];
          ImGui::PushID(i);

          ImGui::AlignTextToFramePadding();
          
          float button_size = ImGui::GetTextLineHeight(); 
          float content_width = ImGui::GetContentRegionAvail().x;

          std::string name;
          if (registry->all_of<Name>(child_entity)) name = registry->get<Name>(child_entity).buffer;
          ImGui::Text("%s", name.c_str());

          ImGui::SameLine(content_width - button_size);
          bool clicked = ImGui::InvisibleButton("##remove", ImVec2(button_size, button_size));
          
          bool is_hovered = ImGui::IsItemHovered();
          ImU32 cross_color = is_hovered ? IM_COL32(255, 0, 0, 255) : ImGui::GetColorU32(ImGuiCol_Text);

          ImVec2 p_min = ImGui::GetItemRectMin();
          ImVec2 p_max = ImGui::GetItemRectMax();

          if (icon_id != 0)
          {
            ImGui::GetWindowDrawList()->AddImage(
              icon_id, 
              p_min, 
              p_max, 
              ImVec2(0, 1), 
              ImVec2(1, 0), 
              cross_color
            );
          }
          else
          {
            ImVec2 text_size = ImGui::CalcTextSize("X");
            ImVec2 text_pos = ImVec2(
              p_min.x + (button_size - text_size.x) * 0.5f, 
              p_min.y + (button_size - text_size.y) * 0.5f
            );
            ImGui::GetWindowDrawList()->AddText(text_pos, cross_color, "X");
          }

          if (clicked)
          {
              if(registry->valid(child_entity) && registry->all_of<UINode>(child_entity)) 
                registry->get<UINode>(child_entity).parent = entt::null;

              node.children.erase(node.children.begin() + i);
              i--;
          }

          ImGui::PopID();
        }
      }

      ImGui::Separator();

      ImGui::Button("DROP ENTITY HERE TO ADD CHILD", ImVec2(ImGui::GetContentRegionAvail().x, 30));

      // merci gemini pour le code drop
      if (ImGui::BeginDragDropTarget())
      {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY"))
        {
          entt::entity dropped_entity = *(const entt::entity*)payload->Data;
          bool already_child = std::find(node.children.begin(), node.children.end(), dropped_entity) != node.children.end();
          
          if (!already_child && registry->valid(dropped_entity))
          {
            node.children.push_back(dropped_entity);

            if (registry->all_of<UINode>(dropped_entity))
            {
              // TODO implémenter la mise à jour du parent pour l'enfant 
              // Note: Comme cette fonction Display ne connait pas l'ID de l'entité "Mère" (node owner),
              // on ne peut pas mettre à jour `childNode.parent = ownerID` ici facilement 
              // sans modifier la signature de Display(). 
              // Pour l'instant, on ajoute juste à la liste des enfants.
            }
          }
        }
        ImGui::EndDragDropTarget();
      }
    }
    ImGui::PopID();
  }

  static void Register()
  {
    entt::meta_factory<UINode>{}
      .type(entt::type_id<UINode>().hash())
      .data<&UINode::isDirty>("is_dirty"_hs)
      .data<&UINode::parent>("parent"_hs)
      .data<&UINode::children>("children"_hs)
      .func<&EditorComponent<UINode>::Display>("display"_hs);
  }
};


#endif // !VOXL_UI_NODE_H