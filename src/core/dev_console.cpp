#include "core/dev_console.h"


#include <SDL3/SDL_keycode.h>
#include <iostream>

#include "core/game_context.h"
#include "events/game_state_change_event.h"
#include "loaders/font_loader.h"
#include "utils/create_text_mesh.h"
#include "utils/game_state.h"
#include "components/text.h"
#include "components/text_mesh.h"
#include "components/tags.h"
#include "components/timer.h"


DevConsole::DevConsole(entt::registry* registry)
  : _pRegistry(registry)
{
  _entity = _pRegistry->create();
  _buffer = "";
}

DevConsole::~DevConsole() {}


bool DevConsole::Init()
{
  _font = LoadFont("roboto_mono");

  auto& game_ctx = _pRegistry->ctx().get<GameContext>();
  
  _pRegistry->emplace<Console>(_entity);
  auto& text = _pRegistry->emplace<Text>(_entity, Text{
    .text = "",
    .pFont = &_font,
    .fontSize = CONSOLE_FONT_SIZE,
    .position = {10.0f, (float) game_ctx.screenInfo.height - BUFFER_Y_OFFSET, 0.0f}
  });
  _pRegistry->emplace<TextMesh>(_entity, CreateTextMesh(text));

  return true;
}


void DevConsole::Update()
{
  auto& game_ctx = _pRegistry->ctx().get<GameContext>();
  
  if (game_ctx.currentState != GameState::CONSOLE) return; // pas besoin d'update la console, on ne l'utilise pas
  
  // on modifie le buffer de la console avant de générer le text qui sera affiché
  std::string temp_buffer = "> " + _buffer;
  _buffer += game_ctx.inputHandler.GetTextInput();
  if (game_ctx.inputHandler.IsKeyPressed(SDLK_BACKSPACE) && !_buffer.empty()) _buffer.pop_back();
  
  auto& console_text = _pRegistry->get<Text>(_entity);

  // on met à jour l'affichage du text de la console
  if (_buffer != temp_buffer)
  {
    console_text.text = "> " + _buffer;
    auto& consoleTextMesh = _pRegistry->get<TextMesh>(_entity);
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
      .position = {10.0f, (float) game_ctx.screenInfo.height - HISTORY_Y_OFFSET, 0.0f}
    });
    _pRegistry->emplace<TextMesh>(_historyEntity, TextMesh{ CreateTextMesh(text) });
  }


  if (game_ctx.inputHandler.IsKeyPressed(SDLK_RETURN))
  {
    if (!_buffer.empty())
    {
      // TODO interpréter la commande
    
      std::cout << "[Console] executing [" << _buffer << "] command\n";

      // on enregistre l'historique des commandes puis on efface le buffer pour acceuillir la prochaine commande
      // TODO ajouter le helper de la commande en cas de mauvaise utilisation updateHistory(_buffer + " - " + helper);
      updateHistory("~ " + _buffer);
      auto& history_text = _pRegistry->get<Text>(_historyEntity);
      history_text.text = getHistoryAsText();
      std::cout << history_text.text;
      UpdateTextMesh(_pRegistry->get<TextMesh>(_historyEntity), history_text);

      _buffer.clear();
    }

    console_text.text.clear();
    auto& console_text_mesh = _pRegistry->get<TextMesh>(_entity);
    UpdateTextMesh(console_text_mesh, console_text);

    resetHistoryTimer(HISTORY_TIMER_TIME, true);

    game_ctx.dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
  }
  else if (game_ctx.inputHandler.IsKeyPressed(SDLK_ESCAPE)) 
  {
    _buffer.clear();

    console_text.text.clear();
    auto& consoleTextMesh = _pRegistry->get<TextMesh>(_entity);
    UpdateTextMesh(consoleTextMesh, console_text);

    resetHistoryTimer(HISTORY_TIMER_TIME, true);

    game_ctx.dispatcher.enqueue<GameStateChangeEvent>({.newState = GameState::IN_GAME});
  }
}


void DevConsole::OnResize(const ResizeEvent& e)
{
  // mise à jour de l'affichage du buffer pour qu'il soit toujours au même endroit dans la fenêtre
  if (_pRegistry->valid(_entity))
  {
    auto& console_text = _pRegistry->get<Text>(_entity);
    console_text.position.y = (float) e.height - BUFFER_Y_OFFSET;
    auto& console_text_mesh = _pRegistry->get<TextMesh>(_entity);
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


void DevConsole::updateHistory(const std::string& buffer)
{
  _history.push_front(buffer);
  
  if (_history.size() > NUM_COMMAND_HISTORY)
    _history.pop_back();
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