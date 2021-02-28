#include "gamemodes.h"

void
localMultiplayer(Boardstate *currBoard, Boardstate *prevBoard)
{
	puts("White to play");
	printBoard(*currBoard);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	U8 result = 0;
	pgnoutput po = makePGN("1", "White", "Black");

	const U8 strsize = 6;
	char s[strsize];
	Coord from, to;

	for(;;) { // strange things are happening 
		from = 0;
		to = 0;
		readInput(s, strsize);
		result = parseInput(s, &from, &to);
		if (result==2) break;
		if (!result) continue;
		movePiece(from, to);

		currBoard->blackplaying=!currBoard->blackplaying; // switch players
		appendMovePGN(*prevBoard, *currBoard, &po, from, to);
		if(inCheckMate(*currBoard)) {
			printBoard(*currBoard);
			puts("Checkmate");
			break; // end game
		}
		if(inStaleMate(*currBoard)) {
			printBoard(*currBoard);
			puts("Stalemate");
			break; // end game
		}
		currBoard->blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		if(inCheck(*currBoard)) puts("Check");
		printBoard(*currBoard);
	}

	dumpPGN(*currBoard, po);

	return;
}
