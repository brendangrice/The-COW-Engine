#include "main.h"

////
// Maybe just update total at the end of each move instead of updating it 
// alongside all the other pieces and movements with one or.
// REMEMBER TO CHECK ALL THE FREE'S
////

Boardstate currBoard;

void
setBitBoard()
{
	currBoard.movementflags=0xF0; // set castling to be available for both players
	currBoard.bitboard = malloc(BITBOARDSIZE);

	for (int i = 0; i < BITBOARDELEMENTS; i++) {
		currBoard.bitboard[i] = 0;		
	}
	
	// encoded chess board
	// First half is black, second is white
	currBoard.bitboard[pawn] = 0x00FF00000000FF00;
	currBoard.bitboard[rook] = 0x8100000000000081;
	currBoard.bitboard[knight] = 0x4200000000000042;
	currBoard.bitboard[bishop] = 0x2400000000000024;
	currBoard.bitboard[queen] = 0x1000000000000010;
	currBoard.bitboard[king] = 0x0800000000000008;
	currBoard.bitboard[black] = 0xFFFF000000000000;
	currBoard.bitboard[total] = currBoard.bitboard[pawn]|currBoard.bitboard[rook]|currBoard.bitboard[knight]|currBoard.bitboard[bishop]|currBoard.bitboard[queen]|currBoard.bitboard[king];
}

int 
main() 
{
	while (1) {
	setBitBoard();
	puts("Select your gamemode.");
	puts("Local multiplayer: 1");
	puts("Quit: q");
	
	char c = getc(stdin);

		switch(c) {
			case('1'):
				localMultiplayer();
				break;

			case('q'):
				return 0;
				break;
			case('Q'):
				return 0;
				break;
		}
	}
}

void
localMultiplayer()
{
	puts("White to play");
	printBoard(currBoard.bitboard, false);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	bool blackplaying = false;
	bool test;
	U8 from, to, stuff;
	for(;;) { // strange things are happening 
LOOP: // works ok to me
		test = parseInput(&from, &to);
		if (!test) goto LOOP;
		test = movePiece(from, to, blackplaying);
		if (!test) goto LOOP;
		blackplaying=!blackplaying; // switch players
		if((stuff = (inCheckMate(currBoard, blackplaying)))==5) {
			printBoard(currBoard.bitboard, blackplaying);
			puts("Checkmate");
			break; // end game
		}
		fprintf(stdout, "%d\n", stuff);
		blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		if(inCheck(currBoard.bitboard, blackplaying)) puts("Check");
		printBoard(currBoard.bitboard, blackplaying);
	}
	return;
}

//add something here for not having to take piece or black
char 
findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard) //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece.
{
	U8 extra;
	if (piece == NULL) piece = &extra;
	if (colourblack == NULL) colourblack = (bool *) &extra;
	U64 p = 1ULL;
	char out = 'A'; // temp for checking for errors
	*colourblack = false;
	U64 bit = bitboard[total]&(p<<pos); // gets whatever piece is at the location
	if (bit) {
		if(bit&bitboard[pawn]) { // ands bit to see if the piece is present on this board
			*piece=pawn;
		       	out = 'P';
		}
		if(bit&bitboard[rook]) {
			*piece=rook;
		       	out = 'R';
		}
		if(bit&bitboard[knight]) {
			*piece=knight;
		       	out = 'N';
		}
		if(bit&bitboard[bishop]) { 
			*piece=bishop;
			out = 'B';
		}
		if(bit&bitboard[queen]) {
			*piece=queen;
			out = 'Q';
		}
		if(bit&bitboard[king]) {
			*piece=king;
			out = 'K';
		}
		if(bit&bitboard[black]) { // if its a black piece
		       	*colourblack=true;
			out += 32; // change the capital letter of a piece to a lowercase for black
		}
	} else { // if there's no piece
		*piece=nopiece; 
		out = '.';	
	}
	return out;
}

Board
calculateAttackVectors(Board *bitboard, bool black) //returns an attack vector for a colour
{
	char piece;
	Board vector = 0;
	for (int i = 0; i < 64; i++) {
		if ((piece=findPiece(i, NULL, NULL, bitboard))!='.') { // find every piece that isn't a blank piece
			if ((piece == 'P') & (!black)) vector |= whitePawnAttackVectors(i); // white and black have different attacks
			if ((piece == 'p') & black) vector |= blackPawnAttackVectors(i);
			piece-=32*black; //if the piece is black subtract so that the char goes into the switch cases nicely
			switch(piece) 
			{
				case('R'):
					vector |= rookAttackVectors(i, bitboard);
					break;
				case('N'):
					vector |= knightAttackVectors(i);
					break;
				case('B'):
					vector |= bishopAttackVectors(i, bitboard);
					break;
				case('Q'):
					vector |= queenAttackVectors(i, bitboard);
					break;
				case('K'):
					vector |= kingAttackVectors(i);
					break;
			}
		}
	}
	return vector;
}

#ifdef DEBUG //compile with debug to see boards as just bits
void
debugPrintBoard(Board b) {
	for (int i = 1; i <= 8; i++) { // go through the long long and make bytes
		printBits(b>>(64-(i*8))&0xFF);
	}
	puts("");
}

void 
printBits(U8 byte) // used in debugging
{
	for (int i = 0; i < 8; i++) {
		putchar(byte&128?'1':'0');
		byte<<=1;	
	}
	putchar('\n');
}
#endif

void
printBoard(Board *bitboard, bool printblack)
{
	if (printblack) { // print the board upside-down
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('1'+(i/8));
				putchar(' ');
			}
			putchar(findPiece(i, NULL, NULL, bitboard));
		}
		puts("\n\n% HGFEDCBA\n");
	} else {
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('8'-(i/8));
				putchar(' ');
			}
			putchar(findPiece(63-i, NULL, NULL, bitboard));
		}
		puts("\n\n% ABCDEFGH\n");
	}
}

bool
parseInput(Coord *coord1, Coord *coord2) 
{
	char l1, l2;
	int n1, n2;
	scanf("%c%d%c%d", &l1, &n1, &l2, &n2);

	if (l1>='a' && l1<='h') l1-=32;
	if (l2>='a' && l2<='h') l2-=32;

	if (l1<'A' || l1>'H') return false;
	if (l2<'A' || l2>'H') return false;

	if (n1<1 || n1>8) return false;
	if (n2<1 || n2>8) return false;

	*coord1 = 7 - l1 + 'A' + (n1-1)*8;
	*coord2 = 7 - l2 + 'A' + (n2-1)*8;
	
	return true;
}

bool 
movePiece(Coord from, Coord to, bool moveblack) 
{
	Boardstate *newbs = malloc(BOARDSTATESIZE); // new boardstate
	newbs->bitboard = malloc(BITBOARDSIZE);
	if (!fauxMove(from, to, moveblack, currBoard, newbs)) {
		free(newbs);
		return false;
	}

	currBoard.movementflags = newbs->movementflags;
	memcpy(currBoard.bitboard, newbs->bitboard, BITBOARDSIZE); // copy across the new values to the global bitboard

	free(newbs->bitboard);
	free(newbs);
	return true;
}

#define FAUXMOVERET(A, B) {free(A->bitboard); free(A); return B;}
bool
fauxMove(Coord from, Coord to, bool moveblack, Boardstate bs, Boardstate *nbs)
{

	Boardstate *extra = malloc(BOARDSTATESIZE); // new boardstate
	extra->bitboard = malloc(BITBOARDSIZE);
	if (nbs==NULL) nbs = extra;
	memcpy(nbs->bitboard, bs.bitboard, BITBOARDSIZE);
	nbs->movementflags = bs.movementflags;

	Coord frompiece, topiece, passantpiece;
	bool fromcolourblack, tocolourblack, passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, nbs->bitboard);
	findPiece(to, &topiece, &tocolourblack, nbs->bitboard);

	if (moveblack-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&(topiece!=nopiece)) {
       	       FAUXMOVERET(extra, false); // Can't move a piece that doesn't exist or take your own piece
	}

	U64 p = 1ULL;
	bool test = false;
	
	//do testing based on piece
	switch(frompiece) {
		case(pawn):
			if (fromcolourblack) {
				test = blackPawnMovement(from, to, *nbs);
				if (test&2) { //en passant logic
					findPiece(to+8, &passantpiece, &passantcolourblack, nbs->bitboard);	
					nbs->bitboard[total]^=p<<(to+8);
				}
			} else {
				test = whitePawnMovement(from, to, *nbs);
				if (test&2) {
					findPiece(to-8, &passantpiece, &passantcolourblack, nbs->bitboard);	
					nbs->bitboard[total]^=p<<(to-8);
				}
			}
			// pawn has reached the end of the board 
			if (test && (0==to>>3 || 7==to>>3)) {
				puts("Enter which piece you want (R=1, K=2, B=3, Q=4): ");
				U8 pieceno;
PROMOTION:
				scanf("%c", &pieceno);
				switch(pieceno) {
					case(rook):
					case(knight):
					case(bishop):
					case(queen):
						frompiece=pieceno;	
						break;
					default:
						goto PROMOTION;
						break;
				}
			}
			break;
		case(rook):
			test = rookMovement(from, to, nbs->bitboard);
			break;
		case(knight):
			test = knightMovement(from, to);
			break;
		case(bishop):
			test = bishopMovement(from, to, nbs->bitboard); 
			break;
		case(queen):
			test = queenMovement(from, to, nbs->bitboard);
			break;
		case(king):
			// return codes:
			// 0 fail
			// 1 normal
			// 2 White left castle
			// 3 White right castle
			// 4 Black left castle
			// 5 Black right castle
			// relevent castle needs to be moved and total updated
			test = kingMovement(from, to, *nbs);
			switch (test) {
				case(2):
					nbs->bitboard[rook]^=p<<7;
					nbs->bitboard[rook]|=p<<4;
					nbs->bitboard[total]^=p<<7;
					nbs->bitboard[total]|=p<<4;
					break;
				case(3):
					nbs->bitboard[rook]^=1;
					nbs->bitboard[rook]|=p<<2;
					nbs->bitboard[total]^=1;
					nbs->bitboard[total]|=p<<2;
					break;
				case(4): // need to update black nbs->bitboard too
					nbs->bitboard[black]^=p<<63;
					nbs->bitboard[black]|=p<<60;
					nbs->bitboard[rook]^=p<<63;
					nbs->bitboard[rook]|=p<<60;
					nbs->bitboard[total]^=p<<63;
					nbs->bitboard[total]|=p<<60;
					break;
				case(5):
					nbs->bitboard[black]^=p<<56;
					nbs->bitboard[black]|=p<<58;
					nbs->bitboard[rook]^=p<<56;
					nbs->bitboard[rook]|=p<<58;
					nbs->bitboard[total]^=p<<56;
					nbs->bitboard[total]|=p<<58;
					break;
			}
			break;
	}

	if (!test) { // if the piece can't move there
		FAUXMOVERET(extra, false);
	}
	// set to piece with opposite colour. remove from piece
	// calc attack vectors
	// if the king is being attacked roll back (no move made)
	//
	// tries to update a piece's position
	if (fromcolourblack) {
		nbs->bitboard[black]^=p<<from;	// remove the old piece's position
		nbs->bitboard[black]|=p<<to;		// update it to the new spot
	} else 	nbs->bitboard[black]&=(-1^(p<<to)); // if the new piece is white it needs to be removed from the black board
	nbs->bitboard[frompiece]^=p<<from; // update piece moving
	if (topiece!=nopiece) nbs->bitboard[topiece]^=p<<to; // remove taken piece
	nbs->bitboard[frompiece]|=p<<to; // move the from piece
	nbs->bitboard[total]^=p<<from; // update total board for tracking/finding pieces
	nbs->bitboard[total]|=p<<to;	

	if (inCheck(nbs->bitboard, moveblack)) {
		//puts("Check");	
		
		// if the king is under attack we need to undo piece movement
		
		if (fromcolourblack) {
			nbs->bitboard[black]^=p<<from;	
			nbs->bitboard[black]|=p<<to;	
		} else 	nbs->bitboard[black]^=p<<to;	
			
		nbs->bitboard[frompiece]^=p<<from; 

		if (topiece!=nopiece) nbs->bitboard[topiece]^=p<<to; // remove taken piece
		nbs->bitboard[frompiece]&=(-1^p<<to);
		nbs->bitboard[total]^=p<<from; // update total board for tracking/finding pieces
		nbs->bitboard[total]&=(-1^p<<to);
		
		FAUXMOVERET(extra, false);
	}

	// if a king or rook succesfully moved
	if (frompiece==king) {
		if (from==0x3) nbs->movementflags &= 0x3F; // can't castle either way
		if (from==0x3B) nbs->movementflags &= 0xCF;
	}

	if (frompiece==rook) {
		if (from==0x7) nbs->movementflags &= 0x7F; // cant castle left anymore
		if (from==0) nbs->movementflags &= 0xBF; // cant castle right anymore
		if (from==0x3F) nbs->movementflags &= 0xDF;
		if (from==0x38) nbs->movementflags &= 0xEF;
	}

	FAUXMOVERET(extra, true);
}

inline Coord btoc(Board b)
{
	if (!b) return 0;
	Coord pos;
	for (pos=0;!(b&1);b>>=1) pos++;
	return pos;
}

// bs.bitboard[king]&(((moveblack-1)&bs.bitboard[total])^bs.bitboard[black])&calculateAttackVectors(!moveblack); returns white/black's threatened king position
// white or black board (player colour) & with king. If attack 
// (opposite colour) & with the white/black king its under threat.

inline bool inCheck(Board *bitboard, bool moveblack) // invert this moveblack
{
	return !!(bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(bitboard, !moveblack));
}

////
//Rewrite to use a bespoke function instead of fauxmove optimised for speed in this case?
////


// checks if the king is in mate
//  -> checks if the king can move to any of its 8 possible moves
//   -> gets every piece of your colour and every potential square the king can be attacked on and keeps trying to move pieces until its out of mate.

bool
inCheckMate(Boardstate bs, bool moveblack)
{
	if (!inCheck(bs.bitboard, moveblack)) return false;
	// Can the King move anywhere
	//         111
	//         1K1
	//         111
	// Find relevent King and get his Coordinates
	Board kingb = bs.bitboard[king]&(((moveblack-1)&bs.bitboard[total])^bs.bitboard[black]); // Board containing just the king in question
	
	Coord kingc = btoc(kingb);
	bool test = false; // check if king can move out of check
	test |= fauxMove(kingc, kingc+9, moveblack, bs, NULL); // if the piece can move its not a check
	test |= fauxMove(kingc, kingc+8, moveblack, bs, NULL);
	test |= fauxMove(kingc, kingc+7, moveblack, bs, NULL);
	test |= fauxMove(kingc, kingc+1, moveblack, bs, NULL);
	test |= fauxMove(kingc, kingc-1, moveblack, bs, NULL);
	test |= fauxMove(kingc, kingc-7, moveblack, bs, NULL);
	test |= fauxMove(kingc, kingc-8, moveblack, bs, NULL);
	test |= fauxMove(kingc, kingc-9, moveblack, bs, NULL);
	if (test) return false; // not checkmate if king can move out of check
	
	//check if any other piece can move to block
	
	Board pieces = kingb^(((moveblack-1)&bs.bitboard[total])^bs.bitboard[black]); //returns a board of all the players pieces not including the king

	Board coverage = queenAttackVectors(kingc, bs.bitboard) | knightAttackVectors(kingc); // every place that can be attacking the king

	Board tempCoverage;

	Coord from, to;

	printBoard(bs.bitboard, 0);
	for (; pieces; pieces&=(pieces-1)) { // iterate through all the possible pieces trying to block
		from = btoc(pieces);
		for (tempCoverage = coverage; tempCoverage; tempCoverage&=(tempCoverage-1)) {
			to = btoc(tempCoverage);	
			if(fauxMove(from, to, moveblack, bs, NULL)) goto MATERETTRUE;
		}
	}

	return false; // couldn't find a potential move


MATERETTRUE: // found a potential move and is exiting

	return true;
}
