OBJS		= shit_chess.o moves.o
CFLAGS		= -g -Wall


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h

shit_chess.o :	shit_chess.h types.h


clean :
		rm $(OBJS) chess
