#include "eval.h"
#include "main.h"

// Pawn positional advantage table
int pawnPosition[64] = {
	0,  0,  0,  0,  0,  0,  0,  0,
	50, 50, 50, 50, 50, 50, 50, 50,
	10, 10, 20, 30, 30, 20, 10, 10,
 	5,  5, 10, 25, 25, 10,  5,  5,
 	0,  0,  0, 20, 20,  0,  0,  0,
 	5, -5,-10,  0,  0,-10, -5,  5,
	5, 10, 10,-20,-20, 10, 10,  5,
 	0,  0,  0,  0,  0,  0,  0,  0
};
// Knight positional advantage table
int knightPosition[64] = {
	-50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-30,  5, 15, 20, 20, 15,  5,-30,
	-30,  0, 15, 20, 20, 15,  0,-30,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-30,-30,-30,-30,-40,-50
};
// Bishop positional advantage table
int bishopPosition[64] = {
	-20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-10,-10,-10,-10,-10,-20
};
// Rook positional advantage table
int rookPosition[64] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  5, 10, 10, 10, 10, 10, 10,  5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	  0,  0,  0,  5,  5,  0,  0,  0
};
// Queen positional advantage table
int queenPosition[64] = {
	-20,-10,-10, -5, -5,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5,  5,  5,  5,  0,-10,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	  0,  0,  5,  5,  5,  5,  0, -5,
	-10,  5,  5,  5,  5,  5,  0,-10,
	-10,  0,  5,  0,  0,  0,  0,-10,
	-20,-10,-10, -5, -5,-10,-10,-20
};
// King midgame positional advantage table
int kingMidPosition[64] = {
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	 20, 20,  0,  0,  0,  0, 20, 20,
	 20, 30, 10,  0,  0, 10, 30, 20
};
// King endgame positional advantage table
int kingEndPosition[64] = {
	-50,-40,-30,-20,-20,-30,-40,-50,
	-30,-20,-10,  0,  0,-10,-20,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-30,  0,  0,  0,  0,-30,-30,
	-50,-30,-30,-30,-30,-30,-30,-50
};

int 
numberOfBits(Board b)
{
	unsigned x = 0;
	for(x = 0; b; x++)
		b&=b-1;
	return x;
}

int
calculatePositionAdvantage(Boardstate state)
{	
	// Seperate out each board and piece

	// white pieces
	U64 whiteBoard = (state.bitboard[total]^state.bitboard[black]);
	U64 wP = whiteBoard&state.bitboard[pawn];
	U64 wB = whiteBoard&state.bitboard[bishop];
	U64 wN = whiteBoard&state.bitboard[knight];
	U64 wQ = whiteBoard&state.bitboard[queen];
	U64 wR = whiteBoard&state.bitboard[rook];
	U64 wK = whiteBoard&state.bitboard[king];
	// black pieces
	U64 bP = state.bitboard[black]&state.bitboard[pawn];
	U64 bB = state.bitboard[black]&state.bitboard[bishop];
	U64 bN = state.bitboard[black]&state.bitboard[knight];
	U64 bQ = state.bitboard[black]&state.bitboard[queen];
	U64 bR = state.bitboard[black]&state.bitboard[rook];
	U64 bK = state.bitboard[black]&state.bitboard[king];

	// for each white and black piece
	// count the number of pieces of type and change the positional value of that pieces

	int whitePawnPos =  0;
	for(int i = 8; i < 64; i++)
	{
		if((wP>>i)&1) whitePawnPos+=(pawnPosition[63-i]);
	}

	int whiteBishopPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wB>>i)&1)
			whiteBishopPos+=(bishopPosition[63-i]);
	}
	int whiteKnightPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wN>>i)&1)
			whiteKnightPos+=(knightPosition[63-i]);
	}
	int whiteQueenPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wQ>>i)&1)
			whiteQueenPos+=(queenPosition[63-i]);
	}
	int whiteRookPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wR>>i)&1)
			whiteRookPos+=(rookPosition[63-i]);
	}
	int whiteKingPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wK>>i)&1)
			whiteKingPos+=(kingMidPosition[63-i]);
	}
	int blackPawnPos =  0;
	for(int i = 0; i < 56; i++)
	{
		if((bP>>i)&1)
			blackPawnPos+=(pawnPosition[i]);
	}

	int blackBishopPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bB>>i)&1)
			blackBishopPos+=(bishopPosition[i]);
	}
	int blackKnightPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bN>>i)&1)
			blackKnightPos+=(knightPosition[i]);
	}
	int blackQueenPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bQ>>i)&1)
			blackQueenPos+=(queenPosition[i]);
	}
	int blackRookPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bR>>i)&1)
			blackRookPos+=(rookPosition[i]);
	}
	int blackKingPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bK>>i)&1)
			blackKingPos+=(kingMidPosition[i]);
	}

	// sum each piece positional value for each colour
	int whiteTotal = whitePawnPos + whiteBishopPos + whiteKnightPos + whiteRookPos + whiteQueenPos + whiteKingPos;
	int blackTotal = blackPawnPos + blackBishopPos + blackKnightPos + blackRookPos + blackQueenPos + blackKingPos;

	// return the difference of the sum of white and black positional value. 
	return whiteTotal - blackTotal;
}



int
calculateMaterialAdvantage(Boardstate state)
{
	// Get count of each type of piece on the board
	int totalPawns = numberOfBits(state.bitboard[pawn]);
	int totalQueens = numberOfBits(state.bitboard[queen]);
	int totalBishops = numberOfBits(state.bitboard[bishop]);
	int totalKnights = numberOfBits(state.bitboard[knight]);
	int totalRooks = numberOfBits(state.bitboard[rook]);
	int totalKings = numberOfBits(state.bitboard[king]);

	    // Distingush between white and black pawns

	    // pawns
	int blackPawns = numberOfBits(state.bitboard[black]&state.bitboard[pawn]);
	int whitePawns = totalPawns-blackPawns;

	// queens
	int blackQueens = numberOfBits(state.bitboard[black]&state.bitboard[queen]);
	int whiteQueens = totalQueens-blackQueens;


	// bishops
	int blackBishops = numberOfBits(state.bitboard[black]&state.bitboard[bishop]);
	int whiteBishops = totalBishops-blackBishops;

	// knights
	int blackKnights = numberOfBits(state.bitboard[black]&state.bitboard[knight]);
	int whiteKnights = totalKnights-blackKnights;

	// rooks
	int blackRooks = numberOfBits(state.bitboard[black]&state.bitboard[rook]);
	int whiteRooks = totalRooks-blackRooks;

	// kings
	int blackKings = numberOfBits(state.bitboard[black]&state.bitboard[king]);
	int whiteKings = totalKings-blackKings;

	    // Calculate Black and White material value using the count of pieces that they have
	    // multiplied by general piece value
	int whiteMaterial = whitePawns + (whiteKnights * 3) + (whiteBishops * 3) + (whiteRooks * 5) + (whiteQueens * 9) + (whiteKings * 100);
	int blackMaterial = blackPawns + (blackKnights * 3) + (blackBishops * 3) + (blackRooks * 5) + (blackQueens * 9) + (blackKings * 100);

	    // return the difference of material value
	return whiteMaterial - blackMaterial;
}

float
calculateAdvantage(Boardstate state)
{
	// Get material and positional advantage
	float score = calculateMaterialAdvantage(state);
	int pos = calculatePositionAdvantage(state);
	// return this score after some weighting adjustments.
	return score+(pos*0.01);
}


