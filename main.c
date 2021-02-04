#include "main.h"

////
// Maybe just update total at the end of each move instead of updating it 
// alongside all the other pieces and movements with one or.
// REMEMBER TO CHECK ALL THE FREE'S
////
                                  

Boardstate currBoard;
char* history[1024];
int step = 0;
const char* SQUARE_ID[64] = {
  "h1 ", "g1 ", "f1 ", "e1 ", "d1 ", "c1 ", "b1 ", "a1 ",
  "h2 ", "g2 ", "f2 ", "e2 ", "d2 ", "c2 ", "b2 ", "a2 ",
  "h3 ", "g3 ", "f3 ", "e3 ", "d3 ", "c3 ", "b3 ", "a3 ",
  "h4 ", "g4 ", "f4 ", "e4 ", "d4 ", "c4 ", "b4 ", "a4 ",
  "h5 ", "g5 ", "f5 ", "e5 ", "d5 ", "c5 ", "b5 ", "a5 ",
  "h6 ", "g6 ", "f6 ", "e6 ", "d6 ", "c6 ", "b6 ", "a6 ",
  "h7 ", "g7 ", "f7 ", "e7 ", "d7 ", "c7 ", "b7 ", "a7 ",
  "h8 ", "g8 ", "f8 ", "e8 ", "d8 ", "c8 ", "b8 ", "a8 "
}; 


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
	setBitBoard();
	puts("White to play");
	printBoard(currBoard.bitboard, false);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	bool blackplaying = false;
	bool test;
	//float advantage;
	U8 from, to;
	for(;;) { // strange things are happening 
LOOP: // works ok to me


		// print the current advantage
		//advantage = calculateAdvantage(currBoard.bitboard);
		//printf("\nAdvantage = %.3f\n", calculateAdvantage(currBoard));
		
		if(!blackplaying)
		{
			test = parseInput(&from, &to);
		}
		else{
			test = negaMax(9, calculateAdvantage(currBoard), true, currBoard, &from, &to);
		}
		
		if (!test) goto LOOP;
		test = movePiece(from, to, blackplaying);
		if (!test) goto LOOP;
		blackplaying=!blackplaying; // switch players
		blackplaying?puts("\nBlack to play"):puts("\nWhite to play");

		printBoard(currBoard.bitboard, blackplaying);
	}
	return(0);
}

char 
findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard) //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece.
{
	U64 p = 1LL;
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
	U8 a; // unused
	bool b; // unused
	for (int i = 0; i < 64; i++) {
		if ((piece=findPiece(i, &a, &b, bitboard))!='.') { // find every piece that isn't a blank piece
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
//#endif
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
	U8 a; // unused
	bool a2; // unused
	if (printblack) { // print the board upside-down
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('1'+(i/8));
				putchar(' ');
			}
			putchar(findPiece(i, &a, &a2, bitboard));
		}
		puts("\n\n% HGFEDCBA\n");
	} else {
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('8'-(i/8));
				putchar(' ');
			}
			putchar(findPiece(63-i, &a, &a2, bitboard));
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

void
generatePGN(Coord square, bool isBlack)
{
	
	char str[10];
	
	if(!isBlack)
	{
		++step;
		//sprintf(str, "%d", step);
		strcat(str, ". ");
		strcat(history, str);
	}
	// what piece is this
	
	strcat(history, SQUARE_ID[square]);
	//printf("\n%s", history);
	
	/*
	const char* move[8];
	//const char* step = "X. ";
	if(!isBlack)
	{
		const char* step = "X. ";
		strcpy(move, step);
	}
	strcpy(move, SQUARE_ID[square]);
	strcpy(history, move);
	//printf("\n%s", history);
	*/
}

bool 
movePiece(Coord from, Coord to, bool moveblack) 
{
	Boardstate *newbs = malloc(BOARDSTATESIZE); // new boardstate
	if (!fauxMove(from, to, moveblack, currBoard, newbs)) {
		free(newbs);
		return false;
	}

	currBoard.movementflags = newbs->movementflags;
	memcpy(currBoard.bitboard, newbs->bitboard, BITBOARDSIZE); // copy across the new values to the global bitboard

	free(newbs->bitboard);
	free(newbs);
	generatePGN(to, moveblack);
	
	return true;
}

bool
fauxMove(Coord from, Coord to, bool moveblack, Boardstate bs, Boardstate *nbs)
{
	*nbs = bs;

	U8 movementflags = bs.movementflags;
	Board *bitboard = malloc(BITBOARDSIZE);
	memcpy(bitboard, bs.bitboard, BITBOARDSIZE);
	
	U64 p = 1LL;;
	bool test = false;
	
	Coord frompiece;
	bool fromcolourblack;

	Coord topiece;
	bool tocolourblack;

	Coord passantpiece;
	bool passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, bitboard);
	findPiece(to, &topiece, &tocolourblack, bitboard);

	if (moveblack-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&(topiece!=nopiece)) {
	       free(bitboard);
       	       return false; // if the from piece doesn't exist or both are the same colour.
	}
	//do testing based on piece
	switch(frompiece) {
		case(pawn):
			if (fromcolourblack) {
				test = blackPawnMovement(from, to, *nbs);
				if (test&2) { //en passant logic
					findPiece(to+8, &passantpiece, &passantcolourblack, bitboard);	
					bitboard[total]^=p<<(to+8);
				}
			} else {
				test = whitePawnMovement(from, to, *nbs);
				if (test&2) {
					findPiece(to-8, &passantpiece, &passantcolourblack, bitboard);	
					bitboard[total]^=p<<(to-8);
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
			test = rookMovement(from, to, bitboard);
			break;
		case(knight):
			test = knightMovement(from, to);
			break;
		case(bishop):
			test = bishopMovement(from, to, bitboard); 
			break;
		case(queen):
			test = queenMovement(from, to, bitboard);
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
					bitboard[rook]^=p<<7;
					bitboard[rook]|=p<<4;
					bitboard[total]^=p<<7;
					bitboard[total]|=p<<4;
					break;
				case(3):
					bitboard[rook]^=0;
					bitboard[rook]|=p<<2;
					bitboard[total]^=0;
					bitboard[total]|=p<<2;
					break;
				case(4): // need to update black bitboard too
					bitboard[black]^=p<<63;
					bitboard[black]|=p<<60;
					bitboard[rook]^=p<<63;
					bitboard[rook]|=p<<60;
					bitboard[total]^=p<<63;
					bitboard[total]|=p<<60;
					break;
				case(5):
					bitboard[black]^=p<<56;
					bitboard[black]|=p<<58;
					bitboard[rook]^=p<<56;
					bitboard[rook]|=p<<58;
					bitboard[total]^=p<<56;
					bitboard[total]|=p<<58;
					break;
			}
			break;
	}

	if (!test) {
		free(bitboard);
		return false;
	}
	// set to piece with opposite colour. remove from piece
	// calc attack vectors
	// if the king is being attacked roll back (no move made)
	//
	// tries to update a piece's position
	if (fromcolourblack) {
		bitboard[black]^=p<<from;	// remove the old piece's position
		bitboard[black]|=p<<to;		// update it to the new spot
	} else 	bitboard[black]&=(-1^(p<<to)); // if the new piece is white it needs to be removed from the black board
	bitboard[frompiece]^=p<<from; // update piece moving
	if (topiece!=nopiece) bitboard[topiece]^=p<<to; // remove taken piece
	bitboard[frompiece]|=p<<to; // move the from piece
	bitboard[total]^=p<<from; // update total board for tracking/finding pieces
	bitboard[total]|=p<<to;	
	

// bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(!moveblack); returns white/black's threatened king position
	// white or black board (player colour) & with king. If attack 
	// (opposite colour) & with the white/black king its under threat.

	// this should be rewritten to be more robust and predictive
	if (bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(bitboard, !moveblack)) {
		//puts("Check");	
		
		// if the king is under attack we need to undo piece movement
		
		if (fromcolourblack) {
			bitboard[black]^=p<<from;	
			bitboard[black]|=p<<to;	
		} else 	bitboard[black]^=p<<to;	
			
		bitboard[frompiece]^=p<<from; 

		if (topiece!=nopiece) bitboard[topiece]^=p<<to; // remove taken piece
		bitboard[frompiece]&=(-1^p<<to);
		bitboard[total]^=p<<from; // update total board for tracking/finding pieces
		bitboard[total]&=(-1^p<<to);
		
		free(bitboard);
		return false;
	}

	// if a king or rook succesfully moved
	if (frompiece==king) {
		if (from==0x3) movementflags &= 0x3F; // can't castle either way
		if (from==0x3B) movementflags &= 0xCF;
	}

	if (frompiece==rook) {
		if (from==0x7) movementflags &= 0x7F; // cant castle left anymore
		if (from==0) movementflags &= 0xBF; // cant castle right anymore
		if (from==0x3F) movementflags &= 0xDF;
		if (from==0x38) movementflags &= 0xEF;
	}

	Boardstate tempbs = {movementflags, bitboard}; // package the new states together to be returned
	*nbs = tempbs; // returned with new boardstate

	return true;
}
