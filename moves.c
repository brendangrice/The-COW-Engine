#include "moves.h"

extern U8 movementFlags; // tracks what rooks and kings have moved
U64 bitboard[CHESSPIECES+(PLAYERS-1)+2+1]; 

bool 
whitePawnMovement(U8 from, U8 to)
{
	U64 p = 1;

	if (movementFlags&7 && (from>>3)==4 && ((to-9==from)|(to-7==from)) && to==40+(movementFlags&0x7)) return 2;

	if (whitePawnAttackVectors(from)&p<<to&bitboard[black]) return true; // taking
	if (bitboard[total]&p<<(from+8)) return false; // if there's something in front of the pawn it can't move
	if (to-from==16 && (from>>3)==1) {
		movementFlags&=0xF0;
		movementFlags|=from%8;
		movementFlags|=0x8;
	       	return true; // moving 2 iff the pawn hasn't mvoed before and there isn't a piece there
	}
	return (to-from)==8; // normal movement
}

bool 
blackPawnMovement(U8 from, U8 to) 
{
	U64 p = 1;

	if (movementFlags&7 && (from>>3)==3 && ((from-9==to)|(from-7==to)) && to==16+(movementFlags&0x7)) return 2; // en passant

	if (blackPawnAttackVectors(from)&p<<to&(bitboard[black]^bitboard[total])) return true; // taking
	if (bitboard[total]&p<<(from-8)) return false; // if there's something in front of the pawn it can't move
	if (from-to==16 && (from>>3)==6) {
		movementFlags&=0xF0;
		movementFlags|=from%8;
		movementFlags|=0x8;
	       	return true; // moving 2 iff the pawn hasn't moved before and there isn't a piece there
	}	
	return (from-to)==8; // normal movement
}

bool 
rookAttackMovement(U8 from, U8 to) 
{
	U64 p = 1;
	return (rookAttackVectors(from)&p<<to)>>to;
}

bool 
knightAttackMovement(U8 from, U8 to)
{
	U64 p = 1;
	return (knightAttackVectors(from)&p<<to)>>to;
}

bool 
bishopAttackMovement(U8 from, U8 to) 
{
	U64 p = 1;
	return (bishopAttackVectors(from)&p<<to)>>to;
}

bool 
queenAttackMovement(U8 from, U8 to)
{
	U64 p = 1;
	return (queenAttackVectors(from)&p<<to)>>to;
}

bool 
kingAttackMovement(U8 from, U8 to)
{
	//castling
	// find which castle it is and whether it can happen
	
	U64 p = 1;
	U64 l1 = 0x7000000000000000;
	U64 l2 = 0x3800000000000000;
	U64 l3 = 0x0600000000000000;
	U64 l4 = 0x0E00000000000000; 

	if ((from>>3)==0 && movementFlags&0x80 && to==0x05) // White left
		// Check squares are empty and not threatened
		if (!(bitboard[total]&0x70)&&!(bitboard[blackAttack]&0x38)) return 2;
	if ((from>>3)==0 && movementFlags&0x40 && to==0x01) // White right
		// Check squares are empty and not threatened
		if (!(bitboard[total]&0x06)&&!(bitboard[blackAttack]&0x0E)) return 3;
	if ((from>>3)==7 && movementFlags&0x20 && to==0x3D) // Black left
		// Check squares are empty and not threatened
		if (!(bitboard[total]&l1)&&!(bitboard[whiteAttack]&l2)) return 4;
	if ((from>>3)==7 && movementFlags&0x10 && to==0x39) // Black right
		// Check squares are empty and not threatened
		if (!(bitboard[total]&l3)&&!(bitboard[whiteAttack]&l4)) return 5;



	return (kingAttackVectors(from)&p<<to)>>to;
}

U64 
whitePawnAttackVectors(U8 pos)
{
	U64 vector = 0;
	U64 p = 1;
	p<<=8+pos;
	if (pos%8 != 0) vector ^= p>>1;
	if (pos%8 != 7) vector ^= p<<1;
	return vector;
}

U64 
blackPawnAttackVectors(U8 pos)
{
	U64 vector = 0;
	U64 p = 1;
	p<<=63;
	p>>=8+(63-pos);
	if (pos%8 != 0) vector ^= p>>1;
	if (pos%8 != 7) vector ^= p<<1;
	return vector;
}

U64 
rookAttackVectors(U8 pos)
{
	U64 vector1 = 0;
	U64 vector2 = 0;
	U64 vector3 = 0;
	U64 vector4 = 0;
	U64 p = 1;
	for (int i = pos+8; i < 64; i+=8) {
		vector1 |= p<<i; //NE 
		if (vector1&bitboard[total]) break;
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

U64 
knightAttackVectors(U8 pos)
{
	U64 vector = 0;
	U64 p = 1;
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

U64 
bishopAttackVectors(U8 pos)
{
	U64 vector1 = 0;
	U64 vector2 = 0;
	U64 vector3 = 0;
	U64 vector4 = 0;
	U64 p = 1;
	for (int i = pos+7; i < 64 && i%8!=7; i+=7) {
		vector1 |= p<<i; //NE 
		if (vector1&bitboard[total]) break;
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

U64 
queenAttackVectors(U8 pos)
{
	return rookAttackVectors(pos)|bishopAttackVectors(pos);	
}


U64 
kingAttackVectors(U8 pos)
{
	U64 vector = 0;
	U64 p = 1;
	U8 ldist = 7-(pos%8);
	U8 rdist = pos%8;
	U8 tdist = 7-(pos>>3);
	U8 bdist = pos>>3;
	
	if (tdist>0) vector|=p<<(pos+8); //N
	if (rdist>0) vector|=p<<(pos-1); //E
	if (bdist>0) vector|=p<<(pos-8); //S
	if (ldist>0) vector|=p<<(pos+1); //W

	vector |= ((vector&(p<<(pos+8)))>>1)&((vector&(p<<(pos-1)))<<8);
	vector |= ((vector&(p<<(pos-1)))>>8)&((vector&(p<<(pos-8)))>>1);
	vector |= ((vector&(p<<(pos-8)))<<1)&((vector&(p<<(pos+1)))>>8);
	vector |= ((vector&(p<<(pos+1)))<<8)&((vector&(p<<(pos+8)))<<1);

	return vector;
}
