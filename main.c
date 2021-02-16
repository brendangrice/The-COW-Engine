#include "main.h"

////
// Maybe just update total at the end of each move instead of updating it 
// alongside all the other pieces and movements with one or.
// REMEMBER TO CHECK ALL THE FREE'S
////

Boardstate currBoard;

void
setBitBoard()
{
	currBoard.movementflags=0xF0; // set castling to be available for both players
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

int 
main() 
{
	setBitBoard();

	while (1) {
		char *cht = malloc(5);
		fputs("\nLocal multiplayer: 1\n", stdout);
		fputs("Online multiplayer: 2\n", stdout);
		fputs("Host an online multiplayer game: 3\n", stdout);
		fputs("Quit: q\n", stdout);
REPEATGAMEMODEINPUT:
		fgets(cht, 4, stdin);
		fflush(stdout);

		switch(*cht) {
			case('1'):
				localMultiplayer();
				break;
			case('2'):
				onlineMultiplayer();
				break;
			case('3'):
				onlineMultiplayerHosting();
				break;
			case('q'):
				return 0;
				break;
			case('Q'):
				return 0;
				break;
			default:
				goto REPEATGAMEMODEINPUT;
				break;
		}
		free(cht);
	}
}

void
localMultiplayer()
{
	setBitBoard();
	puts("White to play");
	printBoard(currBoard.bitboard, false);
	// user input
	// every time the user inputs a new move the attack vectors need to be reevaluated.
	bool blackplaying = false;
	Coord from, to;
	for(;;) { // strange things are happening 
		//replace with do while
		while(!parseInput(&from, &to) || !movePiece(from, to, blackplaying)); // split for special interrupts on inputs/parseInput needs to be redone
		blackplaying=!blackplaying; // switch players
		blackplaying?puts("\nBlack to play"):puts("\nWhite to play");

		printBoard(currBoard.bitboard, blackplaying);
	}
	return;
}

void
onlineMultiplayer()
{
	setBitBoard();
	//add something to handle ip
	
	char domain[20];
	char port[10];
	getDomainAndPort(domain, port);
	sas *servcon = malloc(sizeof(sas));
	fputs("Attempting to connect...\n", stdout);
	if(!serverConnect(domain, port, servcon)) {
		free(servcon);
		return;	
	};
	char serverreplyplayer;
	Coord serverIO[3];
	bool player; //  white is false, black is true
	fputs("Querying...\n", stdout);
	receiveInput(servcon, &serverreplyplayer, 1);
	if (serverreplyplayer == 'W') {
		fputs("You are player White, waiting for player Black.\n", stdout);
		char ok;
		receiveInput(servcon, &ok, 1);
		if (ok != 'S') {
			// Bad input cleanup
			fputs("Bad input 1", stdout);
		} 
		fputs("Player Black connected, game starting.\n", stdout);
		// set up logic for being player white
		player = false;
		fputs("\nYour turn to play: \n", stdout);
		printBoard(currBoard.bitboard, player);
		//send info
		while(!parseInput(serverIO, serverIO+1) || !movePiece(serverIO[0], serverIO[1], player));	// split for special interrupts on inputs/parseInput needs to be redone
		printBoard(currBoard.bitboard, player);
		sendOutput(servcon, (char *) &serverIO, 2);
	} else if(serverreplyplayer == 'B') {
		fputs("You are player Black.\n", stdout);
		// set up logic for being player black
		player = true;
		printBoard(currBoard.bitboard, player);
	} else {
		fputs("Bad input 2", stdout);
		// Bad input cleanup
	}

	// forever
	// recv info
	// printBoard
	// send info
	// when game is mate or opponent concedes (not implemented yet) or draws (not implemented) or mate end

	for(;;) {
		// add error handling
		if (!receiveInput(servcon, (char *) &serverIO, 2)) break;
		movePiece(serverIO[0], serverIO[1], !player);
		fputs("Your turn to play: \n", stdout);
		printBoard(currBoard.bitboard, player);
		while(!parseInput(serverIO, serverIO+1) || !movePiece(serverIO[0], serverIO[1], player)); // split for special interrupts on inputs/parseInput needs to be redone
		printBoard(currBoard.bitboard, player);
		if (!sendOutput(servcon, (char *) &serverIO, 2)) break;
	}
	free(servcon);

	fputs("Connection lost...\n", stdout);
	return;
}

// setup socket + bind port
// wait for a connection
// tell that connection they're black
// repeat for(;;) loop above
	
void
onlineMultiplayerHosting()
{
	const U8 s_port = 10;
	char port[s_port];
	Coord serverIO[3];
	//fgets in
	puts("Enter the port you want to use");
	fgets(port, s_port, stdin);
	if (port[strlen(port)-1]!='\n') {
		puts("shit left");
	}

	sas *serv = malloc(sizeof(sas));
	sas *client = malloc(sizeof(sas));
	puts("Opening a server");
	if (!serverHost(port, serv, client)) return;

	puts("Black connected");

	setBitBoard();
	bool player = 0;
	printBoard(currBoard.bitboard, player);
	while(!parseInput(serverIO, serverIO+1) || !movePiece(serverIO[0], serverIO[1], player)); // split for special interrupts on inputs/parseInput needs to be redone
	if (!sendOutput(client, (char *) &serverIO, 2)) goto HOSTINGCONNECTIONLOST;    	
	printBoard(currBoard.bitboard, player);
	for(;;) {
		// add error handling
		if (!receiveInput(client, (char *) &serverIO, 2)) break;
		movePiece(serverIO[0], serverIO[1], !player);
		fputs("Your turn to play: \n", stdout);
		printBoard(currBoard.bitboard, player);
		while(!parseInput(serverIO, serverIO+1) || !movePiece(serverIO[0], serverIO[1], player)); // split for special interrupts on inputs/parseInput needs to be redone
		printBoard(currBoard.bitboard, player);
		if (!sendOutput(client, (char *) &serverIO, 2)) break;
	}
HOSTINGCONNECTIONLOST:
	close(serv->socket_desc);
	close(client->socket_desc);
	free(client);
	free(serv);

	fputs("Connection lost...\n", stdout);
	return;
}

//add something here for not having to take piece or black
char 
findPiece(Coord pos, U8 *piece, bool *colourblack, Board *bitboard) //returns a char representation of a piece for printing. A number as per the enum and a boolean on the colour of the piece.
{
	U8 *extra = malloc(1);
	if (piece == NULL) piece = extra;
	if (colourblack == NULL) colourblack = (bool *) extra;
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
	free(extra);
	return out;
}

Board
calculateAttackVectors(Board *bitboard, bool black) //returns an attack vector for a colour
{
	char piece;
	Board vector = 0;
	for (int i = 0; i < 64; i++) {
		if ((piece=findPiece(i, NULL, NULL, bitboard))!='.') { // find every piece that isn't a blank piece
			if ((piece == 'P') & (!black)) vector |= whitePawnAttackVectors(i); // white and black have different attacks
			if ((piece == 'p') & black) vector |= blackPawnAttackVectors(i);
			piece-=32*black; //if the piece is black subtract so that the char goes into the switch cases nicely
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
		putchar(byte&128?'1':'0');
		byte<<=1;	
	}
	putchar('\n');
}
#endif

void
printBoard(Board *bitboard, bool printblack)
{
	if (printblack) { // print the board upside-down
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('1'+(i/8));
				putchar(' ');
			}
			putchar(findPiece(i, NULL, NULL, bitboard));
		}
		puts("\n\n% HGFEDCBA\n");
	} else {
		for (int i = 0; i < 64; i++) {
			if (!(i%8))  { 
				putchar('\n');
				putchar('8'-(i/8));
				putchar(' ');
			}
			putchar(findPiece(63-i, NULL, NULL, bitboard));
		}
		puts("\n\n% ABCDEFGH\n");
	}
}

bool
parseInput(Coord *coord1, Coord *coord2) 
{
	char l1, l2;
	int n1, n2;
	scanf("%c%d%c%d", &l1, &n1, &l2, &n2);

	if (l1>='a' && l1<='h') l1-=32;
	if (l2>='a' && l2<='h') l2-=32;

	if (l1<'A' || l1>'H') return false;
	if (l2<'A' || l2>'H') return false;

	if (n1<1 || n1>8) return false;
	if (n2<1 || n2>8) return false;

	*coord1 = 7 - l1 + 'A' + (n1-1)*8;
	*coord2 = 7 - l2 + 'A' + (n2-1)*8;
	
	return true;
}

bool 
movePiece(Coord from, Coord to, bool moveblack) 
{
	Boardstate *newbs = malloc(BOARDSTATESIZE); // new boardstate
	if (!fauxMove(from, to, moveblack, currBoard, newbs)) {
		free(newbs);
		return false;
	}

	currBoard.movementflags = newbs->movementflags;
	memcpy(currBoard.bitboard, newbs->bitboard, BITBOARDSIZE); // copy across the new values to the global bitboard

	free(newbs->bitboard);
	free(newbs);
	return true;
}

bool
fauxMove(Coord from, Coord to, bool moveblack, Boardstate bs, Boardstate *nbs)
{
	*nbs = bs;

	U8 movementflags = bs.movementflags;
	Board *bitboard = malloc(BITBOARDSIZE);
	memcpy(bitboard, bs.bitboard, BITBOARDSIZE);
	
	U64 p = 1ULL;;
	bool test = false;
	
	Coord frompiece;
	bool fromcolourblack;

	Coord topiece;
	bool tocolourblack;

	Coord passantpiece;
	bool passantcolourblack;

	findPiece(from, &frompiece, &fromcolourblack, bitboard);
	findPiece(to, &topiece, &tocolourblack, bitboard);

	if (moveblack-fromcolourblack || frompiece==nopiece || !(fromcolourblack^tocolourblack)&(topiece!=nopiece)) {
	       free(bitboard);
       	       return false; // if the from piece doesn't exist or both are the same colour.
	}
	//do testing based on piece
	switch(frompiece) {
		case(pawn):
			if (fromcolourblack) {
				test = blackPawnMovement(from, to, *nbs);
				if (test&2) { //en passant logic
					findPiece(to+8, &passantpiece, &passantcolourblack, bitboard);	
					bitboard[total]^=p<<(to+8);
				}
			} else {
				test = whitePawnMovement(from, to, *nbs);
				if (test&2) {
					findPiece(to-8, &passantpiece, &passantcolourblack, bitboard);	
					bitboard[total]^=p<<(to-8);
				}
			}
			// pawn has reached the end of the board 
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
			test = rookMovement(from, to, bitboard);
			break;
		case(knight):
			test = knightMovement(from, to);
			break;
		case(bishop):
			test = bishopMovement(from, to, bitboard); 
			break;
		case(queen):
			test = queenMovement(from, to, bitboard);
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
			test = kingMovement(from, to, *nbs);
			switch (test) {
				case(2):
					bitboard[rook]^=p<<7;
					bitboard[rook]|=p<<4;
					bitboard[total]^=p<<7;
					bitboard[total]|=p<<4;
					break;
				case(3):
					bitboard[rook]^=1;
					bitboard[rook]|=p<<2;
					bitboard[total]^=1;
					bitboard[total]|=p<<2;
					break;
				case(4): // need to update black bitboard too
					bitboard[black]^=p<<63;
					bitboard[black]|=p<<60;
					bitboard[rook]^=p<<63;
					bitboard[rook]|=p<<60;
					bitboard[total]^=p<<63;
					bitboard[total]|=p<<60;
					break;
				case(5):
					bitboard[black]^=p<<56;
					bitboard[black]|=p<<58;
					bitboard[rook]^=p<<56;
					bitboard[rook]|=p<<58;
					bitboard[total]^=p<<56;
					bitboard[total]|=p<<58;
					break;
			}
			break;
	}

	if (!test) {
		free(bitboard);
		return false;
	}
	// set to piece with opposite colour. remove from piece
	// calc attack vectors
	// if the king is being attacked roll back (no move made)
	//
	// tries to update a piece's position
	if (fromcolourblack) {
		bitboard[black]^=p<<from;	// remove the old piece's position
		bitboard[black]|=p<<to;		// update it to the new spot
	} else 	bitboard[black]&=(-1^(p<<to)); // if the new piece is white it needs to be removed from the black board
	bitboard[frompiece]^=p<<from; // update piece moving
	if (topiece!=nopiece) bitboard[topiece]^=p<<to; // remove taken piece
	bitboard[frompiece]|=p<<to; // move the from piece
	bitboard[total]^=p<<from; // update total board for tracking/finding pieces
	bitboard[total]|=p<<to;	
	

// bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(!moveblack); returns white/black's threatened king position
	// white or black board (player colour) & with king. If attack 
	// (opposite colour) & with the white/black king its under threat.

	// this should be rewritten to be more robust and predictive
	if (bitboard[king]&(((moveblack-1)&bitboard[total])^bitboard[black])&calculateAttackVectors(bitboard, !moveblack)) {
		puts("Check");	
		
		// if the king is under attack we need to undo piece movement
		
		if (fromcolourblack) {
			bitboard[black]^=p<<from;	
			bitboard[black]|=p<<to;	
		} else 	bitboard[black]^=p<<to;	
			
		bitboard[frompiece]^=p<<from; 

		if (topiece!=nopiece) bitboard[topiece]^=p<<to; // remove taken piece
		bitboard[frompiece]&=(-1^p<<to);
		bitboard[total]^=p<<from; // update total board for tracking/finding pieces
		bitboard[total]&=(-1^p<<to);
		
		free(bitboard);
		return false;
	}

	// if a king or rook succesfully moved
	if (frompiece==king) {
		if (from==0x3) movementflags &= 0x3F; // can't castle either way
		if (from==0x3B) movementflags &= 0xCF;
	}

	if (frompiece==rook) {
		if (from==0x7) movementflags &= 0x7F; // cant castle left anymore
		if (from==0) movementflags &= 0xBF; // cant castle right anymore
		if (from==0x3F) movementflags &= 0xDF;
		if (from==0x38) movementflags &= 0xEF;
	}

	Boardstate tempbs = {movementflags, bitboard}; // package the new states together to be returned
	*nbs = tempbs; // returned with new boardstate

	return true;
}
