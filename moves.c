#include "moves.h"

//TODO COMBINE PAWNMOVEMENT AND ATTACK, MAKING THEM WORK OFF ONE FUNCTION BASED ON COLOUR
//TODO #DEFINE FLAGS FOR PAWN MOVEMENTS AND CASTLING
//TODO PUT THESE FUNCTIONS IN ARRAYS FOR EASY LOOKUP

U8
whitePawnMovement(Coord from, Coord to, Boardstate bs, Board *vector)
{
	Board extra;
	if (vector==NULL) vector = &extra;
	*vector = 0;
	U64 p = 1ULL;
	Board attack = whitePawnAttackVectors(from);
	*vector |= attack&bs.bitboard[black]; // where the piece can attack
	*vector |= (~bs.bitboard[total])&(p<<(from+8)); // if there isn't a piece in front it can move
	if (bs.movementflags&8) *vector |= attack&((p<<40)<<(bs.movementflags&7)); // where the pawn can attack and en passant
	if ((from>>3)==1 && !(bs.bitboard[total]&p<<(from+8))) *vector |= (~bs.bitboard[total])&(p<<(from+16)); //must be on the second file and no piece in front of it by one or two spaces, need to update flags too
	// figure out what sort of move was made
	if (to==40+(bs.movementflags&7) && *vector&p<<to) return 2; // en passant
	if (to==from+16 && *vector&p<<to) return 3; // double move, update flags
	if (*vector&p<<to) return 1; // regular movement
	return false; // can't move
}

U8
blackPawnMovement(Coord from, Coord to, Boardstate bs, Board *vector) 
{	
	Board extra;
	if (vector==NULL) vector = &extra;
	*vector = 0;
	U64 p = 1ULL;

	Board attack = blackPawnAttackVectors(from);
	*vector |= attack&(bs.bitboard[total]^bs.bitboard[black]); //where the pawn can normally attack
	*vector |= (~bs.bitboard[total])&(p<<(from-8)); // if there isn't a piece in front it can move
	if (bs.movementflags&8) *vector |= attack&((p<<16)<<(bs.movementflags&7)); // where the pawn can attack and en passant
	if ((from>>3)==6 && !(bs.bitboard[total]&(p<<(from-8)))) *vector |= (~bs.bitboard[total])&(p<<(from-16)); // must be on the second file and no piece one or two squares in front of it, need to update flags too
	if (to==16+(bs.movementflags&7) && *vector&p<<to) return 2; // en passant
	if (to==from-16 && *vector&p<<to) return 3; // double move, update flags
	if (*vector&p<<to) return 1; // regular movement
	return false; // can't move
}


bool 
rookMovement(Coord from, Coord to, Boardstate bs, Board *vector) //can move to wherever it can attack
{	
	Board extra;
	if (vector==NULL) vector = &extra;
	*vector = rookAttackVectors(from, bs.bitboard)&~getPlayerBoard(bs.bitboard, bs.blackplaying); // find attack vectors of piece and make sure it only intersects with the enemy team/empty squares
	return (*vector&1ULL<<to)>>to;
}

bool 
knightMovement(Coord from, Coord to, Boardstate bs, Board *vector) //can move to wherever it can attack
{
	Board extra;
	if (vector==NULL) vector = &extra;
	*vector = knightAttackVectors(from)&~getPlayerBoard(bs.bitboard, bs.blackplaying); //shouldn't be able to attack your own pieces
	return (*vector&1ULL<<to)>>to;
}

bool 
bishopMovement(Coord from, Coord to, Boardstate bs, Board *vector) //can move to wherever it can attack
{
	Board extra;
	if (vector==NULL) vector = &extra;
	*vector = bishopAttackVectors(from, bs.bitboard)&~getPlayerBoard(bs.bitboard, bs.blackplaying); //should only be able to attack the enemy 
	return (*vector&1ULL<<to)>>to;
}

bool 
queenMovement(Coord from, Coord to, Boardstate bs, Board *vector) //can move to wherever it can attack
{
	Board extra;
	if (vector==NULL) vector = &extra;
	*vector = queenAttackVectors(from, bs.bitboard)&~getPlayerBoard(bs.bitboard, bs.blackplaying); //should only be able to attack the enemy
	return (*vector&1ULL<<to)>>to;
}

U8 
kingMovement(Coord from, Coord to, Boardstate bs, Board *vector) //can move to wherever it can attack + castling
{
	Board extra;
	if (vector==NULL) vector = &extra;
	
	*vector = kingAttackVectors(from)&~getPlayerBoard(bs.bitboard, bs.blackplaying); // can't attack your own pieces
	U64 p = 1ULL;

	//castling
	// find which castle it is and whether it can happen
	U8 result = 0;
	U64 l1 = 0x7000000000000000;
	U64 l2 = 0x3800000000000000;
	U64 l3 = 0x0600000000000000;
	U64 l4 = 0x0E00000000000000;

	Board blackattack = calculateAttackVectors(bs.bitboard, true);
	Board whiteattack = calculateAttackVectors(bs.bitboard, false);

	if ((from>>3)==0 && bs.movementflags&0x80) // White left
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&0x70)&&!(blackattack&0x38)) {
		       	*vector |= p<<5;
			if (to==0x05) result = 2;
		}
	if ((from>>3)==0 && bs.movementflags&0x40) // White right
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&0x06)&&!(blackattack&0x0E)) {
			*vector |= p<<1;
			if (to==0x01) result = 3;
		}
	if ((from>>3)==7 && bs.movementflags&0x20) // Black left
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&l1)&&!(whiteattack&l2)) {
			*vector |= p<<0x3D;	
			if (to==0x3D) result = 4;
		}
	if ((from>>3)==7 && bs.movementflags&0x10) // Black right
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&l3)&&!(whiteattack&l4)) {
			*vector |= p<<0x39;
			if (to==0x39) result = 5;
		}

	if (result) return result;
	return (*vector&1ULL<<to)>>to; // king can move normally
}

Board 
whitePawnAttackVectors(Coord pos) // pawns can attack diagonally
{
	U64 vector = 0;
	U64 p = 1ULL;
	p<<=8+pos; // forward one square
	if (pos%8 != 0) vector ^= p>>1; // left
	if (pos%8 != 7) vector ^= p<<1; // right
	return vector; // this breaks on the last row but pawns promote there anyway
}

Board 
blackPawnAttackVectors(Coord pos) // pawns can attack diagonally
{
	U64 vector = 0;
	U64 p = 1ULL;
	p<<=pos-8; // back one square
	if (pos%8 != 0) vector ^= p>>1;
	if (pos%8 != 7) vector ^= p<<1;
	return vector; // this breaks on the first row but pawns promote there anyway
}

Board 
rookAttackVectors(Coord pos, Board *bitboard) // makes 4 boards and puts them together
{
	Board vector1 = 0;
	Board vector2 = 0;
	Board vector3 = 0;
	Board vector4 = 0;
	U64 p = 1ULL;
	for (int i = pos+8; i < 64; i+=8) {
		vector1 |= p<<i; //NE
		if (vector1&bitboard[total]) break; // stops when there's something in the way
	}
	for (int i = pos-8; i > -1; i-=8) {
		vector2 |= p<<i; //NE 
		if (vector2&bitboard[total]) break;
	}
	for (int i = pos+1; i%8; i++) {
		vector3 |= p<<i; //NE 
		if (vector3&bitboard[total]) break;
	}
	for (int i = pos-1; i%8!=7 && i>-1; i--) {
		vector4 |= p<<i; //NE 
		if (vector4&bitboard[total]) break;
	}
	return vector1|vector2|vector3|vector4;
}

Board 
knightAttackVectors(Coord pos)
{
	Board vector = 0;
	U64 p = 1ULL;
	U8 ldist = 7-(pos%8);
	U8 rdist = pos%8;
	U8 tdist = 7-(pos>>3);
	U8 bdist = pos>>3;
	if (rdist>0 && tdist>1) vector |= p<<(pos+15); // NNW
	if (rdist>1 && tdist>0) vector |= p<<(pos+6); // WNW
	if (rdist>1 && bdist>0) vector |= p<<(pos-10); // WSW
	if (rdist>0 && bdist>1) vector |= p<<(pos-17); // SSW
	if (ldist>0 && bdist>1) vector |= p<<(pos-15); // SSE
	if (ldist>1 && bdist>0) vector |= p<<(pos-6); // ESE
	if (ldist>1 && tdist>0) vector |= p<<(pos+10); // ENE
	if (ldist>0 && tdist>1) vector |= p<<(pos+17); // NNE
	return vector;
}

Board 
bishopAttackVectors(Coord pos, Board *bitboard) // make 4 boards and puts them together
{
	Board vector1 = 0;
	Board vector2 = 0;
	Board vector3 = 0;
	Board vector4 = 0;
	U64 p = 1ULL;
	for (int i = pos+7; i < 64 && i%8!=7; i+=7) {
		vector1 |= p<<i; //NE 
		if (vector1&bitboard[total]) break; //stops when there's something in the way
	}
	for (int i = pos-9; i > -1 && i%8!=7; i-=9) {
		vector2 |= p<<i; //SE 
		if (vector2&bitboard[total]) break;
	}
	for (int i = pos-7; i > -1 && i%8; i-=7) { 
		vector3 |= p<<i; //SW
		if (vector3&bitboard[total]) break;
	}
	for (int i = pos+9; i < 64 && i%8; i+=9) { 
		vector4 |= p<<i; //NW
		if (vector4&bitboard[total]) break;
	}
	return vector1|vector2|vector3|vector4;
}

Board 
queenAttackVectors(Coord pos, Board *bitboard)
{
	return rookAttackVectors(pos, bitboard)|bishopAttackVectors(pos, bitboard); //simply works the same as if a bishop and rook were on the same piece
}

Board 
kingAttackVectors(Coord pos)
{
	U64 vector = 0;
	U64 p = 1ULL;
	U8 ldist = 7-(pos%8);
	U8 rdist = pos%8;
	U8 tdist = 7-(pos>>3);
	U8 bdist = pos>>3;
	
	if (tdist>0) vector|=p<<(pos+8); //N
	if (rdist>0) vector|=p<<(pos-1); //E
	if (bdist>0) vector|=p<<(pos-8); //S
	if (ldist>0) vector|=p<<(pos+1); //W

	// corners
	// if top and right are true then so is top right, etc.
	vector |= ((vector&(p<<(pos+8)))>>1)&((vector&(p<<(pos-1)))<<8); 
	vector |= ((vector&(p<<(pos-1)))>>8)&((vector&(p<<(pos-8)))>>1);
	vector |= ((vector&(p<<(pos-8)))<<1)&((vector&(p<<(pos+1)))>>8);
	vector |= ((vector&(p<<(pos+1)))<<8)&((vector&(p<<(pos+8)))<<1);

	return vector;
}
