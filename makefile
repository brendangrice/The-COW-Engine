OBJS		= gamemodes.o moves.o socket.o pgn.o main.o
CFLAGS		= -g -Wall

chess :		$(OBJS)
	$(CC) -o chess $(OBJS)
gamemodes.o:	gamemodes.h main.h pgn.h types.h	

moves.o :	moves.h main.h types.h

pgn.o :		pgn.h main.h types.h

main.o :	main.h moves.h socket.h types.h

socket.o : 	socket.h main.h types.h

clean :
		-rm -f $(OBJS) chess
