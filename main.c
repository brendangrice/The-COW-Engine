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
	
	initHash();
	currBoard.movementflags=0xF0; // set castling to be available for both players
	currBoard.blackplaying = false; // game starts with white
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
	
	char c; // input
	while (1) {

	setBitBoard();
	puts("Select your gamemode.");
	puts("Local multiplayer: 1");
	puts("Local AI: 2");
	puts("Quit: q");
	
	scanf("%c", &c);

		switch(c) {
			case('1'):
				localMultiplayer();
				break;
			case('2'):
				localAI();
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

Boardstate
makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying)
{
	Boardstate newbs;
	newbs.bitboard = malloc(BITBOARDSIZE);
	newbs.bitboard = memset(newbs.bitboard, 0, BITBOARDSIZE);
	if (bitboard!=NULL) memcpy(newbs.bitboard, bitboard, BITBOARDSIZE);
	newbs.movementflags = movementflags;
	newbs.blackplaying = blackplaying;
	
	return newbs;
}

Boardstate *
cpyBoardstate(Boardstate *to, Boardstate from)
{
		to->bitboard = memcpy(to->bitboard, from.bitboard, BITBOARDSIZE);
		to->movementflags = from.movementflags;
		to->blackplaying = from.blackplaying;
		return to;
}

void
localAI()
{
	puts("White to play");
	printBoard(currBoard);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	bool test;
	U8 from, to;
	for(;;) { // strange things are happening 
LOOP: // works ok to me		
		if(!currBoard.blackplaying) // then white must be playing
		{
			printf("\a"); // notiy that they are to input
			test = parseInput(&from, &to); //take their input
		}
		else{ // otherwise let the bot play
			test = calculateBestMove(currBoard, currBoard.blackplaying, 3, &from, &to);			
		}
		
		if (!test) goto LOOP;
		test = movePiece(from, to);
		if (!test) goto LOOP;
		
		currBoard.blackplaying=!currBoard.blackplaying; // switch players
		
		if(inCheckMate(currBoard)) {
			printBoard(currBoard);
			puts("Checkmate");
			break; // end game
		}
		
		if(inStaleMate(currBoard)) {
			printBoard(currBoard);
			puts("Stalemate");
			break; // end game
		}
		currBoard.blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		if(inCheck(currBoard)) puts("Check");
		printBoard(currBoard);
	}
	return;
}

void
localMultiplayer()
{
	puts("White to play");
	printBoard(currBoard);
	U64 key = generateHash(currBoard, -1);
	printf("\nKey : %llx\n", key);
	printFEN(currBoard, -1, -1);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	bool test;
	U8 from, to;
	for(;;) { // strange things are happening 
LOOP: // works ok to me
		test = parseInput(&from, &to);
		if (!test) goto LOOP;
		test = movePiece(from, to);
		if (!test) goto LOOP;
		currBoard.blackplaying=!currBoard.blackplaying; // switch players
		if(inCheckMate(currBoard)) {
			printBoard(currBoard);
			printFEN(currBoard, from, to);
			puts("Checkmate");
			break; // end game
		}
		if(inStaleMate(currBoard)) {
			printBoard(currBoard);
			printFEN(currBoard, from, to);
			puts("Stalemate");
			break; // end game
		}
		currBoard.blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		if(inCheck(currBoard)) puts("Check");
		printBoard(currBoard);
		printFEN(currBoard, from, to);
		key = generateHash(currBoard, to);
		printf("\nKey : %llx\n", key);
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
calculateAttackVectors(Board *bitboard, bool blackplaying) //returns an attack vector for a colour
{
	char piece;
	Board vector = 0;
	for (int i = 0; i < 64; i++) {
		if ((piece=findPiece(i, NULL, NULL, bitboard))!='.') { // find every piece that isn't a blank piece
			if ((piece == 'P') & (!blackplaying)) vector |= whitePawnAttackVectors(i); // white and black have different attacks
			if ((piece == 'p') & blackplaying) vector |= blackPawnAttackVectors(i);
			piece-=32*blackplaying; //if the piece is black subtract so that the char goes into the switch cases nicely
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
		putc(byte&128?'1':'0', stdout);
		byte<<=1;	
	}
	putchar('\n');
}
#endif

void printFEN(Boardstate bs, Coord from, Coord to)
{
	int blank = 0;
	int j = 0;	
	// Piece Placement
	for (int i = 63; i>=0; i--) {		
		if (!((i+1)%8))  // is in a new rank
		{
			blank = 0; // ensure that the blank count is zero at start of new rank
			if(i!=63)putchar('/'); // denote rank seperation with "/"
			if(findPiece(i, NULL, NULL, bs.bitboard) == '.') // if it is blank
			{
				j = i; // starting where i is positioned
				// while the character is blank
				while(findPiece(j, NULL, NULL, bs.bitboard) == '.')
				{
					blank++; // increment the count of blank characters
					j--; // decrement the position
					if(!((j+1)%8)) // going to enter a new rank, so break out of this
					{
						j++;
						break;
					}
				}
				// found a non blank characters
				i = j; // move the main position to where j left off to avoid double counting
				printf("%d", blank);
				if(((j)%8))
					putchar(findPiece(j, NULL, NULL, bs.bitboard));
				blank = 0;
			}
			else // otherwise it is not blank
			{
				putchar(findPiece(i, NULL, NULL, bs.bitboard));
			}
		}
		else // it is not a new rank
		{
			if(findPiece(i, NULL, NULL, bs.bitboard) == '.') // if it is blank
			{
				if(findPiece(i, NULL, NULL, bs.bitboard) == '.') // if it is blank
				{
					j = i; // starting where i is positioned
					// while the character is blank
					while(findPiece(j, NULL, NULL, bs.bitboard) == '.')
					{
						blank++; // increment the count of blank characters
						j--; // decrement the position
						if(!((j+1)%8)) // going to enter a new rank, so break out of this
						{
							j++;
							break;
						}
					}
					// found a non blank characters
					i = j; // move the main position to where j left off to avoid double counting
					printf("%d", blank);
					if(((j-1)%8) && findPiece(j, NULL, NULL, bs.bitboard) != '.')
						putchar(findPiece(j, NULL, NULL, bs.bitboard));
					blank = 0;
				}
			}
			else // otherwise it is not blank
			{
				putchar(findPiece(i, NULL, NULL, bs.bitboard));
			}
		}
	}

	// Side to move
	putchar(' ');
	bs.blackplaying ? (putchar('b')):(putchar('w'));
	
	// Castling flags
	putchar(' ');
	if(bs.movementflags&0x80) printf("K");
	if(bs.movementflags&0x40) printf("Q");
	if(bs.movementflags&0x20) printf("k");
	if(bs.movementflags&0x10) printf("q");
	
	// En passant target square
	if(bs.blackplaying && ((from+16) == to) && (findPiece(to, NULL, NULL, bs.bitboard) == 'P'))
	{
		printf(" %c%d", ('h'-(to&7)), ((to>>3)+1) - 1);
	}
	else if(((from-16) == to) && (findPiece(to, NULL, NULL, bs.bitboard) == 'p'))
	{
		printf(" %c%d", ('h'-(to&7)), ((to>>3)+1) + 1);
	}
	else
	{
		printf(" -");
	}
	// TODO make these do something, hardcoded at the moment, so that it will still work in FEN viewers
	// halfmove clock
	printf(" 0");
	// fullmove clock
	printf(" 1");
	printf("\n");
}


void
printBoard(Boardstate bs)
{
	if (bs.blackplaying) { // print the board upside-down
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('1'+(i/8));
				putchar(' ');
			}
			putchar(findPiece(i, NULL, NULL, bs.bitboard));
		}
		puts("\n\n% HGFEDCBA\n");
	} else {
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('8'-(i/8));
				putchar(' ');
			}
			putchar(findPiece(63-i, NULL, NULL, bs.bitboard));
		}
		puts("\n\n% ABCDEFGH\n");
	}
	puts("\n");
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
movePiece(Coord from, Coord to) // works exclusively with the current board
{
	Boardstate newbs = makeBoardstate(NULL, 0, 0); // new boardstate
	if (!fauxMove(from, to, currBoard, &newbs)) {
		free(newbs.bitboard);
		return false;
	}

	cpyBoardstate(&currBoard, newbs);

	free(newbs.bitboard);
	return true;
}

#define FAUXMOVERET(A, B) {free(A.bitboard); return B;}
bool
fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs)
{
	Boardstate extra = makeBoardstate(NULL, 0, 0); // new boardstate
	if (nbs==NULL) nbs = &extra;
	nbs = cpyBoardstate(nbs, bs);
	Coord frompiece, topiece, passantpiece;
	bool fromcolourblack, tocolourblack, passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, nbs->bitboard);
	findPiece(to, &topiece, &tocolourblack, nbs->bitboard);

	Board moves;

	
	if (bs.blackplaying-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&(topiece!=nopiece)) {
		//printf("\nCan't move a piece that doesn't exist or take your own piece");
       	       FAUXMOVERET(extra, false); // Can't move a piece that doesn't exist or take your own piece
	}

	U64 p = 1ULL;
	bool test = false;
	
	if (frompiece!=pawn) nbs->movementflags&=0xF0; //TODO find a nicer way overall to handle the movement flags for en passant

	//do testing based on piece
	switch(frompiece) {
		case(pawn):
			if (fromcolourblack) {
				test = blackPawnMovement(from, to, *nbs, &moves);
				if (test == 2) { //en passant logic
					findPiece(to+8, &passantpiece, &passantcolourblack, nbs->bitboard);
					nbs->bitboard[total]^=p<<(to+8);
				} 
				if(test == 3) { // update flags for double move
					nbs->movementflags&=0xF0;
					nbs->movementflags|=from%8;
					nbs->movementflags|=0x8;
				} else {
					nbs->movementflags&=0xF0;
				}
			} else {
				test = whitePawnMovement(from, to, *nbs, &moves);
				if (test == 2) { // en passant
					findPiece(to-8, &passantpiece, &passantcolourblack, nbs->bitboard);	
					nbs->bitboard[black]^=p<<(to-8);
					nbs->bitboard[total]^=p<<(to-8);
				}
				if(test == 3) { // update flags for double move
					nbs->movementflags&=0xF0;
					nbs->movementflags|=from%8;
					nbs->movementflags|=0x8;
				} else {
					bs.movementflags&=0xF0;
				}
			}
			// pawn has reached the end of the board 
			// //update this to be ai friendly
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
			test = rookMovement(from, to, *nbs, NULL);
			break;
		case(knight):
			test = knightMovement(from, to, *nbs, NULL);
			break;
		case(bishop):
			test = bishopMovement(from, to, *nbs, NULL); 
			break;
		case(queen):
			test = queenMovement(from, to, *nbs, NULL);
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
			test = kingMovement(from, to, *nbs, NULL);
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
	if (inCheck(*nbs)) {
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

	//puts("c");
	FAUXMOVERET(extra, true);
}

inline Coord btoc(Board b)
{
	if (!b) return 0;
	Coord pos = 0;
	for (pos=0;!(b&1);b>>=1) pos++;
	return pos;
}

// bs.bitboard[king]&(((moveblack-1)&bs.bitboard[total])^bs.bitboard[black])&calculateAttackVectors(!moveblack); returns white/black's threatened king position
// white or black board (player colour) & with king. If attack 
// (opposite colour) & with the white/black king its under threat.

inline Board getPlayerBoard(Board *bitboard, bool blackplaying) {
	return ((blackplaying-1)&bitboard[total])^bitboard[black];
}

inline bool inCheck(Boardstate bs) // invert this moveblack
{
	return !!(bs.bitboard[king]&getPlayerBoard(bs.bitboard, bs.blackplaying)&calculateAttackVectors(bs.bitboard, !bs.blackplaying));
}

////
//Rewrite to use a bespoke function instead of fauxmove optimised for speed in this case?
////


// checks if the king is in mate
//  -> checks if the king can move to any of its 8 possible moves
//   -> gets every piece of your colour and every potential square the king can be attacked on and keeps trying to move pieces until its out of mate.

bool
inCheckMate(Boardstate bs)
{
	if (!inCheck(bs)) return false;
	// Can the King move anywhere
	//         111
	//         1K1
	//         111
	// Find relevent King and get his Coordinates
	
	Board pieces = getPlayerBoard(bs.bitboard, bs.blackplaying); // Board containing just the king in question

	Board kingb = bs.bitboard[king]&pieces;
	
	Coord kingc = btoc(kingb); // position of the king
	
	Board vectors = queenAttackVectors(kingc, bs.bitboard) | knightAttackVectors(kingc); // every position that can be attacking the king

	Board tempcoverage, coverage;

	Coord from, to;

	U8 piece = nopiece;

	for (; pieces; pieces&=(pieces-1)) { // iterate through all the possible pieces trying to block
		from = btoc(pieces);
		findPiece(from, &piece, NULL, bs.bitboard);
		switch(piece) {
			case(pawn):
				if (bs.blackplaying) {
					blackPawnMovement(from, 0, bs, &coverage);
				} else	whitePawnMovement(from, 0, bs, &coverage);
				break;
			case(rook):
				rookMovement(from, 0, bs, &coverage);
				break;
			case(knight):
				knightMovement(from, 0, bs, &coverage);
				break;
			case(bishop):
				bishopMovement(from, 0, bs, &coverage);
				break;
			case(queen):
				queenMovement(from, 0, bs, &coverage);
				break;
			case(king):
				kingMovement(from, 0, bs, &coverage);
				break;
			default:
				coverage = 0;
				break;
		}
		coverage &= vectors; // only tries to move where pieces can move
		for (tempcoverage = coverage; tempcoverage; tempcoverage&=(tempcoverage-1)) {
			to = btoc(tempcoverage);
			if(fauxMove(from, to, bs, NULL)) return false; // if it can stop the mate
		}
	}

	return true; // couldn't find a potential move
}

// 1) Not in check
// 2) Can't move any piece
//  -> Go through every piece, try and generate its movement vector then try and make that move with fauxmove?

bool
inStaleMate(Boardstate bs)
{
	if (inCheck(bs)) return false; // can't be in check and in stalemate
	
	Board pieces = getPlayerBoard(bs.bitboard, bs.blackplaying); //returns a board of all the players pieces

	Board tempcoverage, coverage;

	Coord from, to;

	U8 piece = nopiece;

	for (; pieces; pieces&=(pieces-1)) { // iterate through all the possible pieces trying to move
		from = btoc(pieces);
		// findPiece, switch according to piece and generate coverage board from that
		// maybe split this into its own function
		findPiece(from, &piece, NULL, bs.bitboard);
		switch(piece) {
			case(pawn):
				if (bs.blackplaying) {
					blackPawnMovement(from, 0, bs, &coverage);
				} else	whitePawnMovement(from, 0, bs, &coverage);
				break;
			case(rook):
				rookMovement(from, 0, bs, &coverage);
				break;
			case(knight):
				knightMovement(from, 0, bs, &coverage);
				break;
			case(bishop):
				bishopMovement(from, 0, bs, &coverage);
				break;
			case(queen):
				queenMovement(from, 0, bs, &coverage);
				break;
			case(king):
				kingMovement(from, 0, bs, &coverage);
				break;
			default:
				coverage = 0;
				break;
		}
		for (tempcoverage = coverage; tempcoverage; tempcoverage&=(tempcoverage-1)) {
			to = btoc(tempcoverage);
			if(fauxMove(from, to, bs, NULL)) return false;
		}
	}

	return true; // couldn't find a potential move
}