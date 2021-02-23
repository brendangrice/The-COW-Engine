#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAIN_H
#define MAIN_H

#include "types.h"
#include "moves.h"
#include "eval.h"
#include "ai.h"

#include "tt.h"


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

Boardstate makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying); // makes a boardstate, pass (NULL, 0, 0) for an empty
Boardstate *cpyBoardstate(Boardstate *to, Boardstate from); // copies data across Boardstates
void localAI(); // function for running the ai
void localMultiplayer(); // function for running the multiplayer
Board calculateAttackVectors(Board *bitboard, bool blackplaying); //returns an attack vector for a colour on given board
void printBoard(Boardstate bs); // parameter determines which way the board prints
void printFEN(Boardstate bs, Coord from,Coord to); // print FEN of given Boardstate, Coord used for en passent
bool parseInput(Coord *from, Coord *to); //reads input and gives from and to as coordinates (0-63)
bool movePiece(Coord from, Coord to); // returns true if the piece moved from [from] to [to], colour of the piece (players turn) needs to be given
bool fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs); // returns a board nbs as if the move had been executed, if its an illegal move the given Boardstate is returned.
char findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece. takes coordinate (0-63)
Coord btoc(Board b); // Board to Coord, returns the smallest coord with a piece on it on that board
Board getPlayerBoard(Board *bitboard, bool blackplaying); // returns the combined board for the player given
bool inCheck(Boardstate bs); // returns a boolean for whether or not the player is in check
bool inCheckMate(Boardstate bs); // returns a boolean for whether or not the player is mate'd
bool inStaleMate(Boardstate bs);

#ifdef DEBUG
void debugPrintBoard(Board b);
void printBits(U8 byte); // used in debugging
#endif

#endif