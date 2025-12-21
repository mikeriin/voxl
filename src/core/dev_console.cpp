#include "core/dev_console.h"


#include <iostream>
#include <string>
#include <sstream>

#include <SDL3/SDL_keycode.h>
#include <vector>

#include "core/command_manager.h"
#include "core/console_context.h"
#include "core/game_context.h"
#include "events/game_state_change_event.h"
#include "loaders/font_loader.h"
#include "platform/input_handler.h"
#include "utils/create_text_mesh.h"
#include "utils/game_state.h"
#include "components/text.h"
#include "components/text_mesh.h"
#include "components/tags.h"
#include "components/timer.h"
#include "utils/get_unique.h"


DevConsole::DevConsole(entt::registry* registry)
  : _pRegistry(registry)
{
  _buffer = "";
}

DevConsole::~DevConsole() {}


bool DevConsole::Init()
{
  _font = LoadFont("roboto_mono");

  auto& game_context = _pRegistry->ctx().get<GameContext>();
  
  auto e = _pRegistry->create();
  _pRegistry->emplace<Console>(e);
  auto& text = _pRegistry->emplace<Text>(e, Text{
    .text = "",
    .pFont = &_font,
    .fontSize = CONSOLE_FONT_SIZE,
    .position = {10.0f, (float) game_context.screenInfo.height - BUFFER_Y_OFFSET, 0.0f}
  });
  _pRegistry->emplace<TextMesh>(e, CreateTextMesh(text));

  return true;
}


void DevConsole::Update()
{
  auto& game_context = _pRegistry->ctx().get<GameContext>();
  auto& input_handler = _pRegistry->ctx().get<InputHandler>();
  auto& dispatcher = _pRegistry->ctx().get<entt::dispatcher>();
  auto console_entity = GetUnique<Console>(*_pRegistry);
  
  if (game_context.currentState != GameState::CONSOLE) return; // pas besoin d'update la console, on ne l'utilise pas
  
  // on modifie le buffer de la console avant de générer le text qui sera affiché
  std::string temp_buffer = "> " + _buffer;
  _buffer += input_handler.GetTextInput();
  if (input_handler.IsKeyPressed(SDLK_BACKSPACE) && !_buffer.empty())
  {
    while (!_buffer.empty()) 
    {
      char c = _buffer.back();
      _buffer.pop_back();

      // en UTF-8 les octets de continuation commencent par '10xxxxxx'
      // le masque 0xC0 (11000000) permet de vérifier les deux premiers bits
      // si (c & 0xC0) != 0x80 (10000000), alors ce n'est PAS un octet de continuation
      // c'est donc le début du caractère (ou un caractère ASCII simple), on arrête là
      if ((c & 0xC0) != 0x80) break;
    }
  }
  
  auto& console_text = _pRegistry->get<Text>(console_entity);

  // on met à jour l'affichage du text de la console
  if (_buffer != temp_buffer)
  {
    console_text.text = " > " + _buffer;
    auto& consoleTextMesh = _pRegistry->get<TextMesh>(console_entity);
    UpdateTextMesh(consoleTextMesh, console_text);
  }


  // on crée l'entité qui affichera l'historique de la console si l'ancienne entité n'est plus valide
  // sinon on remets son timer à zéro
  // on considère qu'à partir d'ici l'entité historique est valide
  if (_pRegistry->valid(_historyEntity))
  {
    resetHistoryTimer(HISTORY_TIMER_TIME, false);
  }
  else 
  {
    _historyEntity = _pRegistry->create();
    _pRegistry->emplace<Timer>(_historyEntity, 
      Timer{
        .isActive = false, 
        .time = HISTORY_TIMER_TIME
      });
    auto& text = _pRegistry->emplace<Text>(_historyEntity, Text{
      .text = getHistoryAsText(),
      .pFont = &_font,
      .fontSize = CONSOLE_FONT_SIZE,
      .position = {10.0f, (float) game_context.screenInfo.height - HISTORY_Y_OFFSET, 0.0f}
    });
    _pRegistry->emplace<TextMesh>(_historyEntity, TextMesh{ CreateTextMesh(text) });
  }


  if (input_handler.IsKeyPressed(SDLK_RETURN))
  {
    if (!_buffer.empty())
    {
      std::cout << "[Console] processing [" << _buffer << "]\n";

      std::vector<std::string> tokens;
      std::stringstream ss(_buffer);
      std::string word;

      // on fait juste du parsing
      while (std::getline(ss, word, ' ')) 
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

          // dans le cas ou Execute est appelé ailleurs de dans la classe Engine, les autres classes n'ont pas accès à DevConsole
          auto& console_context = _pRegistry->ctx().emplace<ConsoleContext>();
          if (console_context.historyBuffer.contains(name)) 
          {
            UpdateHistory(console_context.historyBuffer.at(name));
            console_context.historyBuffer.erase(name);
          }

          if (!does_cmd_exist) UpdateHistory("!> " + CMD_NAME + " doesn't exist");
        }
      }

      UpdateHistory("~> " + _buffer);

      _buffer.clear();
    }

    console_text.text.clear();
    auto& console_text_mesh = _pRegistry->get<TextMesh>(console_entity);
    UpdateTextMesh(console_text_mesh, console_text);

    dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
  }
  else if (input_handler.IsKeyPressed(SDLK_ESCAPE)) 
  {
    _buffer.clear();

    console_text.text.clear();
    auto& consoleTextMesh = _pRegistry->get<TextMesh>(console_entity);
    UpdateTextMesh(consoleTextMesh, console_text);

    resetHistoryTimer(0.0, true);

    dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
  }
}


void DevConsole::OnResize(const ResizeEvent& e)
{  
  auto console_entity = GetUnique<Console>(*_pRegistry);
  // mise à jour de l'affichage du buffer pour qu'il soit toujours au même endroit dans la fenêtre
  if (_pRegistry->valid(console_entity))
  {
    auto& console_text = _pRegistry->get<Text>(console_entity);
    console_text.position.y = (float) e.height - BUFFER_Y_OFFSET;
    auto& console_text_mesh = _pRegistry->get<TextMesh>(console_entity);
    UpdateTextMesh(console_text_mesh, console_text);
  }

  // pareil pour l'historique s'il est encore affiché
  if (_pRegistry->valid(_historyEntity))
  {
    auto& history_text = _pRegistry->get<Text>(_historyEntity);
    history_text.position.y = (float) e.height - HISTORY_Y_OFFSET;
    auto& history_text_mesh = _pRegistry->get<TextMesh>(_historyEntity);
    UpdateTextMesh(history_text_mesh, history_text);
  }
}


void DevConsole::UpdateHistory(const std::string& buffer)
{
  _history.push_front(buffer);
  
  if (_history.size() > NUM_COMMAND_HISTORY)
    _history.pop_back();

  auto& history_text = _pRegistry->get<Text>(_historyEntity);
  history_text.text = getHistoryAsText();
  UpdateTextMesh(_pRegistry->get<TextMesh>(_historyEntity), history_text);
  resetHistoryTimer(HISTORY_TIMER_TIME, true);
}


std::string DevConsole::getHistoryAsText() const
{
  std::string buffer;

  for (const auto& buf: _history) buffer += buf + "\n";

  return buffer;
}


void DevConsole::resetHistoryTimer(double time, bool isActive)
{
  auto& timer = _pRegistry->get<Timer>(_historyEntity);
  timer.time = time;
  timer.isActive = isActive;
}