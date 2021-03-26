#include "gamemodes.h"

U64 keytable[MAXTURNS+1]; // use this to check if a the same boardstate has been seen 3 times for drawing, plus 1 for tracking length

void
setKeytable(U64 firstkey)
{
	memset(keytable, 0, sizeof(keytable)); // reset the table
	keytable[0] = firstkey;
	keytable[MAXTURNS+1] = 1;
}

void
isrt()
{
	// binary search then memmove and insert
	U16 n = keytable[MAXTURNS+1]-1;
	U16 m, l = 0, r = n;
	U64 x = keytable[r];
	
	while(l<=r) {
		m = l + (r - l) / 2;
		if (x >= keytable[m] && (x <= keytable[m+1] || keytable[m+1]==0)) { // append
			if ((++m)!=n)
				memmove(keytable+m+1, keytable+m, (n-m)*8);
			keytable[m] = x;
			break;
		}

		if (keytable[m] < x) {
			l = m + 1;
		} else {
			if (m == 0) { // prepend
				memmove(keytable+m+1, keytable+m, (n-m)*8);
				keytable[m] = x;
				break;
			}
			r = m - 1;
		}
	}
}

bool
triplication()
{
	U8 jmp;
	for (U16 i=0; i < (U16) keytable[MAXTURNS+1]-3; i=i+jmp+1) { // check for triples
		jmp = 0;
		if (keytable[i]==keytable[i+1]) {
			jmp++; // we can skip an element we've already checked
			if (keytable[i]==keytable[i+2]) {
				printf("Key found in 3s: %llu\n", keytable[i]);
				return true; // found 3 in a row
			}
		}
	}
	return false;
}

void
keytableUpkeep(Boardstate *currBoard)
{
	currBoard->key = generateHash(*currBoard);
	// sort table
	keytable[keytable[MAXTURNS+1]] = currBoard->key; // put at the end of the table
	keytable[MAXTURNS+1]++; // increase length
	isrt();
}

/*
------------------- END OF KEYTABLE FUNCTIONS -------------------
*/

//TODO get these functions to return something different depending on who wins?

void
localMultiplayer(Boardstate *currBoard)
{
	setKeytable(currBoard->key);
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
		if (!movePiece(from, to, true, getPromotion)) continue; // actually move the piece and update currBoard

		currBoard->blackplaying=!currBoard->blackplaying; // switch players
		if(inCheckMate(*currBoard)) {
			prettyPrintBoard(*currBoard);
			puts("Checkmate");
			break; // end game
		}

		keytableUpkeep(currBoard);

		// Stalemate, 50 moves have passed since a pawn moved/piece was taken, or the same boardstate is seen thrice
		if(inStaleMate(*currBoard) || currBoard->halfmove-currBoard->fiftymove==50 || triplication()) {
			prettyPrintBoard(*currBoard);
			puts("Stalemate");
			break; // end game
		}
		puts(currBoard->blackplaying?"\nBlack to play":"\nWhite to play");
		if(inCheck(*currBoard)) puts("Check");
		prettyPrintBoard(*currBoard);
	}

	return;
}


void
localAI(Boardstate *currBoard)
{
	setKeytable(currBoard->key);
	currBoard->blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
	prettyPrintBoard(*currBoard);

	U8 result = 0;
	const U8 strsize = 6; // largest input that will be accepted by stdin
	char s[strsize]; // input string
	Coord from, to; // movements
	U8 (*promotion)();

	char *whiteply = "\nWhite to play";
	char *blackply = "\nBlack to play";

	if (currBoard->blackplaying) { // to show that the AI is playing
		whiteply = "\nAI to play";
	} else	blackply = "\nAI to play";

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
		
		keytableUpkeep(currBoard);

		// Stalemate, 50 moves have passed since a pawn moved/piece was taken, or the same boardstate is seen thrice
		if(inStaleMate(*currBoard) || currBoard->halfmove-currBoard->fiftymove==50 || triplication()) {
			prettyPrintBoard(*currBoard);
			puts("Stalemate");
			break; // end game
		}
		puts(currBoard->blackplaying?blackply:whiteply);
		if(inCheck(*currBoard)) puts("Check");
		prettyPrintBoard(*currBoard);
	}
	return;
}
