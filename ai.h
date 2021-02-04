#include "types.h"
#include "eval.h"
#include "shit_chess.h"

bool botMove(Coord *coord1, Coord *coord2, Boardstate bs);

bool negaMax(int depth, float score, bool isBlack, Boardstate bs, Coord *coord1, Coord *coord2);
