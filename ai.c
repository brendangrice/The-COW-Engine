#include "eval.h"
#include "main.h"
#include "ai.h"

bool 
isMoveable(Boardstate bs, int position, char piece, bool isBlack)
{
	// given a boardstate, position and piece is the piece able to move?
	Board vectors = 0ULL; // vector where piece can move
	Board ownPieces = 0ULL; // Board of own colour pieces
	Board excludeSelf = ((bs.bitboard[total])&1ULL<<position); // single bit of the piece 
	
	// decide what colour pieces are the owned, ie. cannot be taken
	isBlack ? (ownPieces = bs.bitboard[black]) : (ownPieces = bs.bitboard[total]^bs.bitboard[black]);
	
	// depending on what type of piece it is, get the attack vectors of that piece from the position
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
		vectors = knightAttackVectors(position, NULL);
	}
	else if(piece=='k'||piece=='K')
	{
		vectors = kingAttackVectors(position, NULL);
	}
	else if(piece=='p'||piece=='P') // TODO fix pawns
	{
		// if its a pawn then vectors are attack OR movement. Can be black or white
		if(isBlack)
		{
			vectors |= (blackPawnAttackVectors(position, NULL));
			vectors |= blackPawnMovement(position, position<<8, bs, &vectors);
			ownPieces = bs.bitboard[black];
		}
		else
		{
			vectors |= (whitePawnAttackVectors(position, NULL));
			vectors |= whitePawnMovement(position, position>>8, bs, &vectors);
			ownPieces = (bs.bitboard[total]^bs.bitboard[black]);
		}
		excludeSelf = ((bs.bitboard[total])&1ULL<<position);
	}
	// return if there is a square where it can move to
	return (
				(((vectors^ownPieces)^excludeSelf^ownPieces)
				&((vectors^ownPieces)^excludeSelf))
				!=0
		);
}

struct MoveStack* 
possibleMoveTo(Boardstate bs, bool isBlack, U8 *from, int size)
{
	struct MoveStack* root = NULL; // create a new stack
	
	Board self = 0ULL; // same color pieces as player
	Board b = 0ULL; // tempoary board to store where the piece is
	Board vectors = 0ULL; // vector board which will contain all possible moveable to squares
	Board temp = 0ULL; // Adding to deal with the changes made to moves.c
	isBlack?(self = bs.bitboard[black]) : (self = bs.bitboard[total] ^ bs.bitboard[black]); // assign self
	
	isBlack ? (b = bs.bitboard[black]) : (b = bs.bitboard[total] ^ bs.bitboard[black]); // filter the board to either consider black or white pieces
	
	
	// for each square
	for(int j = 0; j < size; j++){
		U8 i = from[j]; // position of a piece that can be moved.
		b = 1ULL<<i; // make a board that only has 1 bit to move set.
		if((b>>i)&1) // if the bit is set
		{
			// then check what type of piece it it
			if((b&1ULL<<i)&(bs.bitboard[pawn])) // if it is a pawn
			{
				if(isBlack) // and it is black
				{
					if(blackPawnMovement(i, i-8, bs, &temp)) // can it move down one square
					{
						vectors |= (1ULL<<(i-8))^((1ULL<<(i-8))&self);
					}
					if(blackPawnMovement(i, i-16, bs, &temp)) // can it move down two square
					{
						vectors |= (1ULL<<(i-16))^((1ULL<<(i-16))&self);
					}
				}
				else // or it is white
				{
					if(whitePawnMovement(i, i+8, bs, &temp)) // can it move up one square
					{
						vectors |= (1ULL<<(i+8))^((1ULL<<(i+8))&self);
					}
					if(whitePawnMovement(i, i+16, bs, &temp)) // can it move up two square
					{
						vectors |= (1ULL<<(i+16))^((1ULL<<(i+16))&self);
					}
				}
				
				// pawn attacks
				isBlack ? ((vectors |= blackPawnAttackVectors(i, NULL))):(vectors |= whitePawnAttackVectors(i, NULL));
				
			}
			else if((b&1ULL<<i)&(bs.bitboard[bishop])) // if it is a bishop
			{
				vectors |= bishopAttackVectors(i, bs.bitboard)^(bishopAttackVectors(i, bs.bitboard)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[knight])) // if it is a knight
			{
				vectors |= knightAttackVectors(i, NULL)^(knightAttackVectors(i, NULL)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[rook])) // if it is a rook
			{
				vectors |= (rookAttackVectors(i, bs.bitboard)^((rookAttackVectors(i, bs.bitboard)&self)));
			}
			else if((b&1ULL<<i)&(bs.bitboard[king])) // if it is a king
			{
				vectors |= kingAttackVectors(i, NULL)^(kingAttackVectors(i, NULL)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[queen])) // if it is a queen
			{
				vectors |= queenAttackVectors(i, bs.bitboard)^(queenAttackVectors(i, bs.bitboard)&self);
			}
			else{
				puts("\t UNKNOWN PIECE");
			}
		}
	}
	
	// pass these squares back 
	for(int i = 0 ; i < 64; i++)
	{
		if((vectors>>i)&1)
		{
			push(&root, i); // push these squares that it can be moved to the move stack
		}
	}
	 // return the stack of squares that can be moved to
	return root;
}
struct MoveStack* 
moveablePieces(Boardstate bs, bool isBlack)
{
	Board b;
	isBlack ? (b = bs.bitboard[black]) : (b = bs.bitboard[total] ^ bs.bitboard[black]); // get the black or white board
	struct MoveStack* root = NULL;

	// check all pieces if they can move
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
				puts("\t UNKNOWN PIECE");
			}
			// after establishing what type of piece it is, check if it was a black or white piece
			if((b&1ULL<<i)&(bs.bitboard[black])) //
			{
				piece += 32;
			}
			// check if the piece has a movement or an attack vector			
			// now that we know where the from position is, and what type of piece it is, check if it can moved. 	
			if(isMoveable(bs, i, piece, isBlack)){
				push(&root, i);
			}			
		}
	}
	return root;
}

float
NegaMax(int depth, Boardstate bs, bool isBlack, float alpha, float beta)
{
	// Both players are trying to maximise their score, considering that the score that the subsequent player
	// is trying to beat is the negation of the last move

	// search is finsihed return the value
	if(depth == 0) return calculateAdvantage(bs);

	int from_index = 0;
	int to_index = 0;	

	struct MoveStack* availablePieces = moveablePieces(bs, isBlack); // get a stack of available pieces to move
	U8 from [getSize(availablePieces)]; // from squares
	while(!isEmpty(availablePieces)) from[from_index++] = pop(&availablePieces); // pop from stack into from array
	free(availablePieces);
	struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, from, sizeof(from)); // get a stack of squares the from pieces can move to
	U8 to [getSize(toSquares)]; // to squares
	while(!isEmpty(toSquares)) to[to_index++] = pop(&toSquares); // pop from stack into to array
	free(toSquares);
		
	float bestMove = -9999; // impossibly low score to ensure a move is made

	// for each move
	for(int f = 0 ; f < sizeof(from); f++) 
	{
		for(int t = 0 ; t < sizeof(to); t++)
		{
			// from --> to coords
			Coord one = from[f];
			Coord two = to[t];
			// create a new boardstate
			Boardstate newbs = makeBoardstate(NULL, bs.movementflags, bs.blackplaying);
			// ensure that the move is valid
			if(fauxMove(one, two, bs, &newbs, piecePromotionAI))
			{
				// flip player to move
				newbs.blackplaying = !newbs.blackplaying; 
				// the best move is the higher value of either the current best move
				// or the negation of the next depth search
				float temp = -NegaMax(depth-1, newbs, !isBlack, -beta, -alpha);
				bestMove = MAX(bestMove, temp);
				// alpha is the higher value of either the current alpha or the best move
				alpha = MAX(alpha, bestMove);
				// if alpha is greater than or equal to beta, then we can stop searching this branch.
				// the score is not the in the players favour
				if(alpha >= beta) return alpha;
			}
		}
	}
	return bestMove;
}

bool
calculateBestMove(Boardstate bs, bool isBlack, int depth, Coord *coord1, Coord *coord2)
{	
	int from_index = 0;
	int to_index = 0;	

	struct MoveStack* availablePieces = moveablePieces(bs, isBlack); // get a stack of available pieces to move
	U8 from [getSize(availablePieces)]; // from squares
	while(!isEmpty(availablePieces)) from[from_index++] = pop(&availablePieces); // pop from stack into from array
	free(availablePieces);
	struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, from, sizeof(from)); // get a stack of squares the from pieces can move to
	U8 to [getSize(toSquares)]; // to squares
	while(!isEmpty(toSquares)) to[to_index++] = pop(&toSquares); // pop from stack into to array
	free(toSquares);
	
	// store the best move that it can make
	Coord best1 = 'a'; // arbitary assignment to prevent warnings
	Coord best2 = 'a'; // arbitary assignment to prevent warnings


	float bestScore = -9999.0; // impossibly low score to ensure that a move is made

	// for each move	
	for(int f = 0 ; f < sizeof(from); f++)
	{
		for(int t = 0 ; t < sizeof(to); t++)
		{
			// from --> to coords
			Coord one = from[f]; 
			Coord two = to[t];
			// create a new boardstate
			Boardstate newbs = makeBoardstate(NULL, bs.movementflags, bs.blackplaying);
			// ensure that the move is valid			
			if(fauxMove(one, two, bs, &newbs, piecePromotionAI))
			{
				// flip the player to move
				newbs.blackplaying = !newbs.blackplaying;
				// get the advantage of this position via inital Negamax with extreme alpha and beta
				float newAdvantage = -NegaMax(depth, newbs, !isBlack, -10000, 10000);
				// if the new advantage is greater than the old advantage. Remember the coords for this move

				if(newAdvantage >= bestScore)
				{
					bestScore = newAdvantage;
					best1 = one;
					best2 = two;
				}
			}
		}
	}
	// after search has terminated
	char temp[3];
	char temp2[3];
	temp[2] = temp2[2] = 0; // 0 terminate
	printf("\n>Hmmmmm. . . \n>Im going to play (%s %s)\n", ctoa(temp, best1), ctoa(temp2, best2));
	// assign best found coords to the coord pointers
	*coord1 = best1;
	*coord2 = best2;
	return true;
}

struct MoveStack* createNode(U8 data)
{
	struct MoveStack* moveStack = 
	(struct MoveStack*)malloc(sizeof(struct MoveStack));
	moveStack->data = data;
	moveStack->next = NULL;
	return moveStack;
}
bool isEmpty(struct MoveStack* root)
{
	return !root;
}
void push(struct MoveStack** root, U8 data)
{
	struct MoveStack* moveStack = createNode(data);
	moveStack->next = *root;
	*root = moveStack;
}
int pop(struct MoveStack** root)
{
	if(isEmpty(*root)) return -0;
	struct MoveStack* temp = *root;
	*root = (*root)->next;
	U8 popped = temp->data;
	free(temp);
	return popped;
}
int getSize(struct MoveStack* root)
{
	int size = 0;
	while(root)
	{
		size++;
		root = (root)->next;
	}	
	return size;
}

U8
piecePromotionAI()
{
	return queen; // all my homies hate knights
}
