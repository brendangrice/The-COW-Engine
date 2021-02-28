#include "gamemodes.h"

// maybe get this to return something different depending on who wins?
void
localMultiplayer(Boardstate *currBoard)
{
	puts("White to play");
	printBoard(*currBoard);

	U8 result = 0;
	const U8 strsize = 6; // largest input that will be accepted by stdin
	char s[strsize]; // input string
	Coord from, to; // movements

	while(1) { 
		from = 0;
		to = 0;
		readInput(s, strsize);
		result = parseInput(s, &from, &to);
		if (result==2) break; // quit
		if (!result) continue; // if it successfully parsed
		movePiece(from, to); // actually move the piece and update currBoard

		currBoard->blackplaying=!currBoard->blackplaying; // switch players
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

	return;
}
