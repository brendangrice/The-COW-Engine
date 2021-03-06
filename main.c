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

static char args_doc[] = "[FILE]\n";

static char doc[] = "Plays Chess.\nWith FILE try to interpret FILE as PGN or FEN. When FILE is - read standard input as PGN or FEN\n\vIf -P or -F aren't given but - or a path is given COW tries to guess if it's PGN or FEN\n";

static struct argp_option options[] = {
	{"play", 'l', "GAMEMODE", 0, "Pass a gamemode to skip right into playing that"},
	{0, 1, 0, 0, ""}, // spacing
	{"fen", 'F', 0, 0, "Take in FEN input"},
	{"pgn", 'P', 0, 0, "Take in PGN input"},
	{"all", 'a', 0, 0, "Go through every move when viewing a PGN file"},
	{"step", 's', 0, 0, "Steps through the moves when viewing a PGN file"},
	{"print", 'p', 0, 0, "Don't print boards when viewing a PGN file"},
	{"header", 'h', 0, 0, "Toggle printing pgn header when viewing a PGN file"},
	{"fen-print", 'f', 0, 0, "Toggles printing FEN string for the current Board when viewing a PGN file"},
	{0, 1, 0, 0, ""}, // spacing
	{"output", 'o', "FILE", 0, "Output PGN notation to specified FILE instead of standard date notation"},
	{0, 1, 0, 0, ""}, // spacing
	{"help", '?', 0, 0, "Give this help list"},
	{"usage", ARGP_HELP_USAGE, 0, 0, "Give a short usage message"},
	{"version", 'V', 0, 0, "Print program version"},
	{ 0 }
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
		case 'l':
			arguments->gamemode = arg;
			state->next++; // skip the next argument
			break;
		case 'P':
			arguments->pgn = true;
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
		      	arguments->fenprint = true;
		      	break;
		case 'F':
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
			argp_usage(state, stderr);
			break;
		}
	return 0;
}


static struct argp argp = {options, parse_opt, args_doc, doc};

/*
---------- END OF ARGP ----------
*/

int 
main(int argc, char **argv) 
{
	
	initHash(); // sets up Zobrist hashing
	// if one argument is passed try and take it as input
	struct arguments arguments;
	arguments.arg = NULL;
	arguments.all = false;
	arguments.pgn = false;
	arguments.step = false;
	arguments.print = true;
	arguments.header = false;
	arguments.fenprint = false;
	arguments.fen = false;
	arguments.output_file = NULL;
	arguments.gamemode = NULL;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	po = makePGN("1", "white", "black", arguments.output_file); // set up pgn

	if ( (arguments.arg!=NULL) | arguments.fen | arguments.pgn ) {
		if ( !( arguments.fen | arguments.pgn) ) { // neither is specified, tried to figure out which is given
			if (arguments.arg == NULL || strcmp(arguments.arg, "-")==0) { // read from stdin
				char c = getchar();
				ungetc(c, stdin);
				//probably pgn
				if ( (c == '\n') | (c == '[') ) arguments.pgn = true;
				//probably fen
				if (c <= 'z' && c >= 'a') arguments.fen = true;
			} else 	arguments.pgn = true; // probably a path for a pgn file
		}

		if (arguments.pgn) {
			setBoardstate();
			FILE *in;
			if (arguments.arg != NULL && strcmp(arguments.arg, "-")!=0) {
				in = fopen(arguments.arg, "r");
				if (in==NULL) { // bad file
					fputs("Bad file read in\n", stderr);
					exit(1);
				}
			} else	in = stdin;
			
			po = makePGN(NULL, NULL, NULL, NULL);
			#define ERR() {fputs("Bad PGN Input\n", stderr); exit(1);}
			if (!readPGN(in, &po)) ERR();
			if (!parsePGN(po, &currBoard, &arguments)) ERR();
			#undef ERR
			if(!isatty(STDIN_FILENO) || strcmp(po.header.result,"*")!=0) { // can't continue with this board if not using player input or the game is finished
				if (!(strcmp(po.header.result,"*")==0 || strcmp(po.header.result,"1/2-1/2")==0)) currBoard.blackplaying=!currBoard.blackplaying; // flip board unless its unfinished or a draw
				if (!(arguments.all|arguments.step)) // if neither of these a set just print
				{
					if (!arguments.header) printHeader(po, stdout);
					if (arguments.print) prettyPrintBoard(currBoard);
					if (!arguments.fenprint) printFEN(currBoard, 0, 0);
				}
				return 0;
			} else { // need to format the po.pgn string for use
				int len = strlen(po.pgn)-1;
				while (po.pgn[len]!=' ') len--;
				po.pgn[len+1] = 0; // remove the result at the end of the string
			}
		}

		if (arguments.fen) {

			char fenstring[60]; // about 60 chars long
			if (arguments.arg == NULL || strcmp(arguments.arg, "-")==0) { // read from stdin
				fgets(fenstring, 60, stdin); // read in the FEN string
				parseFEN(fenstring); // needs some error checking
				if (!isatty(STDIN_FILENO)) { // maybe reopen stdin to clear the piping??
					prettyPrintBoard(currBoard);
					return 0; // have to return here if we're piping in
				}
			} else 	setBoardstate(); // just in case
		}
	} else 	setBoardstate();


	// skip straight into a gamemode
	//
	if (arguments.gamemode!=NULL) {
		if (strcmp(arguments.gamemode, "1")==0 || strcmp(arguments.gamemode, "lmp")==0) {
				localMultiplayer(&currBoard);
		} else if (strcmp(arguments.gamemode, "2")==0 || strcmp(arguments.gamemode, "lai")==0) {
				localAI(&currBoard);
		} else {
			fputs("Bad argument: ", stdout);
			fputs(arguments.gamemode, stdout);
			putchar('\n');
		}
		setBoardstate(); // reset boardstate
		po = makePGN("1", "white", "black", arguments.output_file); // reset pgn
	}

	char inp[2]; // input
	while (1) {
	       	puts("Select your gamemode.");
		puts("Local multiplayer: 1");
		puts("Local AI: 2");
		puts("Quit: q");
		// maybe add a read pgn file option here
		
		readInput(inp, 2);
		if (*inp == 'Q' || *inp == 'q') return 0;
		switch(*inp) {
			case '1':
				localMultiplayer(&currBoard);
				break;
			case '2':
				localAI(&currBoard);
				break;
			default:
				puts("Bad Input");
				break;
		}
		setBoardstate(); // reset boardstate
		po = makePGN("1", "white", "black", arguments.output_file); // reset pgn
	}
	return 0;
}

inline void 
strrep(char *s, char pre, char post) // string replace
{
	for (int i=0; i<strlen(s); i++) if (s[i]==pre) s[i]=post;
}

void 
strrm(char *s, char rm) // string remove
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
setBoardstate() // sets Boards to be equal to the default values for a chess board
{
	prevBoard = makeBoardstate(NULL, 0xF0, false, 1, 1);
	currBoard = makeBoardstate(NULL, 0xF0, false, 1, 1);
	currBoard.key = DEFAULTBOARDKEY;

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

Boardstate
makeBoardstate(Board *bitboard, U8 movementflags, bool blackplaying, U16 halfmove, U16 fiftymove)
{
	Boardstate newbs;
	memset(newbs.bitboard, 0, BITBOARDSIZE);
	if (bitboard!=NULL) memcpy(newbs.bitboard, bitboard, BITBOARDSIZE);
	newbs.movementflags = movementflags;
	newbs.blackplaying = blackplaying;
	newbs.halfmove = halfmove;
	newbs.fiftymove = fiftymove;
	newbs.key = BOARD_KEY_NOT_SET;
	
	return newbs;
}

Boardstate *
cpyBoardstate(Boardstate *to, Boardstate from)
{
	return memcpy(to, &from, BOARDSTATESIZE);
}

char 
findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard) //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece.
{
	U8 extra;
	if (piece == NULL) piece = &extra;
	if (colourblack == NULL) colourblack = (bool *) &extra;
	char out = 'A'; // temp for checking for errors
	*colourblack = false;
	U64 bit = bitboard[total]&(1ULL<<pos); // gets whatever piece is at the location
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
	U8 piecenum;
	bool piececolour;
	Board vector = 0;
	for (int i = 0; i < 64; i++) {
		if (( (piece=findPiece(i, &piecenum, &piececolour, bitboard))!='.' ) && (piececolour == blackplaying)) { // find every piece that isn't a blank piece and is matching the colour
			piecenum++; // offset by one to account for the two different pawn funcs
			
			if (piece == 'P') piecenum = 0; // white and black have different attacks
			vector |= attackVectors[piecenum](i, bitboard); // lookup table of functions for attacks
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
	piece++;
	if ((piece==pawn+1) && (!colour)) piece = 0; // white pawn
	movementVectors[piece](pos, 0, bs, &vector); // just want the vector
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
				temp = atoc(s[0]+32, s[1]);
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

bool 
movePiece(Coord from, Coord to, bool PGN, U8(*promote)()) // works exclusively with the current board
{
	Boardstate newbs;
	if (!fauxMove(from, to, currBoard, &newbs, promote)) return false;

	cpyBoardstate(&prevBoard, currBoard);

	cpyBoardstate(&currBoard, newbs);

	if (PGN) {
		appendMovePGN(prevBoard, currBoard, &po, from, to);
		flushPGN(currBoard, po);
	}

	currBoard.halfmove++; // inc

	if (currBoard.bitboard[pawn]!=prevBoard.bitboard[pawn] || (popcnt(currBoard.bitboard[total]) != popcnt(prevBoard.bitboard[total]))) currBoard.fiftymove = currBoard.halfmove; // if a move was made that fits the conditions of the 50 move rule update the value

	return true;
}

U8
defaultPromotion() 
{
	return queen;
}

U8
getPromotion() 
{
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
fauxMove(Coord from, Coord to, Boardstate bs, Boardstate *nbs, U8(*promote)()) // write a companion that's more efficient as this is used EVERYWHERE and has a little extra overhead than needed in some cases
{
	Boardstate extra;
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

inline Coord 
btoc(Board b) // board to coordinate
{
	if (!b) return 0;
	Coord pos = 0;
	for (pos=0;!(b&1);b>>=1) pos++;
	return pos;
}

int 
popcnt(Board b)
{
	unsigned x = 0;
	for(x = 0; b; x++)
		b&=b-1;
	return x;
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
	
	Board pieces = getPlayerBoard(bs.bitboard, bs.blackplaying); // Board containing just the king in question

	Coord kingc = btoc(bs.bitboard[king]&pieces); // position of the king
	
	Board vectors = queenAttackVectors(kingc, bs.bitboard) | knightAttackVectors(kingc, NULL); // every position that can be attacking the king & the king can move to

	return iterateVector(bs, pieces, vectors, NULL, 1) == 0; // try to move ALL of your pieces to ANY square that could be threatening the king
}

// 1) Not in check
// 2) Can't move any piece
//  -> Go through every piece, try and generate its movement vector then try and make that move with fauxmove?

bool
inStaleMate(Boardstate bs)
{
	if (inCheck(bs)) return false; // can't be in check and in stalemate
	Board pieces = getPlayerBoard(bs.bitboard, bs.blackplaying); // returns a board of all the players pieces
	return iterateVector(bs, pieces, -1, NULL, 1) == 0; // try to move ALL of your pieces to ANY square
}
