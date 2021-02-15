#include "main.h"

////
// Maybe just update total at the end of each move instead of updating it 
// alongside all the other pieces and movements with one or.
// REMEMBER TO CHECK ALL THE FREE'S
////

#define CLEARSTDIN while((c=fgetc(stdin)) != EOF && c != '\n');

Boardstate currBoard;

int 
main() 
{
	setBitBoard();

	char c, inp; // input
	while (1) {

		setBitBoard();
		puts("Select your gamemode.");
		puts("Local multiplayer: 1");
		puts("Quit: q");
		
		inp = fgetc(stdin);
		if (inp==EOF) break; //exit
		CLEARSTDIN;
		switch(inp) {
			case('1'):
				localMultiplayer();
				break;
			case('q'):
			case('Q'):
				return 0;
				break;
			default:
				fputs("Bad Input", stdout);
				break;
		}

	}
}

void
setBitBoard()
{
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

Boardstate *
makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying)
{
	Boardstate *newbs = malloc(BOARDSTATESIZE);
	newbs->bitboard = malloc(BITBOARDSIZE);
	newbs->bitboard = memset(newbs->bitboard, 0, BITBOARDSIZE);
	if (bitboard!=NULL) memcpy(newbs->bitboard, bitboard, BITBOARDSIZE);
	newbs->movementflags = movementflags;
	newbs->blackplaying = blackplaying;
	
	return newbs;
}

Boardstate *
cpyBoardstate(Boardstate *to, Boardstate *from)
{
		to->bitboard = memcpy(to->bitboard, from->bitboard, BITBOARDSIZE);
		to->movementflags = from->movementflags;
		to->blackplaying = from->blackplaying;
		return to;
}

void
destroyBoardstate(Boardstate *bs)
{
	free(bs->bitboard);
	bs->bitboard = NULL;
	free(bs);
	bs = NULL;
}

void
localMultiplayer()
{
	puts("White to play");
	printBoard(currBoard);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	U8 from = -1, to = -1, result = 0, temp = -1, pos = 0, pos2 = 0;
	U64 offset;
	Board vector, coverage, tempcoverage;
	bool colour;
	for(;;) { // strange things are happening 
LOOP: // works ok to me
		result = parseInput(&from, &to);
		switch(result) {
			case(255):
				return; // exit
			case(0):
				goto LOOP; // bad input
			case(1):
				if (temp == 255) {
SINGLEINPUT:
					offset = 1ULL<<from;
					if (currBoard.bitboard[total]&offset) { // if this is a piece that exists
						// try to show where the piece can move
						temp = from;
						vector = calculateMovementVector(currBoard, temp);
						findPiece(temp, NULL, &colour, currBoard.bitboard);
						if (!vector || colour!=currBoard.blackplaying) temp = -1; // if it can't move anywhere don't expect it to, or if its the opponents piece you can't move it
						printHighlightBoard(currBoard, vector);
						goto LOOP;
					} else {
						// if only one piece can move to this empty square move it there	
						temp = -1;
						vector = getPlayerBoard(currBoard.bitboard, currBoard.blackplaying);
						for (; vector; vector&=(vector-1)) { // iterate through all the possible pieces trying to move them and see if only one can move
							pos = btoc(vector);
							coverage = calculateMovementVector(currBoard, pos)&offset;
							for (tempcoverage = coverage; tempcoverage; tempcoverage&=(tempcoverage-1)) {
								pos2 = btoc(tempcoverage);
								if(fauxMove(pos, pos2, currBoard, NULL)) {
									if (temp==255) { // if one piece can move set temp and from and to 
										to = from;
										from = pos;	
										temp = 0;
									} else {
										temp = -1;
										to = -1;
										from = -1;
									}
								}
							}
						}
												
					}
				} else { //try to move
					if (!fauxMove(temp, from, currBoard, NULL)) goto SINGLEINPUT; // if its an illegal move its probably meant to be viewing a single input
					to = from; // another half input makes a full input
					from = temp;
				}
				break;
			case(2): // normal input
				break;
		}
		temp = -1;
		if (!movePiece(from, to)) goto LOOP;
		from = -1;
		to = -1;
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

Board
calculateMovementVector(Boardstate bs, Coord pos)
{
	Board vector;
	U8 piece = nopiece;
	bool colour;
	findPiece(pos, &piece, &colour, bs.bitboard);
	switch(piece) {
		case(pawn):
			if (colour) {
				blackPawnMovement(pos, 0, bs, &vector);
			} else	whitePawnMovement(pos, 0, bs, &vector);
			break;
		case(rook):
			rookMovement(pos, 0, bs, &vector);
			break;
		case(knight):
			knightMovement(pos, 0, bs, &vector);
			break;
		case(bishop):
			bishopMovement(pos, 0, bs, &vector);
			break;
		case(queen):
			queenMovement(pos, 0, bs, &vector);
			break;
		case(king):
			kingMovement(pos, 0, bs, &vector);
			break;
		default:
			vector = 0;
			break;
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
		putc(byte&128?'1':'0', stdout);
		byte<<=1;	
	}
	putchar('\n');
}
#endif

void
printBoard(Boardstate bs)
{
	char c;
	if (bs.blackplaying) { // print the board upside-down
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('1'+(i/8));
				putchar(' ');
				putchar(' ');
			}
			c = findPiece(i, NULL, NULL, bs.bitboard);
			if (c=='.' && (i+(i/8))&1) c = ',';
			putchar(c);
			putchar(' ');
		}
		puts("\n\n%  H G F E D C B A\n");
	} else {
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('8'-(i/8));
				putchar(' ');
				putchar(' ');
			}
			c = findPiece(63-i, NULL, NULL, bs.bitboard);
			if (c=='.' && (i+(i/8))&1) c = ',';
			putchar(c);
			putchar(' ');
		}
		puts("\n\n%  A B C D E F G H\n");
	}
}

void
printHighlightBoard(Boardstate bs, Board highlights)
{
	const U8 boardsize = 64; 
	char out[64]; // array for printing
	Coord coord;
	char c;
	coord = btoc(highlights);
	for (int i = 0; i < boardsize; i++) { // store in character array
		c = findPiece(i, NULL, NULL, bs.bitboard);
		if (c=='.' && i == coord) {
			highlights &= highlights-1;
			coord = btoc(highlights);
			c = 'x';
		}
		if (c=='.' && (i+(i/8))&1) c = ',';
		out[i] = c;
	}

	char *row = "\n\n%  A B C D E F G H\n"; // needs to print out differently depending on orientation

	if (bs.blackplaying) {
		row = "\n\n%  H G F E D C B A\n";
	}

	for (int i = 0; i < 64; i++) { // print out formatted
		if (!(i%8)) {
			putchar('\n');
			putchar(bs.blackplaying?'1'+(i/8):'8'-(i/8));
			putchar(' ');
			putchar(' ');
		}
		putchar(bs.blackplaying?out[i]:out[63-i]); // prints out forwards or backwards
		putchar(' ');
	}
	puts(row);
}

U8
parseInput(Coord *from, Coord *to) 
{
	char l1 = 0, n1 = 0, l2 = 0, n2 = 0;

	char c;

	l1 = fgetc(stdin);
	if (l1 == EOF || l1 == 'q' || l1 == 'Q') { // if it reads quit on the first char
		CLEARSTDIN;
		return -1; // quit
	}

	if (l1 == '\n') return 0; // bad input

	n1 = fgetc(stdin);

	if (l1 == 0 || n1 == 0) return 0; // didn't read anything in
	if (l1<'A' || l1>'H' || n1<'1' || n1>'8') return 0;
	if (l1>='a' && l1<='h') l1-=32; // get in proper format
	n1 -= '1';
	*from = 7 - l1 + 'A' + n1*8; // convert to the format used internally (0-63)

	l2 = fgetc(stdin);
	if (l2 == '\n') { // if two character were entered then \n we have a partial input
		return 1;
	}

	n2 = fgetc(stdin); // finally read the last character
	CLEARSTDIN;

	if (l2 == 0 || n2 == 0) return 1; // partial return 
	if (l2<'A' || l2>'H' || n2<'1' || n2>'8') return 1; // partial return
	if (l2>='a' && l2<='h') l2-=32; // get in proper format
	n2 -= '1';
	*to = 7 - l2 + 'A' + n2*8; // convert to the format used internally (0-63)

	return 2; // full return
}

bool 
movePiece(Coord from, Coord to) // works exclusively with the current board
{
	Boardstate *newbs = makeBoardstate(NULL, 0, 0); // new boardstate
	if (!fauxMove(from, to, currBoard, newbs)) {
		destroyBoardstate(newbs);
		return false;
	}

	cpyBoardstate(&currBoard, newbs);

	destroyBoardstate(newbs);
	return true;
}

#define FAUXMOVERET(A, B) {destroyBoardstate(A); return B;}
bool
fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs)
{
	Boardstate *extra = makeBoardstate(NULL, 0, 0); // new boardstate
	if (nbs==NULL) nbs = extra;
	nbs = cpyBoardstate(nbs, &bs);

	Coord frompiece, topiece, passantpiece;
	bool fromcolourblack, tocolourblack, passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, nbs->bitboard);
	findPiece(to, &topiece, &tocolourblack, nbs->bitboard);

	Board moves;

	if (bs.blackplaying-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&(topiece!=nopiece)) {
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
	
	Coord kingc = btoc(kingb);
	
	//check if any other piece can move to break mate
	
	Board vectors = queenAttackVectors(kingc, bs.bitboard) | knightAttackVectors(kingc); // every position that can be attacking the king & the king can move to

	Board tempcoverage, coverage;

	Coord from, to;

	for (; pieces; pieces&=(pieces-1)) { // iterate through all the possible pieces trying to block
		from = btoc(pieces);
		coverage = vectors & calculateMovementVector(bs, from); // only tries to move where pieces can move
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

	for (; pieces; pieces&=(pieces-1)) { // iterate through all the possible pieces trying to move
		from = btoc(pieces);
		// findPiece, switch according to piece and generate coverage board from that
		// maybe split this into its own function
		coverage = calculateMovementVector(bs, from);
		for (tempcoverage = coverage; tempcoverage; tempcoverage&=(tempcoverage-1)) {
			to = btoc(tempcoverage);
			if(fauxMove(from, to, bs, NULL)) return false;
		}
	}

	return true; // couldn't find a potential move
}
