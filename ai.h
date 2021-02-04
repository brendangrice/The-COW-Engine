#include "types.h"
#include "eval.h"
#include "main.h"

#ifndef AI_H
#define AI_H
bool botMove(Coord *coord1, Coord *coord2, Boardstate bs);

bool negaMax(int depth, float score, bool isBlack, Boardstate bs, Coord *coord1, Coord *coord2);
#endif
