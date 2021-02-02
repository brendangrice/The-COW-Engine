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

#define BOARDSIZE 8 // 8x8 Square
#define CHESSPIECES 6 // white and black
#define PLAYERS 2 // number of players
#define BITBOARDELEMENTS (CHESSPIECES+(PLAYERS-1)+1)
#define BITBOARDSIZE (BITBOARDELEMENTS*sizeof(Board))
// Bitboards for each piece, 1 for every player past one, 1 more for the total board. 
#define BOARDSTATESIZE (BITBOARDSIZE*sizeof(Board)+sizeof(U8))

enum { // all available 'pieces'
	pawn,
	rook,
	knight,
	bishop,
	queen,
	king,
	black,
	total,
	nopiece,
};

/*
 *
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

Board calculateAttackVectors(Board *bitboard, bool black); //returns an attack vector for a colour on given board
void printBoard(Board *bitboard, bool printblack); // parameter determines which way the board prints
bool parseInput(Coord *from, Coord *to); //reads input and gives from and to as coordinates (0-63)
bool movePiece(Coord from, Coord to, bool moveblack); // returns true if the piece moved from [from] to [to], colour of the piece (players turn) needs to be given
bool fauxMove(Coord from, Coord to, bool moveblack, Boardstate bs, Boardstate *nbs); // returns a board nbs as if the move had been executed, if its an illegal move the given Boardstate is returned.
char findPiece(Coord pos, Coord *piece, bool *colourblack, Board *bitboard); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece. takes coordinate (0-63)

#ifdef DEBUG
void debugPrintBoard(Board b);
void printBits(U8 byte); // used in debugging
#endif

#endif
