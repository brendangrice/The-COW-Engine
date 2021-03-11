#include "types.h"
#include "eval.h"
#include "main.h"

#ifndef AI_H
#define AI_H
bool calculateBestMove(Boardstate bs, bool isBlack, int depth, Coord *coord1, Coord *coord2); // AI logic driver
struct MoveStack* moveablePieces(Boardstate bs, bool isBlack); // Stack of moveable pieces to be considered
float NegaMax(int depth, Boardstate bs, bool isBlack, float alpha, float beta); // Game search algorithm

struct MoveStack* createNode(U8 data);
bool isEmpty(struct MoveStack* root);
void push(struct MoveStack** root, U8 data);
int pop(struct MoveStack** root);
int getSize(struct MoveStack* root);

bool isMoveable(Boardstate bs, int position, char piece, bool isBlack);
struct MoveStack* possibleMoveTo(Boardstate bs, bool isBlack, U8 *from, int size);

#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
