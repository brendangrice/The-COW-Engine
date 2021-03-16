#include "pgn.h"
#include "main.h"
#include "types.h"

// PGN Format
// [Event "Casual Game"]
// [Site "The COW Engine"]
// [Date ""]
// [Round ""]
// [White ""]
// [Black ""]
// [Result ""]

PGNoutput
makePGN(char *round, char *white, char *black, char *fp)
{

	if (round == NULL) round = "1";
	if (white == NULL) white = "White";
	if (black == NULL) black = "Black";

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	PGNoutput po;

	strcpy(po.header.event, "Casual Game");

	char site[PGNHEADERSIZE];
	char timezone[10];
	memset(site, 0, PGNHEADERSIZE);
	strncat(site, "https://github.com/brendangrice/The-COW-Engine", PGNHEADERSIZE-10);
	strncat(site, ": ", 3);
	strftime(timezone, 10, "%Z", tm);
	strncat(site, timezone, 10);
	strcpy(po.header.site, site);

	char date[11];
	strftime(date, 11, "%F", tm);
	strrep(date, '-', '.');
	strcpy(po.header.date, date);
	strcpy(po.header.round, round);
	strcpy(po.header.white, white);
	strcpy(po.header.black, black);
	strcpy(po.header.result, "*");

	char starttime[9];
	strftime(starttime, 9, "%T", tm);
	strcpy(po.header.time, starttime);
	memset(po.pgn, 0, PGNSTRINGSIZE);
	po.fp = malloc(30);
	if (fp==NULL) {
		char f[30];
		memset(f, 0, 30);
		strncat(f, "pgn/", 5);
		strncat(f, date, strlen(date)+1);
		strncat(f, "_", 2);
		strncat(f, starttime, strlen(starttime)+1);
		strncat(f, ".pgn", 5);
		strrep(f, ':', '.');
		strcpy(po.fp, f);
	} else 	strcpy(po.fp, fp);
	return po;
}

// 
// read in input until it hits '1', '[', or EOF
// 1 -> no header
// '[' -> start reading header
// EOF -> no pgn found
//

bool
readPGN(FILE *in, PGNoutput *po) 
{
	char c, d;
	char inp[PGNHEADERSIZE]; //should only ever be as large as the largest header
	while (1) {
		c = fgetc(in);
		if (c=='1') goto READPGNBODY; // no header 
		if (c=='[') break; // header
		if (c==EOF) return false;
	}

	ungetc(c, in);
	// Fill up the headers
	/*
	"[Event \"%s\"] \n", po.header.event);
	"[Site \"%s\"] \n", po.header.site);
	"[Date \"%s\"]\n", po.header.date);
	"[Round \"%s\"]\n", po.header.round);
	"[White \"%s\"]\n", po.header.white);
	"[Black \"%s\"]\n", po.header.black);
	"[Result \"%s\"]\n", po.header.result);
	*/
	// TODO error handling
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.event, strchr(inp, '"')+1);
	strrep(po->header.event, '"', 0); // terminate the string at the next "
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.site, strchr(inp, '"')+1);
	strrep(po->header.site, '"', 0); // terminate the string at the next "
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.date, strchr(inp, '"')+1);
	strrep(po->header.date, '"', 0); // terminate the string at the next "
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.round, strchr(inp, '"')+1);
	strrep(po->header.round, '"', 0); // terminate the string at the next "
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.white, strchr(inp, '"')+1);
	strrep(po->header.white, '"', 0); // terminate the string at the next "
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.black, strchr(inp, '"')+1);
	strrep(po->header.black, '"', 0); // terminate the string at the next "
	fgets(inp, PGNHEADERSIZE, in);
	strcpy(po->header.result, strchr(inp, '"')+1);
	strrep(po->header.result, '"', 0); // terminate the string at the next "
	// discard everything else
	while (1) {
		c = fgetc(in);
		if (c=='\n') continue; // skip this line
		if (c=='1') break; // found the body
		while ((d=fgetc(in))!='\n' && d!=EOF); // go to next line	
	}

READPGNBODY:
	ungetc(c, in);
	memset(po->pgn, 0, PGNSTRINGSIZE);
	// fill in pgn
	for(int i=0; i<PGNSTRINGSIZE; i++) {
		c = fgetc(in);
		if (c==EOF) return false; // body ended prematurely
		if (c=='\n') c=' ';
		po->pgn[i] = c;
		if (c=='*') break; // all exit cases, *, 1-0, 0-1, 1/2
		if (c=='-'&&po->pgn[i-1]!='O'){
			while((c=fgetc(in))!=EOF && c!='\n') po->pgn[++i] = c; // append the rest
			break;
		}
	}
	return true;
}

U8 promotionpiece = nopiece;

U8
promotionPGN()
{
	return promotionpiece;
}

bool
parsePGN(PGNoutput po, Boardstate *bs, U8 flags)
{
	//parse pgn as input
	char white[9], black[9];
	Coord from, to;
	char *diff, *pos = po.pgn;
	char c;

	#define ERR(A,B) {if (A==0) {fprintf(stderr, "Bad input on move %d: %s", move, B); exit(1);}}
	for (int move = 1;;move++) {
		// separate inputs
		pos = strchr(pos, '.');
		if (pos==NULL) break;
		pos+=2;
		white[0] = 0;
		black[0] = 0;
		diff = strchr(pos, ' ');
		if (diff==NULL) break; // end of input
		strncat(white, pos, diff-pos); // copy one input across
		pos=diff+1;
		if (*white=='1' || *white=='0' || *white=='*') return true; // end of input

		// promotion
		diff = strchr(white, '=');
		if (diff!=NULL) {
			diff[0] = 0; // cut the string
			promotionpiece = ltoe[(int) diff[1]]; // set the promotion piece
		}

		// white input
		ERR(parseInput(white, &from, &to), white);
		ERR(movePiece(from, to, false, promotionPGN), white);
		if (flags&(ARGP_PGN_ALL|ARGP_PGN_STEP)) {
			if (flags&ARGP_PGN_HEADER) printHeader(po, stdout);
			if (flags&ARGP_PGN_PRINT) prettyPrintBoard(*bs);
			if (flags&ARGP_FEN_PRINT) printFEN(*bs, 0, 0); // TODO FIX THIS UP


			if (flags&ARGP_PGN_STEP) if ((c=getchar())=='q' || c==EOF) return true;
		}
		bs->blackplaying=!bs->blackplaying; // switch players
		diff = strchr(pos, ' ');
		if (diff==NULL) break; // end of input
		strncat(black, pos, diff-pos); // copy one input across
		pos=diff+1;
		if (*black=='1' || *black=='0' || *black=='*') return true; // end of input

		// promotion
		diff = strchr(black, '=');
		if (diff!=NULL) {
			diff[0] = 0; // cut the string
			promotionpiece = ltoe[(int) diff[1]]; // set the promotion piece
		}

		// black input
		ERR(parseInput(black, &from, &to), black);
		ERR(movePiece(from, to, false, promotionPGN), black);
		if (flags&(ARGP_PGN_ALL|ARGP_PGN_STEP)) {
			if (flags&ARGP_PGN_HEADER) printHeader(po, stdout);
			if (flags&ARGP_PGN_PRINT) prettyPrintBoard(*bs);
			if (flags&ARGP_FEN_PRINT) printFEN(*bs, 0, 0); // TODO FIX THIS UP


			if (flags&ARGP_PGN_STEP) if ((c=getchar())=='q' || c==EOF) return true;
		}
		bs->blackplaying=!bs->blackplaying; // switch players
	}
	#undef ERR
	return false;
}

bool
appendMovePGN(Boardstate pre, Boardstate post, PGNoutput *po, Coord from, Coord to)
{
	U64 p = 1ULL;
	const int stringsize = 18; // max size a string can be
	char str[stringsize]; // most characters each move can be e.g. Nf6xe4+
	char c, *s = str;
	memset(str, 0, stringsize);
	// create algebraic output with from and to
	// figure out what piece it is
	char piece;
	U8 pieceno, move=1;
	piece = findPiece(from, &pieceno, NULL, pre.bitboard);
	if (piece >= 'a' && piece <= 'z') piece-=32; //lowercase to upper
	if (piece < 'A' || piece > 'Z') return false; // out of bounds
	if (!pre.blackplaying) { // if its white we need to update the move count
		//find the number of moves already present
		for (int i = 0; po->pgn[i]; i++) if (po->pgn[i]=='.') move++;
		sprintf(str, "%d. ", move);
		s+=strlen(str);
	}
	
	// castling
	
	if ((pre.movementflags&0xF0) != (post.movementflags&0xF0)) { // potential castle
		if (pieceno == king) { // if the king moved
			switch(to) {
				// O-O-O
				case 5: // queen side white
				case 61: // queen side black
					*s++='O';
					*s++='-';
				// O-O
				case 1: // king side white
				case 57: // king side black
					*s++='O';
					*s++='-';
					*s++='O';
					goto APPENDEND;
			}
		}
	}
	
	// if its not a pawn (pawn doesnt give P)
	if (piece != 'P') *s++ = piece; // first character is the piece	

	// figure out if two pieces could be moving there, if so we need to specify file, if two are pointing there from the same file we need to specify row
	
	Board possiblepieces = (getPlayerBoard(pre.bitboard, pre.blackplaying)&pre.bitboard[pieceno])^p<<from; // every piece except the from piece
	Coord pos;
	bool file = false, rank = false;

	for (;possiblepieces; possiblepieces&=possiblepieces-1) {
		pos = btoc(possiblepieces);
		if (!!(calculateMovementVector(pre, pos)&p<<to)) {// if a piece can go to that square we need to specify
			file = true;
			if (pos%8 == from%8) rank = true;
		}
	}

	if (file) *s++ = 'h' - from%8; // if file needs to be specified
	if (rank) *s++ = '1' + from%8; // if rank needs to be specified

	if (pre.bitboard[total]&p<<to) {
		if (piece == 'P') *s++ = 'h' - from%8; // pawn needs to specify file when taking
		*s++ = 'x'; // taking
	}

	*s = 'h' - to%8; // file
	s[1] = '1' + to/8; // rank
	s+=2;

	if (piece == 'P' && (to/8==7 || to/8==0)) { // if it's a promotion we need to append =[PIECE]
		*s = '=';
		c = findPiece(to, NULL, NULL, post.bitboard); 
		if (c >= 'a' && c <= 'z') c-=32; //lowercase to upper
		if (c < 'A' || c > 'Z') return false; // out of bounds
		s[1] = c; // find promotion piece
		s+=2;
	}

APPENDEND: // TODO change more stuff to goto here to skip over more unneccessary if statements

	// if it's check/checkmate we need to append +/#
	// need to switch which player is being checked for mate
	post.blackplaying = !post.blackplaying;
	if (inCheck(post)) {
		if (inCheckMate(post)) {
			*s++='#';
		} else {
			*s++='+';
		}
	}

	*s = ' ';

	strncat(po->pgn, str, stringsize); // copy the new movement to the pgn output
	return true;
}
void
printHeader(PGNoutput po, FILE *s)
{
	fprintf(s, "[Event \"%s\"]\n", po.header.event);
	fprintf(s, "[Site \"%s\"]\n", po.header.site);
	fprintf(s, "[Date \"%s\"]\n", po.header.date);
	fprintf(s, "[Round \"%s\"]\n", po.header.round);
	fprintf(s, "[White \"%s\"]\n", po.header.white);
	fprintf(s, "[Black \"%s\"]\n", po.header.black);
	fprintf(s, "[Result \"%s\"]\n", po.header.result);
}

bool
flushPGN(Boardstate bs, PGNoutput po)
{
	FILE *fp = fopen(po.fp, "w"); // needs to be rewritten each turn
	printHeader(po, fp);

	fprintf(fp, "[Time \"%s\"]\n\n", po.header.time);

	if (inCheck(bs)) {
		if (inCheckMate(bs)) strcpy(po.header.result, bs.blackplaying?"0-1":"1-0");
	} else 	if (inStaleMate(bs)) strcpy(po.header.result, "1/2-1/2");

	strncat(po.pgn, po.header.result, 8); // append the result

	fputs(po.pgn, fp);
	fclose(fp);
	return true;
}

bool
dumpPGN(Boardstate bs, PGNoutput po) // write with flushPGN and then free memory
{
	flushPGN(bs, po);
	free(po.fp);
	return true;
}

// format
// [Event "Casual Game"]
// [Site "The COW Engine"]
// [Date ""]
// [Round ""]
// [White ""]
// [Black ""]
// [Result ""]
// [Time ""]
//
// 1. e4 e5 2. ... %RESULT%
