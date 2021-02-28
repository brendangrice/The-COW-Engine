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
} PGNheader;

#define PGNSTRINGSIZE (4+(18*100)) // about 16 chars per turn at 100 turns with the result being printed at the end

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


char *strrep(char *s, char pre, char post); // replaces chars pre with post in the string s. Returns s
PGNoutput makePGN(char *round, char *white, char *black); // makes pgn object and sets up all the memory and default values associated. Returns the pgn
bool appendMovePGN(Boardstate pre, Boardstate post, PGNoutput *po, Coord from, Coord to); // Takes the previous boardstate and the current boardstate to figure out what changes have been made to the board with the moves from and to and updates the pgn body appropriately. Bool returns whether or not it updates successfully
bool flushPGN(Boardstate bs, PGNoutput po); // writes to file described in po.fp
bool dumpPGN(Boardstate bs, PGNoutput po); // runs flushPGN and cleans up the memory

#endif //PGN_H
