OBJS		= shit_chess.o moves.o 
CFLAGS		= -O3 -fexpensive-optimizations 


chess :		$(OBJS) 	
	$(CC) -o chess $(OBJS)

moves.o :	moves.h

shit_chess.o :	shit_chess.h types.h 


clean :
		rm $(OBJS) chess
