#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SHIT_CHESS_H
#define SHIT_CHESS_H

#include "types.h"
#include "moves.h"


enum {
	pawn,
	rook,
	knight,
	bishop,
	queen,
	king,
	black,
	whiteAttack,
	blackAttack,
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

U64 calculateAttackVectors(bool black); //returns an attack vector for a colour
void printBoard();
void printBit(U8 byte);
bool parseInput(U8 *from, U8 *to); //reads input and gives from and to as coordinates (0-64)
bool movePiece(U8 from, U8 to, bool moveblack); //returns true if it moved the piece
char findPiece(U8 pos, U8 *piece, bool *colourblack); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece.

#endif
