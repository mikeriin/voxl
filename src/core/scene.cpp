#include "core/scene.h"
#include "components/text_mesh.h"


#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>
#include <iostream>
using namespace entt::literals;
#include <string>

#include <imgui/imgui.h>

#include "core/engine_context.h"
#include "components/transform.h"
#include "components/rect_transform.h"
#include "components/text.h"
#include "components/text_mesh.h"
#include "components/ui_node.h"
#include "components/name.h"


Scene::Scene(entt::registry* registry)
  : _pRegistry(registry),
    _selectedEntity(entt::null),
    _isInspectorOpen(false),
    _entityRenaming(entt::null),
    _focusRenameInput(false)
{
  // _renameBuffer[sizeof(_renameBuffer) - 1] = '\0';
}


void Scene::DisplayGraph(bool* pOpen)
{
  auto& engine_context = _pRegistry->ctx().get<EngineContext>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.075f, 0.075f, 0.075f, 0.75f));
  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.02f, 0.02f, 0.02f, 0.75f)); 
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.01f, 0.01f, 0.01f, 0.75f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoScrollbar;

  if (ImGui::Begin("Hierarchy", pOpen, window_flags))
  {
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);

    if (ImGui::BeginChild("List", ImVec2(0,0), false))
    {
      _pRegistry->view<entt::entity>().each([this, &dispatcher, &engine_context](auto entity)
      {
        std::string label;
        if (_pRegistry->all_of<Name>(entity)) label = _pRegistry->get<Name>(entity).buffer;
        else label = "Entity " + std::to_string((uint32_t)entity);

        if (_entityRenaming == entity)
        {
          if (_focusRenameInput) 
          {
            ImGui::SetKeyboardFocusHere();
            ImGui::SetScrollHereY();
            _focusRenameInput = false;
          }

          ImGui::PushItemWidth(-1); 

          if (ImGui::InputText("##Rename", _renameBuffer, sizeof(_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
          {
            if (!_pRegistry->all_of<Name>(entity)) _pRegistry->emplace<Name>(entity, std::string(_renameBuffer));
            else _pRegistry->get<Name>(entity).buffer = std::string(_renameBuffer);
            
            _entityRenaming = entt::null; 
          }
          
          if (!ImGui::IsItemActive() && (ImGui::IsMouseClicked(0) || ImGui::IsKeyPressed(ImGuiKey_Escape))) _entityRenaming = entt::null;

          ImGui::PopItemWidth();
        }
        else
        {
          bool is_selected = (_selectedEntity == entity);
          
          std::string id_label = label + "##" + std::to_string((uint32_t)entity);

          if (ImGui::Selectable(id_label.c_str(), is_selected))
          {
            if (is_selected) selectEntity(entt::null);
            else selectEntity(entity);
          }

          if (ImGui::BeginDragDropSource()) 
          {
            ImGui::SetDragDropPayload("DND_ENTITY", &entity, sizeof(entt::entity));
            ImGui::Text("%s", label.c_str()); 
            ImGui::EndDragDropSource();
          }

          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) selectEntity(entity);

          if (ImGui::BeginPopupContextItem()) 
          {
            if (ImGui::MenuItem("Rename"))
            {
              selectEntity(entity);

              _entityRenaming = entity;
              _focusRenameInput = true;

              strcpy_s(_renameBuffer, label.c_str()); 
            }
            
            if (ImGui::MenuItem("Delete")) 
            {
              if (_entityRenaming == entity) _entityRenaming = entt::null;
              
              selectEntity(entt::null);

              engine_context.entitiesToDelete.push_back(entity);
            }

            ImGui::EndPopup();
          }
        }
        
      });

      ImGui::EndChild();
      ImGui::PopStyleVar(1);
    }

    // on peut créer une entité en faisant clique droit et en sélectionnant "Create"
    if (ImGui::BeginPopupContextWindow("Background Popup", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
      if (ImGui::MenuItem("Create New Entity"))
      {
        auto e = _pRegistry->create();
        selectEntity(e);

        _entityRenaming = e;
        _focusRenameInput = true;

        
        std::string label = _pRegistry->emplace<Name>(e, Name{
          .buffer = "Entity " + std::to_string((uint32_t)e)
        }).buffer;
        strcpy_s(_renameBuffer, label.c_str()); 

        dispatcher.enqueue<DevConsoleMessageEvent>(DevConsoleMessageEvent
          {
          .level = DebugLevel::INFO, 
          .buffer = "Created " + label
        });
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Save"))
      {
        dispatcher.enqueue<DevConsoleMessageEvent>(DevConsoleMessageEvent
          {
          .level = DebugLevel::INFO, 
          .buffer = "Saving scene"
        });
      }

      if (ImGui::MenuItem("Load"))
      {
        dispatcher.enqueue<DevConsoleMessageEvent>(DevConsoleMessageEvent
          {
          .level = DebugLevel::INFO, 
          .buffer = "Loading scene"
        });
      }
      ImGui::EndPopup();
    }
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(3);

  if (_isInspectorOpen) displayInspector();
}


void Scene::displayInspector()
{
  if (_selectedEntity == entt::null || !_pRegistry->valid(_selectedEntity))
  {
    selectEntity(entt::null);
    return;
  }

  auto& engine_context = _pRegistry->ctx().get<EngineContext>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.075f, 0.075f, 0.075f, 0.75f));
  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.02f, 0.02f, 0.02f, 0.75f)); 
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.01f, 0.01f, 0.01f, 0.75f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  bool was_open = (_selectedEntity != entt::null);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoScrollbar;

  if (ImGui::Begin("Inspector", &_isInspectorOpen, window_flags))
  {
    float button_height = ImGui::GetFrameHeightWithSpacing();
    
    // Calcul de la taille disponible pour éviter un enfant de taille 0 ou négative
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    contentRegion.y -= button_height; 
    if (contentRegion.y < 1.0f) contentRegion.y = 1.0f; // Sécurité

    if (ImGui::BeginChild("Components", contentRegion, false))
    {
      for (auto [id, storage]: _pRegistry->storage())
      {
        if (storage.contains(_selectedEntity))
        {
          auto meta_type = entt::resolve(id);
          if (!meta_type) continue;

          auto display_func = meta_type.func("display"_hs);
          if (display_func)
          {
            void* raw_ptr = storage.value(_selectedEntity);
            
            if (!raw_ptr) continue;

            entt::registry* pRegistry = _pRegistry;
            auto registry_meta = entt::forward_as_meta(pRegistry);
            entt::meta_any component_meta = meta_type.from_void(raw_ptr);

            auto result = display_func.invoke(component_meta, registry_meta);

            if (!result) 
              std::cerr << "Failed to display component: " << meta_type.info().name() << "\n";
          }
        }
      }
      ImGui::EndChild();
    }

    if (ImGui::Button("Add Component", ImVec2(-1, 0))) // -1 en X pour étirer le bouton
    {
      ImGui::OpenPopup("Add Component Popup");
    }

    if (ImGui::BeginPopup("Add Component Popup")) 
    {
      if (ImGui::MenuItem("Transform"))
      {
        if (_pRegistry->all_of<RectTransform>(_selectedEntity)) _pRegistry->remove<RectTransform>(_selectedEntity);
        if (_pRegistry->all_of<UINode>(_selectedEntity)) _pRegistry->remove<UINode>(_selectedEntity);
        if (_pRegistry->all_of<Text>(_selectedEntity)) _pRegistry->remove<Text>(_selectedEntity);
        if (_pRegistry->all_of<TextMesh>(_selectedEntity)) _pRegistry->remove<TextMesh>(_selectedEntity);
        addComponent<Transform>();
      }

      if (ImGui::MenuItem("Rect Transform"))
      {
        if (_pRegistry->all_of<Transform>(_selectedEntity)) _pRegistry->remove<Transform>(_selectedEntity);
        addComponent<RectTransform>();
      }

      if (ImGui::MenuItem("Text"))
      {
        if (_pRegistry->all_of<Transform>(_selectedEntity)) _pRegistry->remove<Transform>(_selectedEntity);
        addComponent<Text>();
        addComponent<TextMesh>();
        addComponent<RectTransform>();
        addComponent<UINode>();
      }

      if (ImGui::MenuItem("UI Node"))
      {
        if (_pRegistry->all_of<Transform>(_selectedEntity)) _pRegistry->remove<Transform>(_selectedEntity);
        addComponent<UINode>();
      }

      ImGui::EndPopup();
    }
  }

  if (was_open && !_isInspectorOpen) _selectedEntity = entt::null;

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(3);
}


void Scene::selectEntity(entt::entity e)
{
  _selectedEntity = e;
  _isInspectorOpen = (e == entt::null) ? false: true;
}