#include "types.h"
#include "shit_chess.h"

#ifndef MOVES_H
#define MOVES_H

// 1 1 1 1 | 1 1 1 1
// White left castle, White right castle
// Black left castle, White right castle
//
// 1 bit represents when it was set 3 bits to represent which pawn has moved twice for en passant

// all the movements pieces can make

// pawn can move one, can move two, can promote, can take
bool whitePawnMovement(Coord from, Coord to);

bool blackPawnMovement(Coord from, Coord to);

// all of these can move where they can attack

bool rookAttackMovement(Coord from, Coord to);

bool knightAttackMovement(Coord from, Coord to);

bool bishopAttackMovement(Coord from, Coord to);

bool queenAttackMovement(Coord from, Coord to);

// king also can castle

bool kingAttackMovement(Coord from, Coord to);

// all the different positions a piece can attack

// 101
// 0P0
Board whitePawnAttackVectors(Coord pos);

// 0p0
// 101
Board blackPawnAttackVectors(Coord pos);

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
Board rookAttackVectors(Coord pos);

// L and J shapes
// 01010
// 10001
// 00N00
// 10001
// 01010
Board knightAttackVectors(Coord pos);

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

Board bishopAttackVectors(Coord pos);

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

Board queenAttackVectors(Coord pos);
// Every Square immediately around the king
// 00000
// 01110
// 01K10
// 01110
// 00000

Board kingAttackVectors(Coord pos);

#endif
