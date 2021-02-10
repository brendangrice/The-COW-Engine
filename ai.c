#include "eval.h"
#include "main.h"
#include "ai.h"

// Stack of moveable pieces array based - Does not reduce in size
/*
struct Stack {
	int top;
	unsigned size;
	int* array;
};
struct Stack* createStack(unsigned size)
{
	struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
	stack->size = size;
	stack->top = -1;
	stack->array = (int*)malloc(stack->size * sizeof(int));
	return stack;
}
bool isEmpty(struct Stack* stack)
{
	return stack->top==-1;
}
bool isFull(struct Stack* stack)
{
	return stack->top==stack->size-1;
}
void push(struct Stack* stack, int item)
{
	if(isFull(stack)) return;
	stack->array[++stack->top] = item;
	printf("%d pushed to stack\n", item);
}
int pop(struct Stack* stack)
{
	if(isEmpty(stack)) return INT_MIN;
	return stack->array[stack->top--];
}
int peek(struct Stack* stack)
{
	if(isEmpty(stack)) return INT_MIN;
	return stack->array[stack->top];
}

*/

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

struct MoveStack* possibleMoveToBroken(Boardstate bs, bool isBlack, struct MoveStack* from)
{
	printf("\n\t\t\t\t\t OK POSSIBLE");
	Board fullBoard = bs.bitboard[total]; // state of entire game
	printf("\n\t\t\t\t\t OK POSSIBLE");
	Board self = 0;
	//Board opponent;
	Board b; // tempoary board to store where the piece is
	Board vectors = 0; // vector board which will contain all possible moveable to squares
	
	isBlack?(self = bs.bitboard[black]) : (self = bs.bitboard[total] ^ bs.bitboard[black]);
	
	//!isBlack?(opponent = bs.bitboard[black]) : (opponent = bs.bitboard[total] ^ bs.bitboard[black]);
	
	isBlack ? (b = bs.bitboard[black]) : (b = bs.bitboard[total] ^ bs.bitboard[black]); // filter the board to either consider black or white pieces
	//isBlack ? (printf("\nPlaying as black\n")):(printf("\nPlaying as white\n"));
	struct MoveStack* root = NULL;
	printf("\n\t\t\t\t\t OK POSSIBLE");
	
	// for each square
	while(!isEmpty(from))
	{
		printf("\n\t\t\t\t\t\t OK POSSIBLE");
		U8 i = pop(&from); // position of a piece that can be moved.
		b = 1ULL<<i; // make a board that only has 1 bit to move set.
		
		if((b>>i)&1) // if the bit is set
		{
			char piece = 'A';	
			// then check what type of piece it it
			if((b&1ULL<<i)&(bs.bitboard[pawn])) // if it is a pawn
			{
				piece = 'P';
				
				if(isBlack)
				{
					if(blackPawnMovement(i, i-8, bs)) // can it move down one square
					{
						//printf("\n This pawn can move forward one space");
						vectors |= (1ULL<<(i-8))^((1ULL<<(i-8))&self);
					}
					if(blackPawnMovement(i, i-16, bs)) // can it move down two square
					{
						//printf("\n This pawn can move forward two spaces");
						vectors |= (1ULL<<(i-16))^((1ULL<<(i-16))&self);
					}
				}
				else
				{
					if(whitePawnMovement(i, i+8, bs)) // can it move up one square
					{
						//printf("\n This pawn can move forward one space");
						vectors |= (1ULL<<(i+8))^((1ULL<<(i+8))&self);
					}
					if(blackPawnMovement(i, i+16, bs)) // can it move up two square
					{
						//printf("\n This pawn can move forward two spaces");
						vectors |= (1ULL<<(i+16))^((1ULL<<(i+16))&self);
					}
				}
				
				//attaqcks
				isBlack ? ((vectors |= blackPawnAttackVectors(i))):(vectors |= whitePawnAttackVectors(i));
				
			}
			else if((b&1ULL<<i)&(bs.bitboard[bishop])) // if it is a bishop
			{
				piece = 'B';
				vectors |= bishopAttackVectors(i, fullBoard)^(bishopAttackVectors(i, fullBoard)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[knight])) // if it is a knight
			{
				piece = 'N';
				vectors |= knightAttackVectors(i)^(knightAttackVectors(i)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[rook])) // if it is a rook
			{
				piece = 'R';
				vectors |= (rookAttackVectors(i, fullBoard)^((rookAttackVectors(i, fullBoard)&self)));
			}
			else if((b&1ULL<<i)&(bs.bitboard[king])) // if it is a king
			{
				piece = 'K';
				vectors |= kingAttackVectors(i)^(kingAttackVectors(i)&self);
			}
			else if((b&1ULL<<i)&(bs.bitboard[queen])) // if it is a queen
			{
				piece = 'Q';
				vectors |= queenAttackVectors(i, fullBoard)^(queenAttackVectors(i, fullBoard)&self);
			}
			else{
				printf("\n\t UNKNOWN PIECE");
			}
		}
	}
	printf("\nAll of the squares that can be moved to");
	debugPrintBoard(vectors);
	// pass these squares back 
	for(int i = 0 ; i < 63; i++)
	{
		if((vectors>>i)&1)
		{
			push(&root, i);
		}
	}
	free(from);
	return root;
}

struct MoveStack* possibleMoveTo(Boardstate bs, bool isBlack, U8 *from, int size)
{
	struct MoveStack* root = NULL;
	
	Board fullBoard = bs.bitboard[total]; // state of entire game
	
	//debugPrintBoard(fullBoard);
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


struct MoveStack* moveablePieces(Boardstate bs, bool isBlack)
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

bool
negaMax(int depth, float score, bool isBlack, Boardstate bs, Coord *coord1, Coord *coord2)
{
	//while(!isEmpty(availablePieces))
	//{
	//	printf("\nPOP <%d>", pop(&availablePieces));
	//}
	printf("\nCURRENT DEPTH = %d", depth);
	printf("\nSCORE = %.3f", score);
	printBoard(bs.bitboard, isBlack);
	if(depth == 0)
	{
		return true;
	}
	
	// get a stack of available pieces to move
	struct MoveStack* availablePieces = moveablePieces(bs, isBlack);
	
	// array that will hold the squares that can be moved from
	U8 from [getSize(availablePieces)];
	
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
	
	//struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, &from);
	//whatIsInThis(&from, sizeof(from)/sizeof(from[0]));
	struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, &from, sizeof(from));
	
	//printf("######%d", getSize(test));
	// create array to store these squares that they can move to
	U8 to [getSize(toSquares)];
	
	int x = 0;
	// while there are squares that they can move to, assign to to array

	
	while(!isEmpty(toSquares))
	{
		//printf("\n poping ");
		to[x++] = pop(&toSquares);
	}
	

	printf("\nprinting out the array of to positions = [");
	for(int j = 0 ; j < sizeof(to); j++)
	{
		printf("%d ", to[j]);
	}
	printf("]\n");
	
	free(toSquares);
	
	printf("\nShould be finished subsetting");
	
	
	int max = -99999;
	int c = 0;
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
		
		//*coord1 = 7 - l1 + 'A' + (n1-1)*8;
		*coord1 = from[rand()%sizeof(from)];
		
		////printf("\n\t\tCoord2");
		
		//*coord2 = 7 - l2 + 'A' + (n2-1)*8;
		*coord2 = to[rand()%sizeof(to)];	
		
		//printf("\ncoord1 : %d & coord2 : %d", *coord1, *coord2);
		c++;
		if(fauxMove(*coord1, *coord2, isBlack, bs, newbs))
		{
			attempts++;
			newAdvantage = calculateAdvantage(*newbs);
			if(newAdvantage <= bestAdvantage){
				printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
				bestAdvantage = newAdvantage;
				bestCord1 = *coord1;
				bestCord2 = *coord2;
			}
		}
		else{
			//free(newbs);
		}
		
	}
	printf("\nIterations: %d", c++);
	*coord1 = bestCord1;
	*coord2 = bestCord2;
	
	Boardstate newerbs = *newbs;
	
	free(newbs);
	//free(from);
	
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
