#ifndef VOXL_GAME_STATE_CHANGE_H
#define VOXL_GAME_STATE_CHANGE_H


#include "utils/game_state.h"


struct GameStateChangeEvent
{
  const char* name = "GAME_STATE_CHANGE_EVENT";
  GameState newState;
};


#endif // !VOXL_GAME_STATE_CHANGE_H