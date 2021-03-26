#include "main.h"
#include "types.h"
#include "pgn.h"
#include "tt.h"

#ifndef GAMEMODES_H
#define GAMEMODES_H

void localMultiplayer(Boardstate *currBoard); // function for running the multiplayer
void localAI(Boardstate *currBoard); // function for running the ai
#endif // GAMEMODES_H
