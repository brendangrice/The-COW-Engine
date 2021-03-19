#include "main.h"
#include "types.h"

#ifndef FEN_H
#define FEN_H

// FEN constants
#define FENBOARDDEFAULT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

Boardstate parseFEN(char *FEN); // returns a board state based off of a given FEN string
void printFEN(Boardstate bs, Coord from,Coord to); // print FEN of given Boardstate, Coord used for en passent

#endif // FEN_H
