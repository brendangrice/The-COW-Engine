#include "shit_chess.h"

////
// Maybe just update total at the end of each move instead of updating it 
// alongside all the other pieces and movements with one or.
////

// Bitboards for each piece, 1 for every player past one, 1 for attack spaces and 1 more for the total board. 
extern U64 bitboard[CHESSPIECES+(PLAYERS-1)+2+1]; 
// Ugly global makes this work
U8 movementFlags; 

void
setBitBoard()
{
	for (int i = 0; i < CHESSPIECES + PLAYERS + 1; i++) {
		bitboard[i] = 0;		
	}
	
	// encoded chess board
	// First half is black, second is white
	bitboard[pawn] = 0x00FF00000000FF00;
	bitboard[rook] = 0x8100000000000081;
	bitboard[knight] = 0x4200000000000042;
	bitboard[bishop] = 0x2400000000000024;
	bitboard[queen] = 0x1000000000000010;
	bitboard[king] = 0x0800000000000008;
	bitboard[black] = 0xFFFF000000000000;
	bitboard[total] = bitboard[pawn]|bitboard[rook]|bitboard[knight]|bitboard[bishop]|bitboard[queen]|bitboard[king];
	
	movementFlags=0xF0; // set castling to be available for both players
}

int 
main() 
{
	setBitBoard();
	puts("White to play");
	printBoard(bitboard[total]);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	bool blackplaying = false;
	bool pass;
	U8 from, to;
	for(;;) { // strange things are happening 
LOOP: // works ok to me
		pass = parseInput(&from, &to);
		if (!pass) goto LOOP;
		pass = movePiece(from, to, blackplaying);
		if (!pass) goto LOOP;
		blackplaying=!blackplaying;
		blackplaying?puts("\nBlack to play"):puts("\nWhite to play");

		printBoard(bitboard[pawn]);
	}
	return(0);
}

char 
findPiece(U8 pos, U8 *piece, bool *colourblack)
{
	U64 p = 1;
	char out = 'A'; // temp for checking for errors
	*colourblack = false;
	U64 bit = bitboard[total]&(p<<pos);
	if (bit) {
		if(bit&bitboard[pawn]) {
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
		if(bit&bitboard[black]) {
		       	*colourblack=true;
			out += 32;
		}
	} else {
		*piece=nopiece;
		out = '.';	
	}
	return out;
}

U64
calculateAttackVectors(bool black)
{
	char piece;
	U64 vector = 0;
	U8 a;
	bool b;
	for (int i = 0; i < 64; i++) {
		if ((piece=findPiece(i, &a, &b))!='.') {
			if (piece == 'P' & !black) vector |= whitePawnAttackVectors(i);
			if (piece == 'p' & black) vector |= blackPawnAttackVectors(i);
			piece-=32*black;
			switch(piece)
			{
				case('R'):
					vector |= rookAttackVectors(i);
					break;
				case('N'):
					vector |= knightAttackVectors(i);
					break;
				case('B'):
					vector |= bishopAttackVectors(i);
					break;
				case('Q'):
					vector |= queenAttackVectors(i);
					break;
				case('K'):
					vector |= kingAttackVectors(i);
					break;
			}
		}
	}
	return vector;
}

void
printBoard(U64 b)
{
	U64 board = b;

#ifdef DEBUG //compile with debug to see boards as just bits
	for (int i = 1; i <= 8; i++) { // go through the long long and make bytes
		printBits(board>>(64-(i*8))&0xFF);
	}
	puts("");
#endif
#ifndef DEBUG	
	U8 a;
	bool a2;
	findPiece(63,&a,&a2);
	for (int i = 0; i < 64; i++) {
		if (!(i%8))  { 
			putchar('\n');
			putchar('8'-(i/8));
			putchar(' ');
		}
		putchar(findPiece(63-i,&a,&a2));
	}
	puts("\n\n% ABCDEFGH\n");
#endif
}

void 
printBits(U8 byte) 
{
	for (int i = 0; i < 8; i++) {
		putchar(byte&128?'1':'0');
		byte<<=1;	
	}
	putchar('\n');
}

bool
parseInput(U8 *coord1, U8 *coord2) 
{
	char l1, l2;
	int n1, n2;
	scanf("%c%d%c%d", &l1, &n1, &l2, &n2);

	if (l1<'A' || l1>'H') return false;
	if (l2<'A' || l2>'H') return false;

	if (n1<1 || n1>8) return false;
	if (n2<1 || n2>8) return false;

	*coord1 = 7 - l1 + 'A' + (n1-1)*8;
	*coord2 = 7 - l2 + 'A' + (n2-1)*8;
	
	return true;
}

bool
movePiece(U8 from, U8 to, bool moveblack)
{
	U64 p = 1;
	bool test;
	
	U8 frompiece;
	bool fromcolourblack;

	U8 topiece;
	bool tocolourblack;

	U8 passantpiece;
	bool passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack);
	findPiece(to, &topiece, &tocolourblack);

	if (moveblack-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&topiece!=nopiece) return false; // if the from piece doesn't exist or both are the same colour.
	//do testing based on piece
	switch(frompiece) {
		case(pawn):
			if (fromcolourblack) {
				test = blackPawnMovement(from, to);
				if (test&2) { //en passant logic
					findPiece(to+8, &passantpiece, &passantcolourblack);	
					bitboard[total]^=p<<(to+8);
				}
			} else {
				test = whitePawnMovement(from, to);
				if (test&2) {
					
					findPiece(to-8, &passantpiece, &passantcolourblack);	
					bitboard[total]^=p<<(to-8);
				}
			}
			// pawn has reached the end of the board 
			if (test && (0==to>>3 || 7==to>>3)) {
				puts("Enter which piece you want (R=1, K=2, B=3, Q=4): ");
				U8 pieceno;
PROMOTION:
				scanf("%d", &pieceno);
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
			test = rookAttackMovement(from, to);
			break;
		case(knight):
			test = knightAttackMovement(from, to);
			break;
		case(bishop):
			test = bishopAttackMovement(from, to); 
			break;
		case(queen):
			test = queenAttackMovement(from, to);
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
			test = kingAttackMovement(from, to);
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

	if (!test) return false;
	// set to piece with opposite colour. remove from piece
	// calc attack vectors
	// if the king is being attacked roll back (no move made)
	if (fromcolourblack) {
		bitboard[black]^=p<<from;	
		bitboard[black]|=p<<to;	
	} else 	bitboard[black]&=(-1^(p<<to)); //not didnt seem to be working
	bitboard[frompiece]^=p<<from; // update piece moving
	if (topiece!=nopiece) bitboard[topiece]^=p<<to; // remove taken piece
	bitboard[frompiece]|=p<<to;
	bitboard[total]^=p<<from; // update total board for tracking/finding pieces
	bitboard[total]|=p<<to;	
	

// bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(!moveblack); returns white/black's threatened king position
	// return a white or black board & with king. If attack 
	// (opposite colour) & with the white/black king its
	//  under threat.

	if (bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(!moveblack)) {
		puts("Check");	
		
		// undo piece movement
		
		if (fromcolourblack) {
			bitboard[black]^=p<<from;	
			bitboard[black]|=p<<to;	
		} else 	bitboard[black]^=p<<to;	
			
		bitboard[frompiece]^=p<<from; 

		if (topiece!=nopiece) bitboard[topiece]^=p<<to; // remove taken piece
		bitboard[frompiece]&=(-1^p<<to);
		bitboard[total]^=p<<from; // update total board for tracking/finding pieces
		bitboard[total]&=(-1^p<<to);

		return false;
	}

	if (frompiece==king) {
		if (from==0x3) movementFlags &= 0x3F;
		if (from==0x3B) movementFlags &= 0xCF;
	}

	if (frompiece==rook) {
		if (from==0x7) movementFlags &= 0x7F;
		if (from==0) movementFlags &= 0xBF;
		if (from==0x3F) movementFlags &= 0xDF;
		if (from==0x38) movementFlags &= 0xEF;
	}

	return true;
}
