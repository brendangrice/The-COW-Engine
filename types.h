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

typedef struct {
	Board *bitboard;
	U8 movementflags;
	bool blackplaying;
} Boardstate;

#endif
