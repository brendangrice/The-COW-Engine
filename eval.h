#include "types.h"
#include "shit_chess.h"

#ifndef EVAL_H
#define EVAL_H
// count number of set bits on a given board
int numberOfBits(Board b);

// value based off of positional advantage
int calculatePositionAdvantage();

// value based off of materia advantage
int calculateMaterialAdvantage();

// final value of total game state advantage
float calculateAdvantage();


void debugPrint(Board board);

#endif
