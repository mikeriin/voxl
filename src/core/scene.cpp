#include "core/scene.h"


#include <cstdint>
#include <string>

#include <imgui/imgui.h>


Scene::Scene(entt::registry* registry)
  : _pRegistry(registry)
{

}


void Scene::DisplayGraph()
{
  if (ImGui::Begin("Hierarchy"))
  {
    if (ImGui::BeginChild("List", ImVec2(0,0), true))
    {
      _pRegistry->view<entt::entity>().each([this](auto entity)
      {
        std::string label = "Entity " + std::to_string((uint32_t) entity);
        
        bool is_selected = (_selectedEntity == entity);
        
        if (ImGui::Selectable(label.c_str(), is_selected))
        {
          _selectedEntity = entity;
        }
      });

      ImGui::EndChild();
    }
  }
  ImGui::End();
}