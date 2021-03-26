#include "fen.h"

Boardstate
parseFEN(char *FEN)
{
	strrep(FEN, '\n', 0);
	Boardstate bs = makeBoardstate(NULL, 0xF0, false, 1, 1);
	// reset the board
	for(int i = 0 ; i < BITBOARDELEMENTS; i++)
	{
		bs.bitboard[i] = 0;
	}
	
	bs.movementflags = 0;
	bs.blackplaying = false;
	bs.fiftymove = 0;
	bs.halfmove = 0;
	
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
	
	FEN++;	

	U8 enpassant = 'a'-*FEN+7;
	if (enpassant>0 && enpassant<7) { // en passant
		bs.movementflags |= enpassant;
		bs.movementflags |= 0x8;
		FEN++; // don't need to know the file
	}

	FEN+=2;
	char str[10];
	char *ptr = str;
	strncpy(str, FEN, 10);

	while (' '!=*(ptr++));
	bs.fiftymove = atoi(str);
	bs.halfmove = atoi(ptr);

	bs.halfmove*=2;
	if (!bs.blackplaying) bs.halfmove--;

	bs.key = generateHash(bs); // generate key
	return bs;
}

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
	// fifty move clock
	printf(" %d", bs.halfmove-bs.fiftymove);
	// fullmove clock
	printf(" %d", bs.halfmove/2);
	printf("\n");
}

