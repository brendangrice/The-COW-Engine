#include "main.h"
#include "types.h"
#include "pgn.h"

#ifndef GAMEMODES_H
#define GAMEMODES_H

void localMultiplayer(Boardstate *currBoard); // function for running the multiplayer
void onlineMultiplayer(Boardstate *currBoard); // function for running the multiplayer
void onlineMultiplayerHosting(Boardstate *currBoard); // function for running the multiplayer
#endif // GAMEMODES_H
