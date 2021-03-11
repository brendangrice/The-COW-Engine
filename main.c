#include "main.h"

Boardstate currBoard;
Boardstate prevBoard;
PGNoutput po;

#define _ nopiece
const U8 ltoe[] = { // lookup table for converting letters to enum representations e.g. ltoe['p'] -> pawn and ltoe['P'] -> pawn
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
	_, _, _, _, _, _, bishop, _, _, _, _, _, _, _, _, king, _, _, knight, _, 
	pawn, queen, rook, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, bishop, _,
       	_, _, _, _, _, _, _, king, _, _, knight, _, pawn, queen, rook

};
#undef _

int 
main() 
{	
	char inp[2]; // input
	while (1) {
		setBitBoard();
		puts("Select your gamemode.");
		puts("Local multiplayer: 1");
		puts("Local AI: 2");
		puts("Quit: q");
		
		readInput(inp, 2);
		if (*inp == 'Q' || *inp == 'q') return 0;
		po = makePGN("1", "white", "black");
		switch(*inp) {
			case('1'):
				localMultiplayer(&currBoard);
				break;
			case('2'):
				localAI(&currBoard);
				break;
			default:
				puts("Bad Input");
				break;
		}
		dumpPGN(currBoard, po);
	}
}

inline void strrep(char *s, char pre, char post) // string replace
{
	for (int i=0; i<strlen(s); i++) if (s[i]==pre) s[i]=post;
}

void strrm(char *s, char rm) // string remove
{
	U8 len = strlen(s);
	char *ptr = s;
	for (int i=0; ptr[i]; i++) {
		if (ptr[i]==rm) {
			ptr++;
			i--;
			len--;
		} else 	s[i] = ptr[i];
	}
	s[len] = 0;
}

void
setBitBoard()
{
	prevBoard = makeBoardstate(NULL, 0xF0, false);
	currBoard = makeBoardstate(NULL, 0xF0, false);

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
void
setBitBoardFromFEN()
{
	char *FEN = DEFAULT;
	currBoard.movementflags=0xF0; // set castling to be available for both players
	currBoard.bitboard = malloc(BITBOARDSIZE);

	for (int i = 0; i < BITBOARDELEMENTS; i++) {
		currBoard.bitboard[i] = 0;		
	}
	currBoard = parseFEN(FEN, currBoard);
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
calculateMovementVector(Boardstate bs, Coord pos) //returns a vector for where a singular piece can move
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

U8
iterateVector(Boardstate bs, Board fromvector, Board tovector, Coord *co, U8 pass) // iterates through a vector of pieces trying to match their movement to another vector, returns the number of matches found
{
	Coord pos, extra = 0;
       	int count = 0, pos2 = 0;
	Board coverage;
	if (co == NULL) co = &extra;

	for (; fromvector; fromvector&=(fromvector-1)) { // iterate through all the possible pieces trying to move them and see how many can move
		pos = btoc(fromvector);
		coverage = calculateMovementVector(bs, pos)&(1ULL<<tovector); // only try to go where it can
		for (coverage = tovector; coverage; coverage&=(coverage-1)) {
			pos2 = btoc(coverage);
			if(fauxMove(pos, pos2, bs, NULL)) {
				*co = pos;
				if (pass&1) return 1;
				if (pass&2 && count==1) return 2;
				count++;
			}
		}
	}
	return count;
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


void prettyPrintBoard(Boardstate bs)
{
	/*
		  +---+---+---+---+---+---+---+---+
		8 | r |   | b | q | k | b | n | r |
		  +---+---+---+---+---+---+---+---+
		7 | p | p | p | p | p | p | p | p |
	 	  +---+---+---+---+---+---+---+---+
		6 |   |   | n |   |   |   |   |   |
		  +---+---+---+---+---+---+---+---+
		5 |   |   |   |   |   |   |   |   |
		  +---+---+---+---+---+---+---+---+
		4 |   |   |   |   | P |   |   |   |
		  +---+---+---+---+---+---+---+---+
		3 |   |   |   |   |   |   |   |   |
		  +---+---+---+---+---+---+---+---+
		2 | P | P | P | P |   | P | P | P |
		  +---+---+---+---+---+---+---+---+
		1 | R | N | B | Q | K | B | N | R |
		  +---+---+---+---+---+---+---+---+
			A   B   C   D   E   F   G   H
	*/
	bool change = false;
	printf("\n   +---+---+---+---+---+---+---+---+");
	for(int i = 0 ; i < 64; i++)
	{
		if(!(i%8))
		{
			bs.blackplaying ? printf("\n %c |",'1'+(i/8)) : printf("\n %c |",'8'-(i/8));;
			change = !change;
			for(int j = i; j < i+8; j++)
			{
				char c  = bs.blackplaying ? findPiece(j, NULL, NULL, bs.bitboard) : findPiece(63-j, NULL, NULL, bs.bitboard);
				if(c == '.') c = ' '; 
				change ? printf(" %c |", c) : printf(" %c |", c); // maybe shade the dark squares in the future?
				change = !change;
			}				
			printf("\n   +---+---+---+---+---+---+---+---+");	
		}
	}
	bs.blackplaying ? printf("\n     H   G   F   E   D   C   B   A  \n") : printf("\n     A   B   C   D   E   F   G   H  \n");
}
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
	} else { // print the board the right way around
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
	puts("\n");
}


Boardstate
parseFEN(char *FEN, Boardstate bs)
{
	// reset the board
	for(int i = 0 ; i < BITBOARDELEMENTS; i++)
	{
		bs.bitboard[i] = 0;
	}
	
	bs.movementflags = 0;
	bs.blackplaying = false;
	
	//printBoard(bs);
	
	// AFILE 8080 8080 8080 8080
	// BFILE 4040 4040 4040 4040
	// CFILE 2020 2020 2020 2020
	// DFILE 1010 1010 1010 1010
	// EFILE 0808 0808 0808 0808
	// FFILE 0404 0404 0404 0404
	// GFILE 0202 0202 0202 0202
	// HFILE 0101 0101 0101 0101
	
	// 8RANK FF00 0000 0000 0000
	// 7RANK 00FF 0000 0000 0000
	// 6RANK 0000 FF00 0000 0000
	// 5RANK 0000 00FF 0000 0000
	// 4RANK 0000 0000 FF00 0000
	// 3RANK 0000 0000 00FF 0000
	// 2RANK 0000 0000 0000 FF00
	// 1RANK 0000 0000 0000 00FF
	
	int rank = 8; // start on the 8 rank
	int file = 0; // start at the A file
	int count = 0;
	char piece = '.'; // hold the piece
	bool isBlack = false; // indicate if the piece is black
	while((rank >= 1) && *FEN) // while on the board and there is FEN to be parsed
	{
		count = 1;
		
		switch(*FEN)
		{
			// black piece
			case 'p' : piece = 'p'; isBlack = true; break;
			case 'r' : piece = 'r'; isBlack = true; break;
			case 'n' : piece = 'n'; isBlack = true; break;
			case 'b' : piece = 'b'; isBlack = true; break;
			case 'k' : piece = 'k'; isBlack = true; break;
			case 'q' : piece = 'q'; isBlack = true; break;
			// white piece
			case 'P' : piece = 'P'; isBlack = false; break;
			case 'R' : piece = 'R'; isBlack = false; break;
			case 'N' : piece = 'N'; isBlack = false; break;
			case 'B' : piece = 'B'; isBlack = false; break;
			case 'K' : piece = 'K'; isBlack = false; break;
			case 'Q' : piece = 'Q'; isBlack = false; break;
			// number of empty
			case '1' : 
			case '2' : 
			case '3' : 
			case '4' : 
			case '5' : 
			case '6' : 
			case '7' : 
			case '8' : piece = nopiece; count = *FEN - '0'; break;
			// formatting
			case '/' :
			case ' ' : rank--; file = 0; FEN++; continue;
			// error
			default  : printf("\nSomething Went Wrong! %s", FEN);
		}
		for(int i = 0; i < count; i++)
		{
			//int square = rank * 8 + file;
			if(piece != nopiece)
			{
				
				char letter = 'A'+file;
				char number = '0'+rank;
				
				U64 FILE_ID = 0;
				U64 RANK_ID = 0;
				
				int type = 0;
				
				switch(letter)
				{
					case 'A' : FILE_ID = AFILE; break;
					case 'B' : FILE_ID = BFILE; break;
					case 'C' : FILE_ID = CFILE; break;
					case 'D' : FILE_ID = DFILE; break;
					case 'E' : FILE_ID = EFILE; break;
					case 'F' : FILE_ID = FFILE; break;
					case 'G' : FILE_ID = GFILE; break;
					case 'H' : FILE_ID = HFILE; break;
					default  : printf("\nSomething Went Wrong");
				}
				switch(number)
				{
					case '1' : RANK_ID = RANK1; break;
					case '2' : RANK_ID = RANK2; break;
					case '3' : RANK_ID = RANK3; break;
					case '4' : RANK_ID = RANK4; break;
					case '5' : RANK_ID = RANK5; break;
					case '6' : RANK_ID = RANK6; break;
					case '7' : RANK_ID = RANK7; break;
					case '8' : RANK_ID = RANK8; break;
					default  : printf("\nSomething Went Wrong");
				}
				
				switch(piece)
				{
					case 'p' : 
					case 'P' : type = pawn; break;
					case 'r' :
					case 'R' : type = rook; break;
					case 'n' :
					case 'N' : type = knight; break;
					case 'b' :
					case 'B' : type = bishop; break;
					case 'q' :
					case 'Q' : type = queen; break;
					case 'k' :
					case 'K' : type = king; break;
					default  : printf("\nSomething Went Wrong");
				}
				U64 square = (FILE_ID&RANK_ID);
				bs.bitboard[type] += square;
				if(isBlack) bs.bitboard[black] += square;
			}
			file++;
		}
		FEN++;
	}
	
	// combine all of the values
	
	bs.bitboard[total] = 	bs.bitboard[pawn]|
							bs.bitboard[rook]|
							bs.bitboard[knight]|
							bs.bitboard[bishop]|
							bs.bitboard[queen]|
							bs.bitboard[king];
	//printBoard(bs);
	
	// assign side to move
	*FEN == 'b' ? (bs.blackplaying = true) : (bs.blackplaying = false); 
	FEN+=2;

	// assign castling rights
	for(int i = 0; i < 4; i++)
	{
		if(*FEN == ' ') break;
		switch(*FEN)
		{
			case 'K' : bs.movementflags |= 0x80; break;
			case 'Q' : bs.movementflags |= 0x40; break;
			case 'k' : bs.movementflags |= 0x20; break;
			case 'q' : bs.movementflags |= 0x10; break;
			default  : break;
		}
		FEN++;
	}
	
	// TODO assign enpassant to movement flags
	
	return bs;
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

U8 // return number of chars read in
readInput(char *s, U8 strsize) 
{
	char c;
	fgets(s, strsize, stdin);
	if (s[strlen(s)-1]=='\n') { // if the string is shorter than max size
		s[strlen(s)-1]=0; // fix string length
	} else while((c=fgetc(stdin)) != EOF && c != '\n'); // cleaning stdin
	return strlen(s);
}

// parse from algebraic format
// e4/e5/Ne4/Be4 etc.

#define ATOC(A, B) ('a'-A+7+((B)-'0'-1)*8) // array to coordinate
#define INBOUNDS(A, B) ( ( (A>='a' && A<='z')|(A>='A' && A<='Z') ) && (B>='1' && B<='9') )
bool
parseInput(char *s, Coord *from, Coord *to) // used with stdin input
{
	U8 len = strlen(s);
	U8 file;
	U64 p = 1ULL;
	Coord temp;
	Board b, t;

	strrm(s, 'x'); // remove extra symbols
	strrm(s, '+');
	strrm(s, '#');

	if (strcmp(s, "O-O")==0) {
		// check which player is trying to castle
		// fauxmove
		if (currBoard.blackplaying) {
			*from = 59;
			*to = 57;
		} else {
			*from = 3;
			*to = 1;
		}
		return fauxMove(*from, *to, currBoard, NULL);
	} else if (strcmp(s, "O-O-O")==0) {
		// check which player is trying to castle
		// fauxmove
		if (currBoard.blackplaying) {
			*from = 59;
			*to = 61;
		} else {
			*from = 3;
			*to = 5;
		}
		return fauxMove(*from, *to, currBoard, NULL);
	}

	if (!INBOUNDS(s[len-2], s[len-1]) & (len>1)) return 0;
	switch(len) {
		case(0): // bad input, get input again
			return 0;
		case(1): // quit
			if ((*s=='q')|(*s=='Q')) return 2;
			return 0;
		case(2):// pawn movement
			// get playerboard and isolate pawns
			// highlighting
			// fauxmove from to
			if (s[0]<'a') { // highlighting
				temp = ATOC(s[0]-32, s[1]);
				if (!(currBoard.bitboard[total]&1ULL<<temp)) break; // make sure there's a piece there to print
				printHighlightBoard(currBoard, calculateMovementVector(currBoard, temp));
				break;
			}

			b = getPlayerBoard(currBoard.bitboard, currBoard.blackplaying)&currBoard.bitboard[pawn]; // only get the pawns
			*to = ATOC(s[0], s[1]); // convert format to coordinate
			// find from
			return (iterateVector(currBoard, b, p<<*to, from, 2)==1); // if only one piece can move its valid
		case(3): // move piece/pawn Ne4/de4
			 // figure out if the first char is lower or capital
			 // attempt pawn movement/piece movement
			
			*to = ATOC(s[1], s[2]);	
		       	if (*s<'a') { // if its giving a letter e.g. Nf3
				b = getPlayerBoard(currBoard.bitboard, currBoard.blackplaying)&currBoard.bitboard[ltoe[(U8) *s]];
				return (iterateVector(currBoard, b, p<<*to, from, 2)==1); // only want this to work if one piece can move there
			}
			file = 'a' - s[0] + 7; // find file 
			t = p<<file;
			for (int i = 0; i < 8; i++) {
				t<<=8;
				t |= p<<file;
			} // make a board of just that file
			b = getPlayerBoard(currBoard.bitboard, currBoard.blackplaying)&currBoard.bitboard[pawn]&t;
			return (iterateVector(currBoard, b, p<<*to, from, 2)==1); // only should work if one piece can move there 
		case(4): // move piece Nce4 / standard positional notation
			*to = ATOC(s[2], s[3]);
			if (INBOUNDS(s[0], s[1]) && (s[0]<'a')) { // make sure notation is correct
				// standard positional notation	
				*from = ATOC(s[0]-32, s[1]);
				return true;
			}
			if (!(INBOUNDS(s[0], '1')&&(s[0]<'a')&&INBOUNDS(s[1], '1')&&(s[1]>='a'))) break; // check syntax
			file = 'a' - s[1] + 7; // find file 
			t = p<<file;
			for (int i = 0; i < 8; i++) {
				t<<=8;
				t |= p<<file;
			} // make a board of just that file
			b = getPlayerBoard(currBoard.bitboard, currBoard.blackplaying)&currBoard.bitboard[ltoe[(U8) *s]]&t; // get a board of just those pieces on just that file
			return (iterateVector(currBoard, b, p<<*to, from, 2)==1); // only should work if one piece can move there 
		case(5): // move piece Nc3e4
			 // make sure that the piece at the from pos matches the char
			 // standard positional notation move
			*to = ATOC(s[3], s[4]);
			if (!(INBOUNDS(s[1], s[2])&&(s[1]>='a'))) break;
			*from = ATOC(s[1], s[2]);
			findPiece(*from, &temp, NULL, currBoard.bitboard);
			if (temp==ltoe[(U8) s[0]]) return fauxMove(*from, *to, currBoard, NULL);
			break;
	}
	return false;
}
#undef ATOC
#undef INBOUNDS

bool 
movePiece(Coord from, Coord to) // works exclusively with the current board
{
	Boardstate newbs = makeBoardstate(NULL, 0, 0); // new boardstate
	if (!fauxMove(from, to, currBoard, &newbs)) {
		free(newbs.bitboard);
		return false;
	}

	cpyBoardstate(&prevBoard, currBoard);

	cpyBoardstate(&currBoard, newbs);

	free(newbs.bitboard);

	appendMovePGN(prevBoard, currBoard, &po, from, to);
	flushPGN(currBoard, po);

	return true;
}

#define FAUXMOVERET(A, B) {free(A.bitboard); return B;}
bool
fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs) // write a companion that's more efficient as this is used EVERYWHERE and has a little extra overhead than needed in some cases
{
	Boardstate extra = makeBoardstate(NULL, 0, 0); // new boardstate
	if (nbs==NULL) nbs = &extra;
	nbs = cpyBoardstate(nbs, bs);
	Coord frompiece, topiece, passantpiece;
	bool fromcolourblack, tocolourblack, passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, nbs->bitboard);
	findPiece(to, &topiece, &tocolourblack, nbs->bitboard);

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
				test = blackPawnMovement(from, to, *nbs, NULL);
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
				test = whitePawnMovement(from, to, *nbs, NULL);
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
				puts("Enter which piece you want (R=1, N=2, B=3, Q=4): ");
				char pieceno[2];
PROMOTION:
				readInput(pieceno, 2);
				switch(*pieceno-'0') {
					case(rook):
					case(knight):
					case(bishop):
					case(queen):
						frompiece=*pieceno;
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
#undef FAUXMOVERET

inline Coord btoc(Board b) // board to coordinate
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

	Coord kingc = btoc(bs.bitboard[king]&pieces); // position of the king
	
	//check if any other piece can move to break mate
	
	Board vectors = queenAttackVectors(kingc, bs.bitboard) | knightAttackVectors(kingc); // every position that can be attacking the king & the king can move to

	return iterateVector(bs, pieces, vectors, NULL, 1) == 0; // try to move all of your pieces to ANY square
}

// 1) Not in check
// 2) Can't move any piece
//  -> Go through every piece, try and generate its movement vector then try and make that move with fauxmove?

bool
inStaleMate(Boardstate bs)
{
	if (inCheck(bs)) return false; // can't be in check and in stalemate
	Board pieces = getPlayerBoard(bs.bitboard, bs.blackplaying); //returns a board of all the players pieces
	return iterateVector(bs, pieces, -1, NULL, 1) == 0; // try to move all of your pieces to ANY square
}