OBJS		= shit_chess.o moves.o eval.o ai.o
CFLAGS		= -g -Wall


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h types.h

shit_chess.o :	shit_chess.h moves.c eval.h ai.h types.h

eval.o :	eval.h shit_chess.h types.h

ai.o : 		ai.h shit_chess.h eval.h types.h


clean :
		rm $(OBJS) chess
