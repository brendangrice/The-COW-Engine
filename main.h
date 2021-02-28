#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAIN_H
#define MAIN_H

#include "types.h"
#include "moves.h"
#include "pgn.h"
#include "gamemodes.h"

// Should attack boards be tracked like this
// when they need to constantly be re-evaluated anyways?
// mostly used for debugging

#define BOARDSIZE 8 // 8x8 Square
#define CHESSPIECES 6 // white and black
#define PLAYERS 2 // number of players
#define BITBOARDELEMENTS (CHESSPIECES+(PLAYERS-1)+1) // length of enum
#define BITBOARDSIZE (BITBOARDELEMENTS*sizeof(Board)) // one Board per element
// Bitboards for each piece, 1 for every player past one, 1 more for the total board. 
#define BOARDSTATESIZE (BITBOARDSIZE*sizeof(Board)+sizeof(U8))
//maybe make these sizeof's into const variables to reduce the number of times sizeof is used?


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

extern const U8 ltoe[]; // letter to enum, e.g. 'N' -> knight, tentative global

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

void setBitBoard(); // sets the bitboard for a standard chess game
Boardstate makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying);
Boardstate *cpyBoardstate(Boardstate *to, Boardstate from);
Board calculateAttackVectors(Board *bitboard, bool blackplaying); //returns an attack vector for a colour on given board
Board calculateMovementVector(Boardstate bs, Coord pos); // returns a board of every square the piece at pos can move to
U8 iterateVector(Boardstate bs, Board fromvector, Board tovector, Coord *co, U8 pass);
void printBoard(Boardstate bs); // parameter determines which way the board prints
void printHighlightBoard(Boardstate bs, Board highlights); // prints a board but with highlighted features based on the Board given
U8 readInput(char *s, U8 strsize); //reads input and gives from and to as coordinates (0-63), returns -1 on quit, 0 on bad read, 1 on the first term being fully read, 2 on both terms being read 
U8 parseInput(char *s, Coord *from, Coord *to);
bool movePiece(Coord from, Coord to); // returns true if the piece moved from [from] to [to], colour of the piece (players turn) needs to be given
bool fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs); // returns a board nbs as if the move had been executed, if its an illegal move the given Boardstate is returned.
char findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece. takes coordinate (0-63)
Coord btoc(Board b); // Board to Coord, returns the smallest coord with a piece on it on that board
Board getPlayerBoard(Board *bitboard, bool blackplaying); // returns the combined board for the player given
bool inCheck(Boardstate bs); // returns a boolean for whether or not the player is in check
bool inCheckMate(Boardstate bs); // returns a boolean for whether or not the player is mate'd
bool inStaleMate(Boardstate bs); // returns a boolean for whether or not the player is stalemate'd

#ifdef DEBUG
void debugPrintBoard(Board b);
void printBits(U8 byte); // used in debugging
#endif

#endif
