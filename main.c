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

// START OF ARGP

// read in PGN file and be able to play that out
//  -> have another optional arg for single stepping through pgn input

const char *argp_program_version = "<The COW Engine v0.91>";
const char *argp_prgram_bug_address = "<https://github.com/brendangrice/The-COW-Engine>";

static char args_doc[] = "[FILE]";

static char doc[] = "Plays Chess.\nWith FILE try to interpret FILE as PGN. When FILE is - read standard input as PGN\n\vmore docs\n";

static struct argp_option options[] = {
	{"all", 'a', 0, 0, "Go through every move when viewing a PGN file"},
	{"step", 's', 0, 0, "Steps through the moves when viewing a PGN file"},
	{"print", 'p', 0, 0, "Don't print boards when viewing a PGN file"},
	{"header", 'h', 0, 0, "Toggle printing pgn header when viewing a PGN file"},
	{"fen", 'f', 0, 0, "Toggles printing FEN string for the current Board when viewing a PGN file"},
	{"output", 'o', "FILE", 0, "Output PGN notation to specified FILE instead of standard date notation"},
	{"help", '?', 0, 0, "Give this help list"},
	{"usage", ARGP_HELP_USAGE, 0, 0, "Give a short usage message"},
	{"version", 'V', 0, 0, "Print program version"},
	{ 0 }
};

struct arguments 
{
	char *arg;
	bool all;
	bool step;
	bool print;
	bool header;
	bool fen;
	char *output_file;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch(key) {
		case ARGP_HELP_USAGE:
			argp_usage(state, stdout);
			break;
		case 'V':
			argp_version();
			break;
		case '?':
			argp_help(state, stdout);
			break;
		case 'a':
			arguments->all = true;	
			break;
		case 's':
			arguments->step = true;
			break;
		case 'p':
			arguments->print = false;
			break;
		case 'h':
			arguments->header = true;
			break;
		case 'f':
		      	arguments->fen = true;
		      	break;
		case 'o':
			arguments->output_file = arg;
			state->next++; // skip the next argument
			break;

		case ARGP_KEY_ARG:
			if (state->arg_num > 1) argp_usage(state, stderr); // exit here, only takes one input
			arguments->arg = arg;
			break;
		default:
			printf("%d %c\n", key, key);
			argp_usage(state, stderr);
			break;
		}
	return 0;
}


static struct argp argp = {options, parse_opt, args_doc, doc};

// END OF ARGP

int 
main(int argc, char **argv) 
{
	// if one argument is passed try and take it as input
	struct arguments arguments;
	arguments.arg = NULL;
	arguments.all = false;
	arguments.step = false;
	arguments.print = true;
	arguments.header = false;
	arguments.fen = false;
	arguments.output_file = NULL;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	// set up the flags
	U8 flags = arguments.all*ARGP_PGN_ALL | arguments.step*ARGP_PGN_STEP | arguments.print*ARGP_PGN_PRINT | arguments.header*ARGP_PGN_HEADER | arguments.fen*ARGP_FEN_PRINT;
	if (arguments.arg!=NULL) {
		setBitBoard();
		FILE *in;
	       	if (strcmp(arguments.arg, "-")!=0) {
			in = fopen(arguments.arg, "r");
			if (in==NULL) { // bad file
				fputs("Bad file read in\n", stderr);
				exit(1);
			}
		} else	in = stdin;
		
		PGNoutput pgn = makePGN(NULL, NULL, NULL, NULL);
		readPGN(in, &pgn);
		parsePGN(pgn, &currBoard, flags);
		if (!(flags&(ARGP_PGN_ALL|ARGP_PGN_STEP)))
		{
			if (~flags&ARGP_PGN_HEADER) printHeader(pgn, stdout);
			if (flags&ARGP_PGN_PRINT) prettyPrintBoard(currBoard);
			if (~flags&ARGP_FEN_PRINT) printFEN(currBoard, 0, 0); // TODO FIX THIS UP

		}
		return 0;
	}

	char inp[2]; // input
	while (1) {
		setBitBoardFromFEN();
	       	puts("Select your gamemode.");
		puts("Local multiplayer: 1");
		puts("Local AI: 2");
		puts("Quit: q");
		// maybe add a read pgn file option here
		
		readInput(inp, 2);
		if (*inp == 'Q' || *inp == 'q') return 0;
		po = makePGN("1", "white", "black", arguments.output_file);
		switch(*inp) {
			case '1':
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
	return 0;
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
	prevBoard = makeBoardstate(NULL, 0, 0);
	currBoard = makeBoardstate(NULL, 0, 0);
	
	char *FEN = FENBOARDDEFAULT;
	currBoard = parseFEN(FEN, currBoard);
}

Boardstate
makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying)
{
	Boardstate newbs;
	memset(newbs.bitboard, 0, BITBOARDSIZE);
	if (bitboard!=NULL) memcpy(newbs.bitboard, bitboard, BITBOARDSIZE);
	newbs.movementflags = movementflags;
	newbs.blackplaying = blackplaying;
	
	return newbs;
}

Boardstate *
cpyBoardstate(Boardstate *to, Boardstate from)
{
		memcpy(to->bitboard, from.bitboard, BITBOARDSIZE);
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
			if ((piece == 'P') & (!blackplaying)) vector |= whitePawnAttackVectors(i, NULL); // white and black have different attacks
			if ((piece == 'p') & blackplaying) vector |= blackPawnAttackVectors(i, NULL);
			piece-=32*blackplaying; //if the piece is black subtract so that the char goes into the switch cases nicely
			switch(piece) 
			{
				case 'R':
					vector |= rookAttackVectors(i, bitboard);
					break;
				case 'N':
					vector |= knightAttackVectors(i, NULL);
					break;
				case 'B':
					vector |= bishopAttackVectors(i, bitboard);
					break;
				case 'Q':
					vector |= queenAttackVectors(i, bitboard);
					break;
				case 'K':
					vector |= kingAttackVectors(i, NULL);
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
		case pawn:
			if (colour) {
				blackPawnMovement(pos, 0, bs, &vector);
			} else	whitePawnMovement(pos, 0, bs, &vector);
			break;
		case rook:
			rookMovement(pos, 0, bs, &vector);
			break;
		case knight:
			knightMovement(pos, 0, bs, &vector);
			break;
		case bishop:
			bishopMovement(pos, 0, bs, &vector);
			break;
		case queen:
			queenMovement(pos, 0, bs, &vector);
			break;
		case king:
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
			if(fauxMove(pos, pos2, bs, NULL, NULL)) {
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
	if(bs.movementflags&0x80) putchar('K');
	if(bs.movementflags&0x40) putchar('Q');
	if(bs.movementflags&0x20) putchar('k');
	if(bs.movementflags&0x10) putchar('q');
	
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
	const char *linebreak = "\n   +---+---+---+---+---+---+---+---+";

	char c;
	for(int i = 0 ; i < 64; i++)
	{
		if (!(i%8)) {
			puts(linebreak);
			putchar(' ');
			putchar(bs.blackplaying?'1'+(i/8):'8'-(i/8));
			putchar(' ');
			putchar('|');
			putchar(' ');
		}
		c = bs.blackplaying?findPiece(i, NULL, NULL, bs.bitboard):findPiece(63-i, NULL, NULL, bs.bitboard); // needs to read the chars in a different order depending how its printing
		if (c=='.' && (i+(i/8))&1) c = ',';
		putchar(c);
		putchar(' ');
		putchar('|');
		putchar(' ');

	}
	puts(linebreak);
	puts(bs.blackplaying ? "\n     H   G   F   E   D   C   B   A  \n" : "\n     A   B   C   D   E   F   G   H  \n");
}
void
printBoard(Boardstate bs)
{
	char c;
	for (int i = 0; i < 64; i++) {
		if (!(i%8)) { 
			putchar('\n');
			putchar(bs.blackplaying?'1'+(i/8):'8'-(i/8));
			putchar(' ');
			putchar(' ');
		}
		c = bs.blackplaying?findPiece(i, NULL, NULL, bs.bitboard):findPiece(63-i, NULL, NULL, bs.bitboard); // needs to read the chars in a different order depending how its printing
		if (c=='.' && (i+(i/8))&1) c = ',';
		putchar(c);
		putchar(' ');
	}
	puts(bs.blackplaying?"\n\n%  H G F E D C B A\n":"\n\n%  A B C D E F G H\n");
}

void
printHighlightBoard(Boardstate bs, Board highlights)
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

	const U8 boardsize = BOARDSIZE*BOARDSIZE; // 64 
	const char *linebreak = "\n   +---+---+---+---+---+---+---+---+";

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

	for (int i = 0; i < 64; i++) { // print out formatted
		if (!(i%8)) {
			puts(linebreak);
			putchar(' ');
			putchar(bs.blackplaying?'1'+(i/8):'8'-(i/8));
			putchar(' ');
			putchar('|');
			putchar(' ');
		}
		putchar(bs.blackplaying?out[i]:out[63-i]); // prints out forwards or backwards
		putchar(' ');
		putchar('|');
		putchar(' ');
	}
	puts(linebreak);
	puts(bs.blackplaying ? "\n     H   G   F   E   D   C   B   A  \n" : "\n     A   B   C   D   E   F   G   H  \n");
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
	
	int rank = 8; // start on the 8 rank
	int file = 0; // start at the A file
	int count = 0;
	char piece = '.'; // hold the piece
	bool isBlack = false; // indicate if the piece is black
	while((rank >= 1) && *FEN) // while on the board and there is FEN to be parsed
	{
		count = 1;
		
		piece = *FEN;
		isBlack = *FEN>'R';
		switch(*FEN)
		{
			// black piece
			case 'p' : 
			case 'r' : 
			case 'n' : 
			case 'b' : 
			case 'k' : 
			case 'q' :
			// white piece
			case 'P' : 
			case 'R' : 
			case 'N' : 
			case 'B' : 
			case 'K' : 
			case 'Q' : break;
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
			default  : puts("\nSomething Went Wrong!"); exit(1); // cheeky exit
		}
		for(int i = 0; i < count; i++)
		{
			//int square = rank * 8 + file;
			if(piece != nopiece)
			{
				
				char letter = 'A'+file;
				char number = '0'+rank;
				
				U64 file_ID = 0;
				U64 rank_ID = 0;
				
				U8 type = 0;
				
				if ((letter<'A') || (letter>'H')) {
					puts("\nSomething Went Wrong"); 
					exit(1); // cheeky exit
				}
				
				file_ID = 0x0101010101010101*(1<<('H'-letter));

				if ((number<'1') || (number>'8')) {
					puts("\nSomething Went Wrong"); 
					exit(1); // cheeky exit
				}
				
				rank_ID = (0x00000000000000FF*1ULL)<<(8*(number-'1'));

				type = ltoe[(U8) piece];
				if (type == nopiece) {
					puts("\nSomething Went Wrong"); 
					exit(1);
				}

				U64 square = (file_ID&rank_ID);
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
	// assign side to move
	bs.blackplaying = (*FEN == 'b');
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

inline Coord atoc(char l, char n) { // could use a better name
	if (!((l>='a' && l<='z')|(l>='A' && l<='Z') ) && (n>='1' && n<='9')) return -1; // not in bounds
	return 'a'-l+7+((n)-'0'-1)*8; // array to coordinate
}

inline char *ctoa(char *s, Coord c) {
	if (c>63) return NULL;
	s[0] = 'H'-c%8;
	s[1] = c/8+'1';
	return s;
}

#define INBOUNDS(A, B) ( ( (A>='a' && A<='z')|(A>='A' && A<='Z') ) && (B>='1' && B<='9') )
U8
parseInput(char *s, Coord *from, Coord *to) // used with stdin input
{
	strrm(s, 'x'); // remove extra symbols
	strrm(s, '+');
	strrm(s, '#');
	strrm(s, ' ');

	U8 len = strlen(s);
	U8 file;
	U64 p = 1ULL;
	Coord temp;
	Board b, t;

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
		return fauxMove(*from, *to, currBoard, NULL, NULL);
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
		return fauxMove(*from, *to, currBoard, NULL, NULL);
	}

	if (!INBOUNDS(s[len-2], s[len-1]) & (len>1)) return 0;
	switch(len) {
		case 0: // bad input, get input again
			return 0;
		case 1: // quit
			if ((*s=='q')|(*s=='Q')) return 2;
			return 0;
		case 2:// pawn movement
			// get playerboard and isolate pawns
			// highlighting
			// fauxmove from to
			if (s[0]<'a') { // highlighting
				temp = atoc(s[0]-32, s[1]);
				if (!(currBoard.bitboard[total]&1ULL<<temp)) break; // make sure there's a piece there to print
				printHighlightBoard(currBoard, calculateMovementVector(currBoard, temp));
				break;
			}

			b = getPlayerBoard(currBoard.bitboard, currBoard.blackplaying)&currBoard.bitboard[pawn]; // only get the pawns
			*to = atoc(s[0], s[1]); // convert format to coordinate
			// find from
			return (iterateVector(currBoard, b, p<<*to, from, 2)==1); // if only one piece can move its valid
		case 3: // move piece/pawn Ne4/de4
			 // figure out if the first char is lower or capital
			 // attempt pawn movement/piece movement
			
			*to = atoc(s[1], s[2]);	
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
		case 4: // move piece Nce4 / standard positional notation
			*to = atoc(s[2], s[3]);
			if (INBOUNDS(s[0], s[1]) && (s[0]<'a')) { // make sure notation is correct
				// standard positional notation	
				*from = atoc(s[0]-32, s[1]);
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
		case 5: // move piece Nc3e4
			 // make sure that the piece at the from pos matches the char
			 // standard positional notation move
			*to = atoc(s[3], s[4]);
			if (!(INBOUNDS(s[1], s[2])&&(s[1]>='a'))) break;
			*from = atoc(s[1], s[2]);
			findPiece(*from, &temp, NULL, currBoard.bitboard);
			if (temp==ltoe[(U8) s[0]]) return fauxMove(*from, *to, currBoard, NULL, NULL);
			break;
	}
	return false;
}
#undef INBOUNDS

U8
defaultPromotion() {
	return queen;
}

U8
getPromotion() {
	puts("Enter which piece you want (R=1, N=2, B=3, Q=4): ");
	char pieceno[2];
PROMOTION:
	readInput(pieceno, 2);
	switch(*pieceno-'0') {
		case(rook):
		case(knight):
		case(bishop):
		case(queen):
			return *pieceno-'0';
			break;
		default:
			goto PROMOTION;
			break;
	}
}

bool 
movePiece(Coord from, Coord to, bool PGN, U8(*promote)()) // works exclusively with the current board
{
	Boardstate newbs = makeBoardstate(NULL, 0, 0); // new boardstate
	if (!fauxMove(from, to, currBoard, &newbs, promote)) return false;

	cpyBoardstate(&prevBoard, currBoard);

	cpyBoardstate(&currBoard, newbs);

	if (PGN) {
		appendMovePGN(prevBoard, currBoard, &po, from, to);
		flushPGN(currBoard, po);
	}

	return true;
}

bool
fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs, U8(*promote)()) // write a companion that's more efficient as this is used EVERYWHERE and has a little extra overhead than needed in some cases
{
	Boardstate extra = makeBoardstate(NULL, 0, 0); // new boardstate
	if (nbs==NULL) nbs = &extra;
	nbs = cpyBoardstate(nbs, bs);
	if (promote == NULL) promote = defaultPromotion;

	Coord frompiece, topiece, passantpiece;
	bool fromcolourblack, tocolourblack, passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, nbs->bitboard);
	findPiece(to, &topiece, &tocolourblack, nbs->bitboard);
	
       	// Can't move a piece that doesn't exist or take your own piece
	if (bs.blackplaying-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&(topiece!=nopiece)) return false;

	U64 p = 1ULL;
	bool test = false;
	
	if (frompiece!=pawn) nbs->movementflags&=0xF0; //TODO find a nicer way overall to handle the movement flags for en passant

	//do testing based on piece
	switch(frompiece) {
		case pawn:
			if (fromcolourblack) {
				test = blackPawnMovement(from, to, *nbs, NULL);
				if (test == MOVES_PAWN_EN_PASSANT) { //en passant logic
					findPiece(to+8, &passantpiece, &passantcolourblack, nbs->bitboard);
					nbs->bitboard[total]^=p<<(to+8);
				} 
				if(test == MOVES_PAWN_DOUBLE) { // update flags for double move
					nbs->movementflags&=0xF0;
					nbs->movementflags|=from%8;
					nbs->movementflags|=0x8;
				} else {
					nbs->movementflags&=0xF0;
				}
			} else {
				test = whitePawnMovement(from, to, *nbs, NULL);
				if (test == MOVES_PAWN_EN_PASSANT) { // en passant
					findPiece(to-8, &passantpiece, &passantcolourblack, nbs->bitboard);	
					nbs->bitboard[black]^=p<<(to-8);
					nbs->bitboard[total]^=p<<(to-8);
				}
				if(test == MOVES_PAWN_DOUBLE) { // update flags for double move
					nbs->movementflags&=0xF0;
					nbs->movementflags|=from%8;
					nbs->movementflags|=0x8;
				} else {
					bs.movementflags&=0xF0;
				}
			}
			// pawn has reached the end of the board 
			if (test && (0==to>>3 || 7==to>>3)) {
				frompiece = promote();
				if (frompiece<rook || frompiece>queen) frompiece = queen; // just in case
			}

			break;
		case rook:
			test = rookMovement(from, to, *nbs, NULL);
			break;
		case knight:
			test = knightMovement(from, to, *nbs, NULL);
			break;
		case bishop:
			test = bishopMovement(from, to, *nbs, NULL); 
			break;
		case queen:
			test = queenMovement(from, to, *nbs, NULL);
			break;
		case king:
			// return codes:
			// 0 fail
			// 1 normal
			// 2 White queenside castle
			// 3 White kingside castle
			// 4 Black queenside castle
			// 5 Black kingside castle
			// relevent castle needs to be moved and total updated
			test = kingMovement(from, to, *nbs, NULL);
			switch (test) {
				case MOVES_KING_WHITE_QUEEN:
					nbs->bitboard[rook]^=p<<7;
					nbs->bitboard[rook]|=p<<4;
					nbs->bitboard[total]^=p<<7;
					nbs->bitboard[total]|=p<<4;
					break;
				case MOVES_KING_WHITE_KING:
					nbs->bitboard[rook]^=1;
					nbs->bitboard[rook]|=p<<2;
					nbs->bitboard[total]^=1;
					nbs->bitboard[total]|=p<<2;
					break;
				case MOVES_KING_BLACK_QUEEN: // need to update black nbs->bitboard too
					nbs->bitboard[black]^=p<<63;
					nbs->bitboard[black]|=p<<60;
					nbs->bitboard[rook]^=p<<63;
					nbs->bitboard[rook]|=p<<60;
					nbs->bitboard[total]^=p<<63;
					nbs->bitboard[total]|=p<<60;
					break;
				case MOVES_KING_BLACK_KING:
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

	if (!test) return false; // if the piece can't move there
	
	// set to piece with opposite colour. remove from piece
	// calc attack vectors
	// if the king is being attacked roll back (no move made)
	//
	// tries to update a piece's position
	
	if (fromcolourblack) {
		nbs->bitboard[black]^=p<<from;	// remove the old piece's position
		nbs->bitboard[black]|=p<<to;		// update it to the new spot
	} else 	nbs->bitboard[black]&=~(p<<to); // if the new piece is white it needs to be removed from the black board
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
		
		return false;
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

	return true;
}

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

inline Board getPlayerBoard(Board *bitboard, bool blackplaying) 
{
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
	
	Board vectors = queenAttackVectors(kingc, bs.bitboard) | knightAttackVectors(kingc, NULL); // every position that can be attacking the king & the king can move to

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
