#include "eval.h"
#include "main.h"

int numberOfBits(Board b)
{
	unsigned x = 0;
	for(x = 0; b; x++)
		b&=b-1;
	return x;
}
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
calculatePositionAdvantage(Boardstate state)
{	

	// white pawn positional value
	// white pawns cannot exist in the first rank
	
	U64 whiteBoard = (state.bitboard[total]^state.bitboard[black]);
	U64 wP = whiteBoard&state.bitboard[pawn];
	U64 wB = whiteBoard&state.bitboard[bishop];
	U64 wN = whiteBoard&state.bitboard[knight];
	U64 wQ = whiteBoard&state.bitboard[queen];
	U64 wR = whiteBoard&state.bitboard[rook];
	U64 wK = whiteBoard&state.bitboard[king];

	U64 bP = state.bitboard[black]&state.bitboard[pawn];
	U64 bB = state.bitboard[black]&state.bitboard[bishop];
	U64 bN = state.bitboard[black]&state.bitboard[knight];
	U64 bQ = state.bitboard[black]&state.bitboard[queen];
	U64 bR = state.bitboard[black]&state.bitboard[rook];
	U64 bK = state.bitboard[black]&state.bitboard[king];

	// calculate all of white position score
	int whitePawnPos =  0;
	for(int i = 8; i < 64; i++)
	{
		if((wP>>i)&1){
			whitePawnPos+=(pawnPosition[63-i]);
		}
	}

	int whiteBishopPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wB>>i)&1){
			whiteBishopPos+=(bishopPosition[63-i]);
		}
	}
	int whiteKnightPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wN>>i)&1){
			whiteKnightPos+=(knightPosition[63-i]);
		}
	}
	int whiteQueenPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wQ>>i)&1){
			whiteQueenPos+=(queenPosition[63-i]);
		}
	}
	int whiteRookPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wR>>i)&1){
			whiteRookPos+=(rookPosition[63-i]);
		}
	}
	int whiteKingPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((wK>>i)&1){
			whiteKingPos+=(kingMidPosition[63-i]);
		}
	}
	int blackPawnPos =  0;
	for(int i = 0; i < 56; i++)
	{
		if((bP>>i)&1){
			blackPawnPos+=(pawnPosition[i]);
		}
	}

	int blackBishopPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bB>>i)&1){
			blackBishopPos+=(bishopPosition[i]);
		}
	}
	int blackKnightPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bN>>i)&1){
			blackKnightPos+=(knightPosition[i]);
		}
	}
	int blackQueenPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bQ>>i)&1){
			blackQueenPos+=(queenPosition[i]);
		}
	}
	int blackRookPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bR>>i)&1){
			blackRookPos+=(rookPosition[i]);
		}
	}
	int blackKingPos =  0;
	for(int i = 0; i < 64; i++)
	{
		if((bK>>i)&1){
			blackKingPos+=(kingMidPosition[i]);
		}
	}

	/*
	printf("\nWhite Pawn Position = %d", whitePawnPos);
	printf("\nWhite Bishop Position = %d", whiteBishopPos);
	printf("\nWhite Knight Position = %d", whiteKnightPos);
	printf("\nWhite Rook Position = %d", whiteRookPos);
	printf("\nWhite Queen Position = %d", whiteQueenPos);
	printf("\nWhite King Position = %d", whiteKingPos);

	printf("\nBlack Pawn Position = %d", blackPawnPos);
	printf("\nBlack Bishop Position = %d", blackBishopPos);
	printf("\nBlack Knight Position = %d", blackKnightPos);
	printf("\nBlack Rook Position = %d", blackRookPos);
	printf("\nBlack Queen Position = %d", blackQueenPos);
	printf("\nBlack King Position = %d\n", blackKingPos);
	*/

	int whiteTotal = whitePawnPos + whiteBishopPos + whiteKnightPos + whiteRookPos + whiteQueenPos + whiteKingPos;
	int blackTotal = blackPawnPos + blackBishopPos + blackKnightPos + blackRookPos + blackQueenPos + blackKingPos;

	//printf("\nPositional Difference : %d\n", whiteTotal - blackTotal);

	return whiteTotal - blackTotal;
}



int
calculateMaterialAdvantage(Boardstate state)
{
	// TODO Make function similar to below - easier to read and to change in the future

	/*
		materialScore = kingWt  * (wK-bK)
              		+ queenWt * (wQ-bQ)
              		+ rookWt  * (wR-bR)
              		+ knightWt* (wN-bN)
              		+ bishopWt* (wB-bB)
              		+ pawnWt  * (wP-bP)

		mobilityScore = mobilityWt * (wMobility-bMobility) 
	*/

	// Pawn worth 1
	// Knight worth 3
	// Bishop worth 3
	// Rook worth 5
	// Queen worth 9
	
	
	// total number of pawns       
	int totalPawns = numberOfBits(state.bitboard[pawn]);
	//printf("\nPawns = %d", totalPawns);
	int totalQueens = numberOfBits(state.bitboard[queen]);
    int totalBishops = numberOfBits(state.bitboard[bishop]);
    int totalKnights = numberOfBits(state.bitboard[knight]);
    int totalRooks = numberOfBits(state.bitboard[rook]);
    int totalKings = numberOfBits(state.bitboard[king]);

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

	// white score
    int whiteMaterial = whitePawns + (whiteKnights * 3) + (whiteBishops * 3) + (whiteRooks * 5) + (whiteQueens * 9);

    // black score
    int blackMaterial = blackPawns + (blackKnights * 3) + (blackBishops * 3) + (blackRooks * 5) + (blackQueens * 9);

	int materialScore = whiteMaterial - blackMaterial;
	/*
	printf("\nPawns: %d", totalPawns);
	printf("\nQueens: %d", totalQueens);
	printf("\nBishops: %d", totalBishops);
	printf("\nKnights: %d", totalKnights);
	printf("\nRooks: %d", totalRooks);
	printf("\nKings: %d", totalKings);

	printf("\nBlack Pawns : %d", blackPawns);
	printf("\nBlack Queens : %d", blackQueens);
	printf("\nBlack Bishops : %d", blackBishops);
    printf("\nBlack Knights : %d", blackKnights);
    printf("\nBlack Rooks : %d", blackRooks);
	printf("\nBlack Kings : %d", blackKings);

    printf("\nWhite Pawns : %d", whitePawns);	
    printf("\nWhite Queens : %d", whiteQueens);
    printf("\nWhite Bishops : %d", whiteBishops);
    printf("\nWhite Knights : %d", whiteKnights);
    printf("\nWhite Rooks : %d", whiteRooks);
    printf("\nWhite Kings : %d", whiteKings);

    printf("\nWhite Material : %d", whiteMaterial);
    printf("\nBlack Material : %d", blackMaterial);
    
    printf("\nMaterial Advantage : %d\n", materialScore);
	*/
    return materialScore;
}

void 
debugPrint(Board board)
{
    printf("\n");
    for (int i = 1; i <= 8; i++) { // go through the long long and make bytes
		//printBits(board>>(64-(i*8))&0xFF);
	}
	puts("");
}	

float
calculateAdvantage(Boardstate state)
{
	// positive score => white advantage
	// negative score => black advantage
	float score = calculateMaterialAdvantage(state);
	int pos = calculatePositionAdvantage(state);

	return score+(pos*0.01);
}


