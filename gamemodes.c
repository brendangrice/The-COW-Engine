#include "gamemodes.h"

// maybe get this to return something different depending on who wins?
void
localMultiplayer(Boardstate *currBoard)
{
	puts(currBoard->blackplaying?"\nBlack to play":"\nWhite to play");
	prettyPrintBoard(*currBoard);

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
		movePiece(from, to, true, getPromotion); // actually move the piece and update currBoard

		currBoard->blackplaying=!currBoard->blackplaying; // switch players
		if(inCheckMate(*currBoard)) {
			prettyPrintBoard(*currBoard);
			puts("Checkmate");
			break; // end game
		}
		if(inStaleMate(*currBoard)) {
			prettyPrintBoard(*currBoard);
			puts("Stalemate");
			break; // end game
		}
		currBoard->blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		puts(currBoard->blackplaying?"\nBlack to play":"\nWhite to play");
		if(inCheck(*currBoard)) puts("Check");
		prettyPrintBoard(*currBoard);
	}

	return;
}


void
localAI(Boardstate *currBoard)
{
	currBoard->blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
	prettyPrintBoard(*currBoard);

	U8 result = 0;
	const U8 strsize = 6; // largest input that will be accepted by stdin
	char s[strsize]; // input string
	Coord from, to; // movements
	U8 (*promotion)();

	for(;;) { // strange things are happening 
LOOP: // works ok to me		
		if(!currBoard->blackplaying) // then white must be playing
		{
			puts("\a"); // notify that they are to input
			readInput(s, strsize);
			result = parseInput(s, &from, &to);
			if (result==2) break; // quit
			promotion = getPromotion;
		}
		else{ // otherwise let the bot play
			result = calculateBestMove(*currBoard, currBoard->blackplaying, 4, &from, &to);			
			promotion = piecePromotionAI;
		}

		if (!result) goto LOOP;
		result = movePiece(from, to, true, promotion); // record the game and use whichever promotion function fits
		if (!result) goto LOOP;
		
		currBoard->blackplaying=!currBoard->blackplaying; // switch players
		
		if(inCheckMate(*currBoard)) {
			prettyPrintBoard(*currBoard);
			puts("Checkmate");
			break; // end game
		}
		
		if(inStaleMate(*currBoard)) {
			prettyPrintBoard(*currBoard);
			puts("Stalemate");
			break; // end game
		}
		currBoard->blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		if(inCheck(*currBoard)) puts("Check");
		prettyPrintBoard(*currBoard);
	}
	return;
}
