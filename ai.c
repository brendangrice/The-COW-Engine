#include "eval.h"
#include "shit_chess.h"
#include "ai.h"

bool
negaMax(int depth, float score, bool isBlack, Boardstate bs, Coord *coord1, Coord *coord2)
{
	if(depth == 0)
	{ 
		return true;
	}
	int max = -99999;
	Coord bestCord1;
	Coord bestCord2;
	
	Coord *tempcoord1 = malloc(sizeof(Coord));
	Coord *tempcoord2 = malloc(sizeof(Coord));
	
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
	printf("\n The current game state advantage = %.3f\n", currentAdvantage);
	
	printf("\nMaking new board");
	Boardstate *newbs = malloc(BOARDSTATESIZE);
	printf("\nNew board made successfully");
	
	while(attempts < 10)
	{		
		printf("\n\tassign l1");
		l1 = letter[rand()%8];
		printf("\n\tassign l2");
		l2 = letter[rand()%8];
		printf("\n\tassign n1");
		n1 = number[rand()%8];
		printf("\n\tassign n2");
		n2 = number[rand()%8];
		
		printf("\n\t\tCoord1");
		*tempcoord1 = 7 - l1 + 'A' + (n1-1)*8;
		//*coord1 = 7 - l1 + 'A' + (n1-1)*8;
		printf("\n\t\tCoord2");
		*tempcoord2 = 7 - l2 + 'A' + (n2-1)*8;
		//*coord2 = 7 - l2 + 'A' + (n2-1)*8;
		
		
		printf("\nAttempting fauxMove");
		//if(fauxMove(*coord1, *coord2, isBlack, bs, newbs))
		if(fauxMove(*tempcoord1, *tempcoord2, isBlack, bs, newbs))
		{
			attempts++;
			newAdvantage = calculateAdvantage(newbs);
			if(newAdvantage <= bestAdvantage){
				printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
				bestAdvantage = newAdvantage;
				bestCord1 = *tempcoord1;
				bestCord2 = *tempcoord2;
				//bestCord1 = *coord1;
				//bestCord2 = *coord2;
			}
		}
		else{
			//free(newbs);
		}
		
	}
	
	//*coord1 = bestCord1;
	//*coord2 = bestCord2;
	*tempcoord1 = bestCord1;
	*tempcoord2 = bestCord2;	
	
	Boardstate newerbs = *newbs;
	free(newbs);
	score = -negaMax(depth-1, score, !isBlack, newerbs, *coord1, *coord2);
	if(score > max)
		max = score;
	return true;
}

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
			//printf("\n\t\t ## New Advantage = %.3f", newAdvantage);
			if(newAdvantage <= bestAdvantage){
				printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
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
			//printf("\n\t\t ## New Advantage = %.3f", newAdvantage);
			if(newAdvantage <= bestAdvantage){
				printf("\n\t\t\t ## New Best = %.3f", newAdvantage);
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