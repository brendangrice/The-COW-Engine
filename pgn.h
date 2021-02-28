#include <stdio.h>
#include <time.h>

#ifndef PGN_H
#define PGN_H

#include "types.h"
#include "main.h"

typedef struct {
	char *event;
	char *site;
	char *date;
	char *round;
	char *white;
	char *black;
	char *result;
	char *time;
} pgnheader;

#define PGNSTRINGSIZE (4+(18*100)) // about 16 chars per turn at 100 turns with the result being printed at the end

typedef struct {
	pgnheader header;
	char pgn[PGNSTRINGSIZE];
	FILE *fp;
} pgnoutput;


// [Event "Casual Game"]
// [Site "The COW Engine"]
// [Date ""]
// [Round ""]
// [White ""]
// [Black ""]
// [Result ""]
// [Time ""]


char *strrep(char *s, char pre, char post);
pgnoutput makePGN(char *round, char *white, char *black);
bool appendMovePGN(Boardstate pre, Boardstate post, pgnoutput *po, Coord from, Coord to);
bool dumpPGN(Boardstate bs, pgnoutput po);

#endif //PGN_H
