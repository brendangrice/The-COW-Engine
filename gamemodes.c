#include "gamemodes.h"

const U8 strsize = 6; // largest input that will be accepted by stdin

// maybe get this to return something different depending on who wins?
void
localMultiplayer(Boardstate *currBoard)
{
	char s[strsize]; // input string
	U8 result;
	Coord from, to; // movements

	puts("White to play");
	printBoard(*currBoard);

	while(1) { 
		from = 0;
		to = 0;
		readInput(s, strsize);
		result = parseInput(s, &from, &to);
		if (result==2) break; // quit
		if (!result) continue; // check if it successfully parsed
		movePiece(from, to); // actually move the piece and update currBoard

		currBoard->blackplaying=!currBoard->blackplaying; // switch players
		if(inCheckMate(*currBoard)) {
			printBoard(*currBoard);
			puts("Checkmate");
			break; // end game
		}
		if(inStaleMate(*currBoard)) {
			printBoard(*currBoard);
			puts("Stalemate");
			break; // end game
		}
		currBoard->blackplaying?puts("\nBlack to play"):puts("\nWhite to play");
		if(inCheck(*currBoard)) puts("Check");
		printBoard(*currBoard);
	}

	return;
}

void
onlineMultiplayer(Boardstate *currBoard)
{
	char s[strsize]; // input string
	U8 result;
	setBitBoard();
	//add something to handle ip
	
	char domain[20];
	char port[10];
	getDomainAndPort(domain, port);
	sas *servcon = malloc(sizeof(sas));
	puts("Attempting to connect...");
	if(!serverConnect(domain, port, servcon)) {
		free(servcon);
		return;	
	};
	char serverreplyplayer;
	Coord serverIO[3];
	puts("Querying...");
	receiveInput(servcon, &serverreplyplayer, 1);
	if (serverreplyplayer == 'W') {
		puts("You are player White, waiting for player Black.");
		char ok;
		receiveInput(servcon, &ok, 1);
		if (ok != 'S') {
			// Bad input cleanup
			puts("Bad input 1");
		} 
		puts("Player Black connected, game starting.");
		// set up logic for being player white
		puts("Your turn to play: ");
		printBoard(*currBoard);
		//send info

		while (!result) {
			readInput(s, strsize);
			result = parseInput(s, &serverIO[0], &serverIO[1]);
			if (result==2) goto CLIENTCONNECTIONLOST; // quit
		}
		movePiece(serverIO[0], serverIO[1]); // actually move the piece and update currBoard
		
		printBoard(*currBoard);
		currBoard->blackplaying = !currBoard->blackplaying;
		sendOutput(servcon, (char *) &serverIO, 2);
	} else if(serverreplyplayer == 'B') {
		puts("You are player Black.");
		// set up logic for being player black
		currBoard->blackplaying = !currBoard->blackplaying;
		printBoard(*currBoard);
		currBoard->blackplaying = !currBoard->blackplaying;
	} else {
		puts("Bad input 2");
		// Bad input cleanup
	}

	// forever
	// recv info
	// printBoard
	// send info
	// when game is mate or opponent concedes (not implemented yet) or draws (not implemented) or mate end

	for(;;) {
		result = 0;
		// add error handling
		if (!receiveInput(servcon, (char *) &serverIO, 2)) break;
		movePiece(serverIO[0], serverIO[1]);
		currBoard->blackplaying = !currBoard->blackplaying;
		printBoard(*currBoard);

		while (!result) {
			readInput(s, strsize);
			result = parseInput(s, &serverIO[0], &serverIO[1]);
			if (result==2) goto CLIENTCONNECTIONLOST; // quit
		}
		movePiece(serverIO[0], serverIO[1]); // actually move the piece and update currBoard

		printBoard(*currBoard);
		currBoard->blackplaying = !currBoard->blackplaying;
		if (!sendOutput(servcon, (char *) &serverIO, 2)) break;
	}

CLIENTCONNECTIONLOST:
	close(servcon->socket_desc);
	free(servcon);
	puts("Connection lost...\n");
	return;
}

// setup socket + bind port
// wait for a connection
// tell that connection they're black
// repeat for(;;) loop above
	
void
onlineMultiplayerHosting(Boardstate *currBoard)
{
	char s[strsize]; // input string
	U8 result;

	const U8 s_port = 10;
	char port[s_port];
	Coord serverIO[3];

	fputs("Enter the port you want to use: ", stdout);
	readInput(port, s_port);

	sas *serv = malloc(sizeof(sas));
	sas *client = malloc(sizeof(sas));
	puts("Opening a server");
	if (!serverHost(port, serv, client)) return;

	puts("Black connected.\n");

	puts("Your turn to play:");
	setBitBoard();
	printBoard(*currBoard);
	
	while (!result) {
		readInput(s, strsize);
		result = parseInput(s, &serverIO[0], &serverIO[1]);
		if (result==2) goto HOSTINGCONNECTIONLOST; // quit
	}
	movePiece(serverIO[0], serverIO[1]); // actually move the piece and update currBoard
	
	if (!sendOutput(client, (char *) &serverIO, 2)) goto HOSTINGCONNECTIONLOST;    	
	printBoard(*currBoard);
	currBoard->blackplaying = !currBoard->blackplaying;
	for(;;) {
		result = 0;
		// add error handling
		if (!receiveInput(client, (char *) &serverIO, 2)) break;
		movePiece(serverIO[0], serverIO[1]);
		currBoard->blackplaying = !currBoard->blackplaying;
		puts("Your turn to play:");
		printBoard(*currBoard);
		
		while (!result) {
			readInput(s, strsize);
			result = parseInput(s, &serverIO[0], &serverIO[1]);
			if (result==2) goto HOSTINGCONNECTIONLOST; // quit
		}
		movePiece(serverIO[0], serverIO[1]); // actually move the piece and update currBoard
		
		printBoard(*currBoard);
		currBoard->blackplaying = !currBoard->blackplaying;
		if (!sendOutput(client, (char *) &serverIO, 2)) break;
	}
HOSTINGCONNECTIONLOST:
	close(serv->socket_desc);
	close(client->socket_desc);
	free(client);
	free(serv);

	puts("Connection lost...");
	return;
}
