#include "core/dev_console.h"


#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include <SDL3/SDL_keycode.h>
using namespace entt::literals;
#include <imgui/imgui.h>

#include "core/command_manager.h"
#include "events/dev_console_message_event.h"


DevConsole::DevConsole(entt::registry* registry)
  : _pRegistry(registry)
{
  _inputBuffer[0] = '\0';
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();
  dispatcher.sink<DevConsoleMessageEvent>().connect<&DevConsole::onDevConsoleMessage>(this);

  registerCommands();
}

DevConsole::~DevConsole() {}


bool DevConsole::Init()
{
  return true;
}


void DevConsole::OpenDevConsole(bool* pOpen)
{
  if (!pOpen || !(*pOpen))
    return;

  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.075f, 0.075f, 0.075f, 0.75f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  ImGui::SetNextWindowSize(ImVec2(DEV_CONSOLE_WIDTH, DEV_CONSOLE_HEIGHT), ImGuiCond_FirstUseEver);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar 
    | ImGuiWindowFlags_NoCollapse 
    | ImGuiWindowFlags_NoResize;

  if (ImGui::Begin("DevConsole", pOpen, window_flags))
  {
    float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
    ImGui::BeginChild("History", ImVec2(0, -footer_height), false);
    
    for (const auto& item : _history) ImGui::TextWrapped("%s", item.c_str()); 

    if (_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
      ImGui::SetScrollHereY(1.0f);
      _scrollToBottom = false;
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar();

    if (ImGui::IsWindowAppearing())
      ImGui::SetKeyboardFocusHere();
    
    ImGui::PushItemWidth(-1);
    
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1.0f, 1.0f, 1.0f, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_InputTextCursor, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    bool enter_pressed = ImGui::InputTextWithHint("##cmd", "$", _inputBuffer, IM_ARRAYSIZE(_inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopStyleColor(4);
    if (enter_pressed) 
    {
      if (strlen(_inputBuffer) > 0)
      {
        auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

        dispatcher.enqueue(DevConsoleMessageEvent{
          .level = DebugLevel::NONE,
          .buffer = _inputBuffer,
        });

        std::vector<std::string> tokens;
        std::stringstream ss(_inputBuffer);
        std::string word;

        // on fait juste du parsing + on accepte que 10 arguments, ça évite de check un long texte pour rien
        while (std::getline(ss, word, ' ') && tokens.size() < 10) 
        {
          tokens.push_back(word);
        }

        if (!tokens.empty())
        {
          // c'est une commande et elle à un nom donc on la process
          const std::string& CMD_NAME = tokens[0];
          if (CMD_NAME.size() > 0 && CMD_NAME[0] == '$')
          {
            // on récupère les arguments s'il y en a, sinon args sera un vecteur vide ce qui correspond aux commandes sans arguments
            std::vector<std::string> args;
            if (tokens.size() > 1) args.assign(tokens.begin() + 1, tokens.end());

            auto& command_manager = _pRegistry->ctx().get<CommandManager>();
            std::string name = CMD_NAME.substr(1);

            
            bool does_cmd_exist = command_manager.Execute(name, args);
            if (!does_cmd_exist) 
            {
              dispatcher.enqueue(DevConsoleMessageEvent{
                .level = DebugLevel::WARNING,
                .buffer = name + " command doesn't exist",
              });
            }
          }
        }

        _inputBuffer[0] = '\0';
        _scrollToBottom = true;
      }

      ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::PopItemWidth();
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(1);
}


void DevConsole::onDevConsoleMessage(const DevConsoleMessageEvent& e)
{
  _history.push_back(GetDebugLevel(e.level) + "> " + e.buffer);
}

void DevConsole::registerCommands()
{
  auto& command_manager = _pRegistry->ctx().get<CommandManager>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();

  std::string helper = "$clear --> doesn't need args";
  command_manager.Register(Command{
    .name = "clear",
    .helper = helper,
    .func = [this, &dispatcher, helper](auto& args)
    {
      try 
      {
        if (!args.empty()) 
          throw std::out_of_range("[Engine] $clear doesn't accept args");
        _history.clear();
      } 
      // la dite erreur
      catch (const std::out_of_range &e) 
      {
        dispatcher.enqueue(DevConsoleMessageEvent{
          .level = DebugLevel::WARNING,
          .buffer = helper,
        });
        std::cerr << e.what() << "\n";
      }
    }
  }); 
}