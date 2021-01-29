#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SHIT_CHESS_H
#define SHIT_CHESS_H

#include "types.h"
#include "moves.h"

// Should attack boards be tracked like this
// when they need to constantly be re-evaluated anyways?
// mostly used for debugging

enum { // all available boards
	pawn,
	rook,
	knight,
	bishop,
	queen,
	king,
	black,
	whiteattack,
	blackattack,
	total,
	nopiece,
};


#define BOARDSIZE 8 // 8x8 Square
#define CHESSPIECES 6 // white and black
#define PLAYERS 2 // number of players

/*
 *	rnbqkbnr
 *	pppppppp
 *	00000000
 *	00000000
 *	00000000
 *	00000000
 *	PPPPPPPP 
 *	RNBQKBNR 
 *
 */

Board calculateAttackVectors(bool black); //returns an attack vector for a colour
void printBoard(Board b); // parameter is only used in debugging to print a specific board as its bit representation
void printBits(U8 byte); // used in debugging
bool parseInput(Coord *from, Coord *to); //reads input and gives from and to as coordinates (0-63)
bool movePiece(Coord from, Coord to, bool moveblack); //returns true if it moved the piece takes coordinates (0-63)
char findPiece(Coord pos, Coord *piece, bool *colourblack); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece. takes coordinate (0-63)

#endif
