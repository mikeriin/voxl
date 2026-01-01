#ifndef VOXL_TEXT_H
#define VOXL_TEXT_H


#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>
#include <string.h>
#include <string>

#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <entt/entt.hpp>
using namespace entt::literals;

#include "resources/font.h"
#include "components/editor_component.h"
#include "events/dev_console_message_event.h"
#include "core/resource_manager.h"
#include "utils/draw_component_header.h"


struct Text
{
  std::string text;
  Font* pFont;
  float fontSize;
  glm::vec3 position;
  glm::vec4 color;

  // TODO implémenter un système de bounding box pour le texte cliquable
  glm::vec2 min;
  glm::vec2 max;
};


template<>
struct EditorComponent<Text>
{
  static void Display(Text& t, entt::registry* registry)
  {
    ImGui::PushID(&t);

    if (DrawComponentHeader("\tText"))
    {
      static char text_buffer[512] = "";
      strncpy_s(text_buffer, t.text.c_str(), sizeof(text_buffer));
      text_buffer[sizeof(text_buffer) - 1] = '\0';

      if (ImGui::InputTextWithHint("##cmd", "Text...", text_buffer, IM_ARRAYSIZE(text_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
      {
        t.text = text_buffer;
      }

      ImGui::Spacing();
      ImGui::Text("Font");

      float button_width = 60.0f;
      float spacing = ImGui::GetStyle().ItemSpacing.x;

      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - button_width - spacing);

      std::string preview_value = "No Font";

      auto& resource_manager = registry->ctx().get<ResourceManager>();
      const auto& font_names = resource_manager.GetFontNames();

      for (const auto& name: font_names)
      {
        auto res_font = resource_manager.Get<Font>(name);
        if (res_font && res_font.handle().get() == t.pFont)
        {
          preview_value = name;
          break;
        }
      }

      if (ImGui::BeginCombo("##font_selector", preview_value.c_str()))
      {
        for (const auto& name: font_names)
        {
          auto res_font = resource_manager.Get<Font>(name);
          bool is_selected = (t.pFont != nullptr && res_font.handle().get() == t.pFont);

          if (ImGui::Selectable(name.c_str(), is_selected)) t.pFont = res_font.handle().get();

          if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      ImGui::SameLine();
      if (ImGui::Button("Import", ImVec2(button_width, 0)))
      {
        /* TODO importer la police en selectionnant un dossier
        le dossier doit contenir un atlas.png, un charset.txt, un font.ttf et un metrics.json 
        utiliser le générateur pour ovtenir ces fichiers 
        nécessaire pour l'affichage msdf */
        auto& dispatcher = registry->ctx().get<entt::dispatcher>();
        dispatcher.enqueue<DevConsoleMessageEvent>(DevConsoleMessageEvent{
          .level = DebugLevel::INFO,
          .buffer = "Font loaded"
        });
      }

      ImGui::DragFloat("Font Size", &t.fontSize, 0.5f, 1.0f, 100.0f);
      ImGui::DragFloat3("Position", &t.position.x); // TODO enlever la position de la font et utiliser la position du Rect Transform (implémentation ui à faire)
      ImGui::ColorEdit4("Color", &t.color.x);
    }
    ImGui::PopID();
  }

  static void Register()
  {
    entt::meta_factory<Text>{}
      .type(entt::type_id<Text>().hash())
      .data<&Text::text>("text"_hs)
      .data<&Text::pFont>("p_font"_hs)
      .data<&Text::fontSize>("font_size"_hs)
      .data<&Text::position>("position"_hs)
      .data<&Text::color>("color"_hs)
      .data<&Text::min>("min"_hs)
      .data<&Text::max>("max"_hs)
      .func<&EditorComponent<Text>::Display>("display"_hs);
  }
};


#endif // !VOXL_TEXT_H