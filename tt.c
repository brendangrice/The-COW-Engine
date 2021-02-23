#include "tt.h"
#include "ai.h"
// Zobrist hashing - unique identifier key of the gamestate used for transposition table tt

// init array of pseudorandom numbers

// one number for each peice at each square 							12x16
// one number to indicate if black to move								1
// four numbers to indicate castling rights 							4 
// eight numbers to indicate the file of a valid en passant square 		8 
// == 																	781 RANDOM NUMBERS

// pseudorandom https://en.wikipedia.org/wiki/Xorshift
unsigned int seed = 1804289383;
unsigned int randU32()
{
	unsigned int number = seed;
	number ^= number << 13;
	number ^= number >> 17;
	number ^= number << 5;
	
	seed = number;
	
	return number;
}
U64 randU64()
{
	U64 u1, u2, u3, u4;
	u1 = (U64)(randU32())&0xFFFF;
	u2 = (U64)(randU32())&0xFFFF;
	u3 = (U64)(randU32())&0xFFFF;
	u4 = (U64)(randU32())&0xFFFF;
	
	return u1|(u2<<16)|(u3<<32)|(u4<<48);
}


U64 piece_key[12][64];
U64 move_key;
U64 castle_key[4];
U64 enpassant_key[8];


void initHash()
{
	seed = 1804289383;
	// pieces on squares
	for(int i = 0 ; i < 12; i++)
		for(int j = 0 ; j < 64; j++)
			piece_key[i][j] = randU64();
	// en passant
	for(int i = 0 ; i < 8; i++)
		enpassant_key[i] = randU64();
	// castle_key
	for(int i = 0 ; i < 4; i++)
		castle_key[i] = randU64();
	// move key
	move_key = randU64();
}

static inline int getLeastBitIndex(Board b)
{
	if(b) return numberOfBits((b&-b)-1);
	return -1;
}

U64 generateHash(Boardstate bs, Coord to)
{
	U64 key = 0ULL;
	Board board;
	// calculate positional key influence
	for(int piece = 0 ; piece < 12; piece++)
	{
		// get the board of the current piece
		(piece<6)?(board=bs.bitboard[piece%6]&(bs.bitboard[total]^ bs.bitboard[black])):((board=bs.bitboard[black]&bs.bitboard[piece%6]));
		while(board)
		{
			int square = getLeastBitIndex(board); 	// get the index of the type of piece
			key ^= piece_key[piece][square]; 		// XOR the pseudorandom value of this peice on this square
			board &= ~(1ULL << square);				// pop the least significant bit of this board
		}
	}
	// calculate en passant influence
	if((to!=-1)&&(bs.movementflags&15))
	{
		key ^= enpassant_key[(to%8)]; // get enpassant value of the file that can be take en passant
	}
	// calculate castling influence
	U8 castleRights = (U8)bs.movementflags&0xF0; // isolate castle right bits from movementgflags
	while(castleRights)
	{
		int flag = getLeastBitIndex(castleRights); 	// get the index of the least significant flag
		key ^= castle_key[flag]; 					// XOR the pseudorandom value of this castle right flag
		castleRights &= ~(1ULL << flag);			// pop the least significant bit of this board
	}
	// calculate player to move influence
	if(bs.blackplaying)
	{
		key^=move_key;								// XOR value if black to move
	}
	return key;
}
