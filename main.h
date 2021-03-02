#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAIN_H
#define MAIN_H

#include "types.h"
#include "moves.h"
#include "socket.h"
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

void strrep(char *s, char pre, char post); // replaces chars pre with post in the string s. Returns s
void strrm(char *s, char rm); // removes all instances of character rm in string s
void setBitBoard(); // sets the bitboard for a standard chess game
Boardstate makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying); // makes a boardstate, if NULL is given for bitboard it makes an empty one. Returns the new boardstate
Boardstate *cpyBoardstate(Boardstate *to, Boardstate from); // copies boardstate and everything inside. Returns a pointer to the new boardstate
Board calculateAttackVectors(Board *bitboard, bool blackplaying); //returns an attack vector for a colour on given board
Board calculateMovementVector(Boardstate bs, Coord pos); // returns a board of every square the piece at pos can move to
U8 iterateVector(Boardstate bs, Board fromvector, Board tovector, Coord *co, U8 pass); // iterates through the fromvector trying to move pieces to the tovector according to what pieces they would be on the boardstate. Pass tells it how many times to go, 1 or 2. Returns a count of how many matches were found
void printBoard(Boardstate bs); // prints off the bitboard according to the internal player boolean
void printHighlightBoard(Boardstate bs, Board highlights); // prints a board but with highlighted features based on the Board given
U8 readInput(char *s, U8 strsize); //reads input and clears stdin. Returns the number of chars read in
bool parseInput(char *s, Coord *from, Coord *to); //parses the string s according to algebraic notation and returns from and to as read in from s. Returns a boolean whether it can successfully parse or not
bool movePiece(Coord from, Coord to); // returns true if the piece moved from [from] to [to], assumes currBoard
bool fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs); // returns a board nbs as if the move had been executed, if its an illegal move nbs is set as bs. Boolean represents whether it was successful or not
char findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece. takes coordinate (0-63)
Coord btoc(Board b); // Board to Coord, returns the smallest coord with a piece on it on that board
Board getPlayerBoard(Board *bitboard, bool blackplaying); // returns the combined board for the player given
bool inCheck(Boardstate bs); // returns a boolean for whether or not the player whose turn it is is in check
bool inCheckMate(Boardstate bs); // returns a boolean for whether or not the player whose turn it is is mate'd
bool inStaleMate(Boardstate bs); // returns a boolean for whether or not the player whose turn it is is stalemate'd

#ifdef DEBUG // used in debugging
void debugPrintBoard(Board b); // prints a whole board as just bits
void printBits(U8 byte); // prints a U8 as bits
#endif

#endif
