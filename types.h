#include "main.h"

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char U8; // unsigned 8 bit
typedef unsigned short U16; // unsigned 16 bit

#if UINT_MAX == 4294967295U // 32bit vs 64bit
typedef unsigned int U32; // unsigned 32 bit
#else
typedef unsigned long U32; // unsigned 32 bit
#endif

typedef unsigned long long U64; // unsigned 64 bit

typedef U8 bool; // boolean

enum {false, true}; // true and false

typedef U8 Coord; //always between 0-63 to fit on the board, helps with legibility and intent

typedef U64 Board; //used for representing bitboards, helps with legibility and intent

#define BOARDSIZE 8 // 8x8 Square
#define CHESSPIECES 6 // white and black
#define PLAYERS 2 // number of players

#define BITBOARDELEMENTS (CHESSPIECES+(PLAYERS-1)+1) // length of enum
#define BITBOARDSIZE (BITBOARDELEMENTS*sizeof(Board)) // one Board per element
// Bitboards for each piece, 1 for every player past one, 1 more for the total board. 
#define BOARDSTATESIZE (sizeof(Boardstate))
//maybe make these sizeof's into const variables to reduce the number of times sizeof is used?

typedef struct { // general object for tracking the game
	Board bitboard[BITBOARDELEMENTS]; // tracks the board and pieces
	U8 movementflags; // tracks en passant and castling
	bool blackplaying; // tracks which player's turn it is
	U16 halfmove; // counts the number of half moves made
	U16 fiftymove; // tracks the last time a move was made that interacts with the fifty move rule
	U64 key;
} Boardstate;

typedef struct { // unused
    U64 key;
    int depth;
    int flag; // 0 = exact, 1 = alpha, 2 = beta
    float score; 
} TransitionTable;

struct MoveStack { //used for ai
	U8 data;
	struct MoveStack* next;
};

struct arguments { // used for argument parsing
	char *arg; // input string
	bool all; // go thorugh all boards
	bool pgn; // parse a pgn string to play out a board
	bool step; // step through boards
	bool print; // print boards that are gone through 
	bool header; // print the PGN header
	bool fenprint; // print FEN string
	bool fen; // parse a FEN string to set up the board
	char *output_file; // new file to write pgn out to
	char *gamemode;
};

#endif
