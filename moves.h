#include "types.h"
#include "shit_chess.h"

#ifndef MOVES_H
#define MOVES_H

// 1 1 1 1 | 1 1 1 1
// White left castle, White right castle
// Black left castle, White right castle
//
// 1 bit represents when it was set 3 bits to represent which pawn has moved twice for en passant

bool whitePawnMovement(U8 from, U8 to);

bool blackPawnMovement(U8 from, U8 to);

bool rookAttackMovement(U8 from, U8 to);

bool knightAttackMovement(U8 from, U8 to);

bool bishopAttackMovement(U8 from, U8 to);

bool queenAttackMovement(U8 from, U8 to);

bool kingAttackMovement(U8 from, U8 to);

U64 whitePawnAttackVectors(U8 pos);

U64 blackPawnAttackVectors(U8 pos);

U64 rookAttackVectors(U8 pos);

U64 knightAttackVectors(U8 pos);

U64 bishopAttackVectors(U8 pos);

U64 queenAttackVectors(U8 pos);

U64 kingAttackVectors(U8 pos);

#endif
