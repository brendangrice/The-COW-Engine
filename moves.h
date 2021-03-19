#include "types.h"
#include "main.h"

#ifndef MOVES_H
#define MOVES_H

// movementflags
// 1 1 1 1 | 1 1 1 1
// White left castle, White right castle
// Black left castle, White right castle
//
// 1 bit represents when it was set 3 bits to represent which pawn has moved twice for en passant

// all the movements pieces can make

// pawn can move one, can move two, can promote, can take maybe rename these from 'movement'
// canWhitePawnMove?
// returns 2 for en passant, 3 for double move, returns 1 for a regular valid move
#define MOVES_PAWN_NORMAL 1
#define MOVES_PAWN_EN_PASSANT 2
#define MOVES_PAWN_DOUBLE 3

U8 whitePawnMovement(Coord from, Coord to, Boardstate bs, Board *vector);
U8 blackPawnMovement(Coord from, Coord to, Boardstate bs, Board *vector);

// all of these can move where they can attack
// returns whether the piece can move 'from' to 'to'
// vector is a board representing all the moves a piece can make from there, can be null

bool rookMovement(Coord from, Coord to, Boardstate bs, Board *vector);
bool knightMovement(Coord from, Coord to, Boardstate bs, Board *vector);
bool bishopMovement(Coord from, Coord to, Boardstate bs, Board *vector);
bool queenMovement(Coord from, Coord to, Boardstate bs, Board *vector);

// king also can castle for more movement

#define MOVES_KING_WHITE_QUEEN 2
#define MOVES_KING_WHITE_KING 3
#define MOVES_KING_BLACK_QUEEN 4
#define MOVES_KING_BLACK_KING 5

U8 kingMovement(Coord from, Coord to, Boardstate bs, Board *vector);

// all the different positions a piece can attack

// 101
// 0P0
Board whitePawnAttackVectors(Coord pos, Board *bitboard);

// 0p0
// 101
Board blackPawnAttackVectors(Coord pos, Board *bitboard);

// Ordinal Directions
//      ^
//      |
//    00100
//    00100
// <- 11R11 ->
//    00100
//    00100
//      |
//      V
Board rookAttackVectors(Coord pos, Board *bitboard);

// L and J shapes
// 01010
// 10001
// 00N00
// 10001
// 01010
Board knightAttackVectors(Coord pos, Board *bitboard);

/* 
// Diagonals
// \           /
//   \       /
//     10001
//     01010
//     00B00
//     01010
//     10001
//   /       \
// /            \
*/

Board bishopAttackVectors(Coord pos, Board *bitboard);

// Rook and Bishop movement
/* \     ^     /
//   \   |   /
//     10101
//     01110
//  <- 11Q11 ->
//     01110
//     10101
//   /   |   \
// /     V     \
*/ 

Board queenAttackVectors(Coord pos, Board *bitboard);
// Every Square immediately around the king
// 00000
// 01110
// 01K10
// 01110
// 00000

Board kingAttackVectors(Coord pos, Board *bitboard);

extern bool (*movementVectors[]) (Coord from, Coord to, Boardstate bs, Board *vector);
extern Board (*attackVectors[]) (Coord pos, Board *bitboard);

#endif // MOVES_H
