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

bool ai_main(Boardstate bs, int depth, bool isBlack, Coord *one, Coord *two)
{
	
	if(depth == 0)
	{
		return true; // reached the end coords should be valid
	}
	//int score = -999; // a very small number
	
	
	// generate set of moves that can be made
	
	
	//Tree *tree = NULL;
	//Tree *InsertTree(Boardstate bs, Tree *pointer, int numberOfChildern);
	
	//InsertTree(bs, &tree, 5);
	
	// make a list of different states we can get from this position
	
	
	
	// create the node of the tree. The current gamestate
	struct Node *root = makeNode(bs);
	Boardstate *newbs = malloc(BOARDSTATESIZE);
	// subset sqsuares that can be considered	
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
	struct MoveStack* toSquares = possibleMoveTo(bs, isBlack, &from, sizeof(from));
	// create array to store these squares that they can move to
	U8 to [getSize(toSquares)];
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
	
	
	
	
	
	
	// make a move that could improve the position
	Coord bestCord1;
	Coord bestCord2;
	int attempts = 0;
	traverseTree(root);
	//float bestAdvantage = root->advantage;
	//float bestAdvantage = calculateAdvantage(bs);
	float bestAdvantage = -9999;
	if(isBlack) bestAdvantage = bestAdvantage * -1; 
	printf("\nThe current best advantage at the start = %.3f", bestAdvantage);
	float newAdvantage = bestAdvantage;
	
	
	while(attempts < 50)
	{		
		// make a move from selecting from random squares
		*one = from[rand()%sizeof(from)]; // assign to square
		*two = to[rand()%sizeof(to)];	  // assign from square
	
		// attempt to make this move on a fake board
		if(fauxMove(*one, *two, isBlack, bs, newbs))
		{
			// if the move on the fake board was successful
			attempts++; // increment the number of successful attempts
			
			debugPrintBoard(newbs);
			
			// calculate the score of that fake board
			newAdvantage = calculateAdvantage(*newbs);
			if(newAdvantage > bestAdvantage){
				
				bestAdvantage = newAdvantage;
				bestCord1 = *one;
				bestCord2 = *two;
				printf("\n\t\t\t found a move that improves position = %.3f (%d %d)", bestAdvantage, bestCord1, bestCord2);
				Boardstate newerbs = *newbs;
				addChild(root, newerbs);
				//free(newerbs);
				debugPrintBoard(newbs);
			
				break;	
			}
		}
		else{
			//free(newbs);
		}
		
	}
	*one = bestCord1;
	*two = bestCord2;
	
	//Boardstate newerbs = *newbs;
	free(newbs);
	
	//if(newAdvantage > bestAdvantage)
	//{
		//printf("\n Move to make could be %d %d", bestCord1, bestCord2);
		//score = -negaMax(depth-1, max, !isBlack, newerbs, coord1, coord2);
	//}
	printf("\n#########################################\n");
	traverseTree(root);
	
	printf("\n#########################################\n");
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
