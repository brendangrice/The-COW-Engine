#include "eval.h"
#include "main.h"
#include "ai.h"


bool isMoveable(Boardstate bs, int position, char piece, bool isBlack)
{
	// given a boardstate, position and piece is the piece able to move?
	Board vectors = 0ULL;
	Board ownPieces = 0ULL;
	Board excludeSelf = ((bs.bitboard[total])&1ULL<<position);
	
	isBlack ? (ownPieces = bs.bitboard[black]) : (ownPieces = bs.bitboard[total]^bs.bitboard[black]);
		
	if(piece=='r'||piece=='R')
	{
		vectors = rookAttackVectors(position, bs.bitboard);
	}
	else if(piece=='b'||piece=='B')
	{
		vectors = bishopAttackVectors(position, bs.bitboard);
	}
	else if(piece=='q'||piece=='Q')
	{
		vectors = queenAttackVectors(position, bs.bitboard);
	}
	else if(piece=='n'||piece=='N')
	{
		vectors = knightAttackVectors(position);
	}
	else if(piece=='k'||piece=='K')
	{
		vectors = kingAttackVectors(position);
	}
	else if(piece=='p'||piece=='P') // TODO fix pawns
	{
		
		if(isBlack)
		{
			//vectors |= (blackPawnAttackVectors(position)&(bs.bitboard[total]^bs.bitboard[black]));
			vectors |= (blackPawnAttackVectors(position));
			vectors |= blackPawnMovement(position, position<<8, bs);
			ownPieces = bs.bitboard[black];
		}
		else
		{
			//vectors |= (whitePawnAttackVectors(position)&(bs.bitboard[black]));
			vectors |= (whitePawnAttackVectors(position));
			vectors |= whitePawnMovement(position, position>>8, bs);
			ownPieces = bs.bitboard[total]^bs.bitboard[black];
		}
		//excludeSelf = ((bs.bitboard[total])&1ULL<<position);
		
		/*
		return (
				(((vectors^ownPieces)^excludeSelf^ownPieces)
				&(vectors^ownPieces)^excludeSelf)
				!=0
		);
		*/
	}
	
	
	return (
				(((vectors^ownPieces)^excludeSelf^ownPieces)
				&((vectors^ownPieces)^excludeSelf))
				!=0
		);
}

void printBits(U8 byte) // used in debugging
{
	for (int i = 0; i < 8; i++) {
		putchar(byte&128?'1':'0');
		byte<<=1;	
	}
	putchar('\n');
}
void debugPrintBoard(Board b) {
	printf("\n");
	for (int i = 1; i <= 8; i++) { // go through the long long and make bytes
		printBits(b>>(64-(i*8))&0xFF);
	}
	puts("");
}

bool moveablePieces(Boardstate bs, bool isBlack)
{
	Board b = bs.bitboard[black];
	//printf("\nID = %llu", b);
	//printBoard(b, isBlack);
	
	printf("\n[");
	// print out all of ints that can be moved. 
	for(int i = 0 ; i < 64; i++)
	{
		if((b>>i)&1) // if the bit is set
		{
			char piece = 'A';	
			// then check what type of piece it it
			if((b&1ULL<<i)&(bs.bitboard[pawn])) // if it is a pawn
			{
				piece = 'P';
			}
			else if((b&1ULL<<i)&(bs.bitboard[bishop])) // if it is a bishop
			{
				piece = 'B';
			}
			else if((b&1ULL<<i)&(bs.bitboard[knight])) // if it is a knight
			{
				piece = 'N';
			}
			else if((b&1ULL<<i)&(bs.bitboard[rook])) // if it is a rook
			{
				piece = 'R';
			}
			else if((b&1ULL<<i)&(bs.bitboard[king])) // if it is a king
			{
				piece = 'K';
			}
			else if((b&1ULL<<i)&(bs.bitboard[queen])) // if it is a queen
			{
				piece = 'Q';
			}
			else{
				printf("\n\t UNKNOWN PIECE");
			}
			
			// after establishing what type of piece it is, check if it was a black or white piece
			if((b&1ULL<<i)&(bs.bitboard[black])) //
			{
				piece += 32;
			}
			
			// check if the piece has a movement or an attack vector
			
			// now that we know where the from position is, and what type of piece it is, check if it can moved. 
			
			//printf("\n[");
			if(isMoveable(bs, i, piece, isBlack))
				printf("%d (%c), ",i ,piece);
			//printf(" can this piece move ?, %d", isMoveable(bs, i, piece));
			
		}
	}
	printf("]\n");
	return true;
}

bool
negaMax(int depth, float score, bool isBlack, Boardstate bs, Coord *coord1, Coord *coord2)
{
	printf("\nCURRENT DEPTH = %d", depth);
	printf("\nSCORE = %.3f", score);
	//printBoard(bs.bitboard, isBlack);
	if(depth == 0)
	{ 
		return true;
	}
	int max = -99999;
	Coord bestCord1;
	Coord bestCord2;
	

	// TODO remove total randomness of what move is selected, instead subset pieces that could be moved, then use this subset. 
	char l1, l2;
	int n1, n2;
	
	char letter[8] = {'A','B','C','D','E','F','G','H'};
	int  number[8] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 };
	
	int attempts = 0;
	
	time_t t;
	srand((unsigned) time(&t));
	
	float currentAdvantage = calculateAdvantage(bs);
	float newAdvantage;
	float bestAdvantage = currentAdvantage;
	//printf("\n The current game state advantage = %.3f\n", currentAdvantage);
	
	////printf("\nMaking new board");
	Boardstate *newbs = malloc(BOARDSTATESIZE);
	////printf("\nNew board made successfully");
	
	while(attempts < 50)
	{		
		////printf("\n\tassign l1");
		l1 = letter[rand()%8];
		////printf("\n\tassign l2");
		l2 = letter[rand()%8];
		////printf("\n\tassign n1");
		n1 = number[rand()%8];
		////printf("\n\tassign n2");
		n2 = number[rand()%8];
		
		////printf("\n\t\tCoord1");
		
		*coord1 = 7 - l1 + 'A' + (n1-1)*8;
		////printf("\n\t\tCoord2");
		
		*coord2 = 7 - l2 + 'A' + (n2-1)*8;
		
		
		////printf("\nAttempting fauxMove");
		if(fauxMove(*coord1, *coord2, isBlack, bs, newbs))
		{
			attempts++;
			newAdvantage = calculateAdvantage(*newbs);
			if(newAdvantage <= bestAdvantage){
				//printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
				bestAdvantage = newAdvantage;
				bestCord1 = *coord1;
				bestCord2 = *coord2;
			}
		}
		else{
			//free(newbs);
		}
		
	}
	
	*coord1 = bestCord1;
	*coord2 = bestCord2;
	
	Boardstate newerbs = *newbs;
	//newAdvantage = calculateAdvantage(*newbs);
	free(newbs);
	//score = -negaMax(depth-1, score, !isBlack, newerbs, coord1, coord2);
	//score = -negaMax(depth-1, score	, !isBlack, newerbs, coord1, coord2);
	if(score > max)
	{
		max = score;
		score = -negaMax(depth-1, max, !isBlack, newerbs, coord1, coord2);
	}
	return true;
}


/*
bool
botMove(Coord *coord1, Coord *coord2, Boardstate state)
{
	char l1, l2;
	int n1, n2;
	
	int depth = 5;

	char letter[8] = {'A','B','C','D','E','F','G','H'};
	int  number[8] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 };

	bool foundMove = false;
	
	//struct Node *start = NULL;

	time_t t;
	srand((unsigned) time(&t));

	float currentAdvantage = calculateAdvantage(state);
	float newAdvantage;
	float bestAdvantage = currentAdvantage;
	printf("\n The current game state advantage = %.3f\n", currentAdvantage);

	int attempts = 0;
	
	Coord bestCord1;
	Coord bestCord2;
	
	while(!foundMove && attempts < 50)
	{
		// get random coordinates
		l1 = letter[rand()%8];
		l2 = letter[rand()%8];
		n1 = number[rand()%8];
		n2 = number[rand()%8];

		*coord1 = 7 - l1 + 'A' + (n1-1)*8;
		*coord2 = 7 - l2 + 'A' + (n2-1)*8;
		
		Boardstate *newbs = malloc(BOARDSTATESIZE); // new boardstate
		
		//if(validMove(*coord1, *coord2, true))
		if(fauxMove(*coord1, *coord2, true, state, newbs))
		{
			attempts++;
			//foundMove = true;
			
			
			// calculateAdvantage of the new state
			newAdvantage = calculateAdvantage(newbs);
			////printf("\n\t\t ## New Advantage = %.3f", newAdvantage);
			if(newAdvantage <= bestAdvantage){
				//printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
				//push(&start, &newbs, sizeof(Boardstate));
				bestAdvantage = newAdvantage;
				bestCord1 = *coord1;
				bestCord2 = *coord2;
			}
		}
		else{
			free(newbs);
		}
		
	}
	
	*coord1 = bestCord1;
	*coord2 = bestCord2;
	
	return true;
}
*/
/*
bool
botMove(Coord *coord1, Coord *coord2, Boardstate state)
{
	char l1, l2;
	int n1, n2;

	char letter[8] = {'A','B','C','D','E','F','G','H'};
	int  number[8] = { 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 };

	bool foundMove = false;
	
	//struct Node *start = NULL;

	time_t t;
	srand((unsigned) time(&t));

	float currentAdvantage = calculateAdvantage(state);
	float newAdvantage;
	float bestAdvantage = currentAdvantage;
	//printf("\n The current game state advantage = %.3f\n", currentAdvantage);

	int attempts = 0;
	
	Coord bestCord1;
	Coord bestCord2;
	
	while(!foundMove && attempts < 50)
	{
		// get random coordinates
		l1 = letter[rand()%8];
		l2 = letter[rand()%8];
		n1 = number[rand()%8];
		n2 = number[rand()%8];

		*coord1 = 7 - l1 + 'A' + (n1-1)*8;
		*coord2 = 7 - l2 + 'A' + (n2-1)*8;
		
		Boardstate *newbs = malloc(BOARDSTATESIZE); // new boardstate
		
		//if(validMove(*coord1, *coord2, true))
		if(fauxMove(*coord1, *coord2, true, state, newbs))
		{
			attempts++;
			//foundMove = true;
			
			
			// calculateAdvantage of the new state
			newAdvantage = calculateAdvantage(newbs);
			////printf("\n\t\t ## New Advantage = %.3f", newAdvantage);
			if(newAdvantage <= bestAdvantage){
				//printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
				//push(&start, &newbs, sizeof(Boardstate));
				bestAdvantage = newAdvantage;
				bestCord1 = *coord1;
				bestCord2 = *coord2;
			}
		}
		else{
			free(newbs);
		}
		
	}
	
	*coord1 = bestCord1;
	*coord2 = bestCord2;
	
	return true;
}
*/
