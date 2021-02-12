#include "eval.h"
#include "main.h"
#include "ai.h"


// tree 
/*
typedef struct Node{
	struct Node *children; // array of childern nodes
	Boardstate boardstate; // data of the node
	float advantage;	   // advantage score of the bitboard
} Tree;

Tree *InsertTree(Boardstate bs, Tree *pointer, int numberOfChildern)
{
	if(pointer)
	{
		pointer = (Tree*)malloc(sizeof(Tree));
		pointer -> boardstate = bs;
		
		for(int i = 0 ; i < numberOfChildern; i++)
		{
			pointer -> children[i] = pointer[i];
			printf("\n%d %d", pointer, pointer->children);
			//debugPrintBoard((pointer->childern[i]->boardstate).bitboard[total]);
		}
	}
}
*/
struct Node
{
	Boardstate boardstate; // each node has a boardstate
	float advantage; // each state has some advantage
	struct Node *next; // pointer to next node
	struct Node *child; // pointer to child node
};

struct Node* makeNode(Boardstate bs)
{
	struct Node* newNode = malloc(sizeof(struct Node));
	newNode-> boardstate = bs;
	newNode-> advantage = calculateAdvantage(bs);
	newNode-> next = newNode->child = NULL;
	return newNode;
};
struct Node *addSibling(struct Node *node, Boardstate bs)
{
	if(node == NULL) return NULL;
	while(node->next) node = node-> next;
	return (node->next = makeNode(bs));
};
struct Node *addChild(struct Node *node, Boardstate bs)
{
	if(node == NULL) return NULL;
	if(node->child) return addSibling(node->child, bs);
	return(node->child = makeNode(bs));
}
void traverseTree(struct Node *root)
{
	if(root == NULL) return;
	while(root)
	{
		debugPrintBoard(root->boardstate.bitboard[total]);
		printf("\t (id:%d) child = %d", root, root->next);
		printf("\t Advantage = %.3f", root->advantage);
		if(root->child) traverseTree(root->child);
		root = root->next;
	}
}



// Stack of linked lists
struct MoveStack{
	U8 data;
	struct MoveStack* next;
};
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
	if(isEmpty(*root)) return INT_MIN;
	struct MoveStack* temp = *root;
	*root = (*root)->next;
	U8 popped = temp->data;
	free(temp);
	return popped;
}
int peek(struct MoveStack* root)
{
	if(isEmpty(root)) return INT_MIN;
	return root->data;
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
			ownPieces = (bs.bitboard[total]^bs.bitboard[black]);
		}
		excludeSelf = ((bs.bitboard[total])&1ULL<<position);
		
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

struct MoveStack* 
possibleMoveTo(Boardstate bs, bool isBlack, U8 *from, int size)
{
	struct MoveStack* root = NULL;
	
	Board fullBoard = bs.bitboard[total]; // state of entire game
	Board self = 0ULL;
	Board b = 0ULL; // tempoary board to store where the piece is
	Board vectors = 0ULL; // vector board which will contain all possible moveable to squares
	
	isBlack?(self = bs.bitboard[black]) : (self = bs.bitboard[total] ^ bs.bitboard[black]);
	
	//!isBlack?(opponent = bs.bitboard[black]) : (opponent = bs.bitboard[total] ^ bs.bitboard[black]);
	
	isBlack ? (b = bs.bitboard[black]) : (b = bs.bitboard[total] ^ bs.bitboard[black]); // filter the board to either consider black or white pieces
	//struct MoveStack* root = NULL;
	
	
	// for each square
	for(int j = 0; j < size; j++){
		U8 i = from[j]; // position of a piece that can be moved.
		b = 1ULL<<i; // make a board that only has 1 bit to move set.
	
		if((b>>i)&1) // if the bit is set
		{
			// then check what type of piece it it
			if((b&1ULL<<i)&(bs.bitboard[pawn])) // if it is a pawn
			{
				if(isBlack)
				{
					if(blackPawnMovement(i, i-8, bs)) // can it move down one square
					{
						vectors |= (1ULL<<(i-8))^((1ULL<<(i-8))&self);
					}
					if(blackPawnMovement(i, i-16, bs)) // can it move down two square
					{
						vectors |= (1ULL<<(i-16))^((1ULL<<(i-16))&self);
					}
				}
				else
				{
					if(whitePawnMovement(i, i+8, bs)) // can it move up one square
					{
						vectors |= (1ULL<<(i+8))^((1ULL<<(i+8))&self);
					}
					if(blackPawnMovement(i, i+16, bs)) // can it move up two square
					{
						vectors |= (1ULL<<(i+16))^((1ULL<<(i+16))&self);
					}
				}
				
				//attaqcks
				isBlack ? ((vectors |= blackPawnAttackVectors(i))):(vectors |= whitePawnAttackVectors(i));
				
			}
			else if((b&1ULL<<i)&(bs.bitboard[bishop])) // if it is a bishop
			{
				vectors |= bishopAttackVectors(i, bs.bitboard)^(bishopAttackVectors(i, bs.bitboard)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[knight])) // if it is a knight
			{
				vectors |= knightAttackVectors(i)^(knightAttackVectors(i)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[rook])) // if it is a rook
			{
				vectors |= (rookAttackVectors(i, bs.bitboard)^((rookAttackVectors(i, bs.bitboard)&self)));
			}
			else if((b&1ULL<<i)&(bs.bitboard[king])) // if it is a king
			{
				vectors |= kingAttackVectors(i)^(kingAttackVectors(i)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[queen])) // if it is a queen
			{
				vectors |= queenAttackVectors(i, bs.bitboard)^(queenAttackVectors(i, bs.bitboard)&self);
			}
			else{
				printf("\n\t UNKNOWN PIECE");
			}
		}
	}
	//printf("\nAll of the squares that can be moved to");
	//debugPrintBoard(vectors);
	// pass these squares back 
	for(int i = 0 ; i < 64; i++)
	{
		if((vectors>>i)&1)
		{
			push(&root, i);
		}
	}
	
	//debugPrintBoard(vectors);
	return root;
}


struct MoveStack* 
moveablePieces(Boardstate bs, bool isBlack)
{
	Board b;
	isBlack ? (b = bs.bitboard[black]) : (b = bs.bitboard[total] ^ bs.bitboard[black]);
	//isBlack ? (printf("\nPlaying as black\n")):(printf("\nPlaying as white\n"));
	struct MoveStack* root = NULL;
	
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
			
			if(isMoveable(bs, i, piece, isBlack)){
				push(&root, i);
			}
			
		}
	}
	return root;
}

float
Min(float a, float b)
{
	return (((a)<(b))?(a):(b));
}
float
Max(float a, float b)
{
	return (((a)>(b))?(a):(b));
}

float
minimax(int depth, Boardstate bs, bool isBlack, float alpha, float beta)
{	
	if(depth == 0)
	{
		//printf("\n\tReached the end of a branch");
		//printf("\n\tAdvantage of this branch = %.3f", calculateAdvantage(bs));
		return -calculateAdvantage(bs);
	}
	
	// get a stack of available pieces to move
	struct MoveStack* availablePieces = moveablePieces(bs, isBlack);
	U8 from [getSize(availablePieces)]; // array that will hold the squares that can be moved from
	int i = 0;
	// while from pieces exist assign to from array
	while(!isEmpty(availablePieces))
	{
		from[i++] = pop(&availablePieces);
	}
	free(availablePieces);
	// get the squares that player can move to
	struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, &from, sizeof(from));
	U8 to [getSize(toSquares)]; // create array to store these squares that they can move to
	int x = 0;
	// while there are squares that they can move to, assign to to array
	while(!isEmpty(toSquares))
	{
		to[x++] = pop(&toSquares);
	}
	free(toSquares);
	
	
	float bestMove;
	int attempts = 0;
	
	
	if(!isBlack) // is maximising
	{
		bestMove = -9999;
		while(attempts < 50)
		{
			// make a move from selecting from random squares
			Coord one = from[rand()%sizeof(from)]; // assign to square
			Coord two = to[rand()%sizeof(to)];	  // assign from square
			
			Boardstate newbs;
			
			if(fauxMove(one, two, isBlack, bs, &newbs))
			{
				attempts++;
				//float newAdvantage = ; 
				bestMove = Max(bestMove, minimax(depth-1, newbs, !isBlack, alpha, beta));
				alpha = Max(alpha, bestMove);
				if(beta <= alpha)
				{
					return bestMove;
				}
			}
		}
	}
	else // is minimizing
	{
		bestMove = 9999;
		while(attempts < 50)
		{
			// make a move from selecting from random squares
			Coord one = from[rand()%sizeof(from)]; // assign to square
			Coord two = to[rand()%sizeof(to)];	  // assign from square
			
			Boardstate newbs;
			
			if(fauxMove(one, two, isBlack, bs, &newbs))
			{
				attempts++;
				float newAdvantage = minimax(depth -1, newbs, !isBlack, alpha, beta);
				bestMove = Min(bestMove, minimax(depth-1, newbs, !isBlack, alpha, beta));
				beta = Min(beta, bestMove);
				if(beta<=alpha)
				{
					return bestMove;
				}
			}
		}
	}
	return bestMove;
}

void
calculateBestMove(Boardstate bs, bool isBlack, int depth, Coord *coord1, Coord *coord2)
{
	// get a stack of available pieces to move
	struct MoveStack* availablePieces = moveablePieces(bs, isBlack);
	U8 from [getSize(availablePieces)]; // array that will hold the squares that can be moved from
	int i = 0;
	// while from pieces exist assign to from array
	while(!isEmpty(availablePieces))
	{
		from[i++] = pop(&availablePieces);
	}
	printf("\nprinting out the array of from positions = [");
	for(int j = 0 ; j < sizeof(from); j++)
	{
		printf("%d ", from[j]);
	}
	printf("]\n");
	// free memory
	free(availablePieces);
	// get the squares that player can move to
	struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, &from, sizeof(from));
	U8 to [getSize(toSquares)]; // create array to store these squares that they can move to
	int x = 0;
	// while there are squares that they can move to, assign to to array
	while(!isEmpty(toSquares))
	{
		to[x++] = pop(&toSquares);
	}
	printf("\nprinting out the array of to positions = [");
	for(int j = 0 ; j < sizeof(to); j++)
	{
		printf("%d ", to[j]);
	}
	printf("]\n");
	// free memory
	free(toSquares);
	printf("\nFinished subsetting");
	
	
	// store the best move that it can make
	Coord best1; 
	Coord best2;
	// track the best score of the game
	float bestScore = -9999.0;
	int attempts = 0; // try for a number of successful moves that improve position
	//bool bestMoveFound;
	while(attempts < 1000)
	{
		// make a move from selecting from random squares
		Coord one = from[rand()%sizeof(from)]; // assign to square
		Coord two = to[rand()%sizeof(to)];	  // assign from square
		
		Boardstate newbs;
		
		if(fauxMove(one, two, isBlack, bs, &newbs))
		{
			attempts++;
			//printf("\nThe move (%d %d) can be made", one, two);
			//debugPrintBoard(newbs.bitboard[total]);
			// check to see if this new board position is favorable 
			//float newAdvantage = calculateAdvantage(newbs);
			float newAdvantage = minimax(depth-1, newbs, !isBlack, -10000, 10000);
			
			if(newAdvantage > bestScore)
			{
				printf("\nadvantage = %.3f (%d %d)", newAdvantage, one, two);
				bestScore = newAdvantage;
				best1 = one;
				best2 = two;
			}
			
		}
		
	}
	printf("\nThe best move to make is (%d %d) @ %.3f", best1, best2, bestScore);
	*coord1 = best1;
	*coord2 = best2;
}