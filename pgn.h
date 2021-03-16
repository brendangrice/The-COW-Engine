#include <stdio.h>
#include <time.h>

#ifndef PGN_H
#define PGN_H

#include "types.h"
#include "main.h"

#define PGNHEADERSIZE 80

typedef struct {
	char event[PGNHEADERSIZE];
	char site[PGNHEADERSIZE];
	char date[PGNHEADERSIZE];
	char round[PGNHEADERSIZE];
	char white[PGNHEADERSIZE];
	char black[PGNHEADERSIZE];
	char result[PGNHEADERSIZE];
	char time[PGNHEADERSIZE];
} PGNheader;

#define PGNSTRINGSIZE (4+(18*100)) // about 18 chars per turn at 100 turns with the result being printed at the end

typedef struct {
	PGNheader header;
	char pgn[PGNSTRINGSIZE]; // body of the moves being made
	char *fp; // string representation of the path being saved to
} PGNoutput;


// [Event "Casual Game"]
// [Site "The COW Engine"]
// [Date ""]
// [Round ""]
// [White ""]
// [Black ""]
// [Result ""]
// [Time ""]


PGNoutput makePGN(char *round, char *white, char *black, char *fp); // makes pgn object and sets up all the memory and default values associated. Returns the pgn
bool readPGN(FILE *in, PGNoutput *po);
bool parsePGN(PGNoutput po, Boardstate *bs, bool step);
bool appendMovePGN(Boardstate pre, Boardstate post, PGNoutput *po, Coord from, Coord to); // Takes the previous boardstate and the current boardstate to figure out what changes have been made to the board with the moves from and to and updates the pgn body appropriately. Bool returns whether or not it updates successfully
void printHeader(PGNoutput po, FILE *s); // prints the basic 7 header values of a PGN file
bool flushPGN(Boardstate bs, PGNoutput po); // writes to file described in po.fp
bool dumpPGN(Boardstate bs, PGNoutput po); // runs flushPGN and cleans up the memory

#endif //PGN_H
