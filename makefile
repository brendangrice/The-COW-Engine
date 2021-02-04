OBJS		= shit_chess.o moves.o socket.o
CFLAGS		= -g -Og -Wall


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h types.h

shit_chess.o :	shit_chess.h moves.h types.h

socket.o :	socket.h types.h

clean :
		-rm -f $(OBJS) chess
