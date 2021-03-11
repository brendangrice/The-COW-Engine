OBJS		= gamemodes.o moves.o pgn.o eval.o ai.o main.o
CFLAGS		= -g -Wall -D DEBUG


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)
gamemodes.o:	gamemodes.h main.h pgn.h types.h	

moves.o :	moves.h main.h types.h

pgn.o :		pgn.h main.h types.h

eval.o :	eval.h main.h types.h

ai.o : 		ai.h main.h eval.h types.h

main.o :	main.h moves.h eval.h ai.h types.h 


clean :
		-rm -f $(OBJS) chess
