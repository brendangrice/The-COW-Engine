OBJS		= shit_chess.o moves.o 
CFLAGS		= -O3 -fexpensive-optimizations 

chess :	 	$(OBJS)
		$(CC) -o chess $(OBJS)	

moves.o :	moves.c moves.h
		$(CC) $(CFLAGS) -c moves.c

shit_chess.o :	shit_chess.c shit_chess.h types.h 
		$(CC) $(CFLAGS) -c shit_chess.c


clean :
		rm $(OBJS) chess
