#include "main.h"

#define HASH_SIZE 0x400000

// no hash entry found constant
#define NO_HASH_ENTRY 100000 // outside of alpha beta, maybe replace this with NaN?

// transposition table hash flags
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

#define DEFAULTBOARDKEY 1574655228986904411 // this needs to update if hashing algo changes
#define BOARD_KEY_NOT_SET 0

U32 randU32();
U64 randU64();
void initHash();
U64 generateHash(Boardstate bs);

void TTclear();
// fix these warnings
float TTread(int alpha, int beta, int depth); // read Transition Table for an entry matching the parameters.  
void TTwrite(int score, int depth, int flag); // writes to Transition Table with score depth and flag EXACT|ALPHA|BETA
