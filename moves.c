#include "moves.h"

bool 
whitePawnMovement(Coord from, Coord to, Boardstate bs)
{
	U64 p = 1LL;;

	if (bs.movementflags&7 && (from>>3)==4 && ((to-9==from)|(to-7==from)) && to==40+(bs.movementflags&0x7)) return 2;

	if (whitePawnAttackVectors(from)&p<<to&bs.bitboard[black]) return true; // taking
	if (bs.bitboard[total]&p<<(from+8)) return false; // if there's something in front of the pawn it can't move
	if (to-from==16 && (from>>3)==1 && ~bs.bitboard[total]&p<<(from+16)) {
		bs.movementflags&=0xF0;
		bs.movementflags|=from%8;
		bs.movementflags|=0x8;
	       	return true; // moving 2 iff the pawn hasn't mvoed before and there isn't a piece there
	}
	return (to-from)==8; // normal movement
}

bool 
blackPawnMovement(Coord from, Coord to, Boardstate bs) 
{
	U64 p = 1LL;

	if (bs.movementflags&7 && (from>>3)==3 && ((from-9==to)|(from-7==to)) && to==16+(bs.movementflags&0x7)) return 2; // en passant

	if (blackPawnAttackVectors(from)&p<<to&(bs.bitboard[black]^bs.bitboard[total])) return true; // taking
	if (bs.bitboard[total]&p<<(from-8)) return false; // if there's something in front of the pawn it can't move
	if (from-to==16 && (from>>3)==6 && ~bs.bitboard[total]&p<<(from-16)) {
		bs.movementflags&=0xF0;
		bs.movementflags|=from%8;
		bs.movementflags|=0x8;
	       	return true; // moving 2 iff the pawn hasn't moved before and there isn't a piece there
	}	
	return (from-to)==8; // normal movement
}

bool 
rookMovement(Coord from, Coord to, Board *bitboard) //can move to wherever it can attack
{
	return (rookAttackVectors(from, bitboard)&1LL<<to)>>to;
}

bool 
knightMovement(Coord from, Coord to) //can move to wherever it can attack
{
	return (knightAttackVectors(from)&1LL<<to)>>to;
}

bool 
bishopMovement(Coord from, Coord to, Board *bitboard) //can move to wherever it can attack
{
	return (bishopAttackVectors(from, bitboard)&1LL<<to)>>to;
}

bool 
queenMovement(Coord from, Coord to, Board *bitboard) //can move to wherever it can attack
{
	return (queenAttackVectors(from, bitboard)&1LL<<to)>>to;
}

bool 
kingMovement(Coord from, Coord to, Boardstate bs) //can move to wherever it can attack + castling
{
	//castling
	// find which castle it is and whether it can happen
	
	U64 l1 = 0x7000000000000000;
	U64 l2 = 0x3800000000000000;
	U64 l3 = 0x0600000000000000;
	U64 l4 = 0x0E00000000000000; 

	Board blackattack = calculateAttackVectors(bs.bitboard, true);
	Board whiteattack = calculateAttackVectors(bs.bitboard, false);

	if ((from>>3)==0 && bs.movementflags&0x80 && to==0x05) // White left
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&0x70)&&!(blackattack&0x38)) return 2;
	if ((from>>3)==0 && bs.movementflags&0x40 && to==0x01) // White right
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&0x06)&&!(blackattack&0x0E)) return 3;
	if ((from>>3)==7 && bs.movementflags&0x20 && to==0x3D) // Black left
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&l1)&&!(whiteattack&l2)) return 4;
	if ((from>>3)==7 && bs.movementflags&0x10 && to==0x39) // Black right
		// Check squares are empty and not threatened
		if (!(bs.bitboard[total]&l3)&&!(whiteattack&l4)) return 5;

	return (kingAttackVectors(from)&1LL<<to)>>to;
}

Board 
whitePawnAttackVectors(Coord pos) // pawns can attack diagonally
{
	U64 vector = 0;
	U64 p = 1LL;
	p<<=8+pos; // forward one square
	if (pos%8 != 0) vector ^= p>>1; // left
	if (pos%8 != 7) vector ^= p<<1; // right
	return vector; // this breaks on the last row but pawns promote there anyway
}

Board 
blackPawnAttackVectors(Coord pos) // pawns can attack diagonally
{
	U64 vector = 0;
	U64 p = 1LL;
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
	U64 p = 1LL;
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
	U64 p = 1LL;
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
	U64 p = 1LL;
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
	U64 p = 1LL;
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
