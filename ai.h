#include "types.h"
#include "eval.h"
#include "main.h"

#ifndef AI_H
#define AI_H
//bool botMove(Coord *coord1, Coord *coord2, Boardstate bs);
bool calculateBestMove(Boardstate bs, bool isBlack, int depth, Coord *coord1, Coord *coord2);
struct MoveStack* moveablePieces(Boardstate bs, bool isBlack);
bool negaMax(int depth, float score, bool isBlack, Boardstate bs, Coord *coord1, Coord *coord2);
#endif
