#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef MAIN_H
#define MAIN_H

#define MAXTURNS 500 // maximum turns expected (half turns)

#include "types.h"
#include "argp.h"
#include "moves.h"

#include "eval.h"
#include "ai.h"

#include "pgn.h"
#include "fen.h"
#include "gamemodes.h"
#include "tt.h"


// Should attack boards be tracked like this
// when they need to constantly be re-evaluated anyways?
// mostly used for debugging

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
void setBoardstate(); // sets the bitboard for a standard chess game
Boardstate makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying, U16 halfmove, U16 fiftymove); // makes a boardstate, if NULL is given for bitboard it makes an empty one. Returns the new boardstate
Boardstate *cpyBoardstate(Boardstate *to, Boardstate from); // copies boardstate and everything inside. Returns a pointer to the new boardstate
Board calculateAttackVectors(Board *bitboard, bool blackplaying); //returns an attack vector for a colour on given board
Board calculateMovementVector(Boardstate bs, Coord pos); // returns a board of every square the piece at pos can move to
U8 iterateVector(Boardstate bs, Board fromvector, Board tovector, Coord *co, U8 pass); // iterates through the fromvector trying to move pieces to the tovector according to what pieces they would be on the boardstate. Pass tells it how many times to go, 1 or 2. Returns a count of how many matches were found
void printBoard(Boardstate bs); // prints off the bitboard according to the internal player boolean
void printHighlightBoard(Boardstate bs, Board highlights); // prints a board but with highlighted features based on the Board given
void prettyPrintBoard(Boardstate bs); // print large board with defined squares
U8 readInput(char *s, U8 strsize); //reads input and clears stdin. Returns the number of chars read in
U8 parseInput(char *s, Coord *from, Coord *to); //parses the string s according to algebraic notation and returns from and to as read in from s. Returns a boolean whether it can successfully parse or not
U8 defaultPromotion();
U8 getPromotion();
bool movePiece(Coord from, Coord to, bool PGN, U8 (*promote)()); // returns true if the piece moved from [from] to [to], assumes currBoard takes function for promotion, default with NULL
bool fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs, U8(*promote)()); // returns a board nbs as if the move had been executed, if its an illegal move nbs is set as bs. Boolean represents whether it was successful or not
char findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard); //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece. takes coordinate (0-63)
Coord btoc(Board b); // Board to Coord, returns the smallest coord with a piece on it on that board
Coord atoc(char l, char n); // Array to Coord, returns the Coord of the respective algebraic notation H 1 -> 0. Out of bounds returns 255
char *ctoa(char *s, Coord c); // Coord to Array, returns an Algebraic representation of the Coord;
int popcnt(Board b); // count number of set bits on a given board
Board getPlayerBoard(Board *bitboard, bool blackplaying); // returns the combined board for the player given
bool inCheck(Boardstate bs); // returns a boolean for whether or not the player whose turn it is is in check
bool inCheckMate(Boardstate bs); // returns a boolean for whether or not the player whose turn it is is mate'd
bool inStaleMate(Boardstate bs); // returns a boolean for whether or not the player whose turn it is is stalemate'd

#ifdef DEBUG
void debugPrintBoard(Board b);
void printBits(U8 byte); // used in debugging
#endif

#endif // MAIN_H
