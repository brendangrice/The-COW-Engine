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
makePGN(char *round, char *white, char *black)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	PGNoutput po;

	po.header.event = "Casual Game";

	char *site = malloc(60);
	char timezone[10];
	memset(site, 0, 60);
	strncat(site, "https://github.com/brendangrice/The-COW-Engine", 50);
	strncat(site, ": ", 3);
	strftime(timezone, 10, "%Z", tm);
	strncat(site, timezone, 10);
	po.header.site = site;

	char *date = malloc(11);
	strftime(date, 11, "%F", tm);
	strrep(date, '-', '.');
	po.header.date = date;
	po.header.round = round;
	po.header.white = white;
	po.header.black = black;
	po.header.result = "*";

	char *starttime = malloc(9);
	strftime(starttime, 9, "%T", tm);
	po.header.time = starttime;
	memset(po.pgn, 0, PGNSTRINGSIZE);
	char *f = malloc(30);
	memset(f, 0, 30);
	strncat(f, "pgn/", 5);
	strncat(f, date, strlen(date)+1);
	strncat(f, "_", 2);
	strncat(f, starttime, strlen(starttime)+1);
	strncat(f, ".pgn", 5);
	strrep(f, ':', '.');
	po.fp = f;
	return po;
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
				case(5): // queen side white
				case(61): // queen side black
					*s++='O';
					*s++='-';
				// O-O
				case(1): // king side white
				case(57): // king side black
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

bool
flushPGN(Boardstate bs, PGNoutput po)
{

	FILE *fp = fopen(po.fp, "w"); // needs to be rewritten each turn
	fprintf(fp, "[Event \"%s\"]\n", po.header.event);
	fprintf(fp, "[Site \"%s\"]\n", po.header.site);
	fprintf(fp, "[Date \"%s\"]\n", po.header.date);
	fprintf(fp, "[Round \"%s\"]\n", po.header.round);
	fprintf(fp, "[White \"%s\"]\n", po.header.white);
	fprintf(fp, "[Black \"%s\"]\n", po.header.black);
	fprintf(fp, "[Result \"%s\"]\n", po.header.result);

	fprintf(fp, "[Time \"%s\"]\n\n", po.header.time);

	if (inCheck(bs)) {
		if (inCheckMate(bs)) po.header.result = bs.blackplaying?"0-1":"1-0";
	} else 	if (inStaleMate(bs)) po.header.result = "1/2-1/2";

	strncat(po.pgn, po.header.result, 8); // append the result

	fputs(po.pgn, fp);
	fclose(fp);
	return true;
}

bool
dumpPGN(Boardstate bs, PGNoutput po) // write with flushPGN and then free memory
{
	flushPGN(bs, po);
	free(po.header.date);
	free(po.header.time);
	free(po.header.site);
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
