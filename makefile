OBJS		= main.o moves.o eval.o ai.o
CFLAGS		= -g -Wall


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h types.h

eval.o :	eval.h main.h types.h

ai.o : 		ai.h main.h eval.h types.h

main.o :	main.h moves.h eval.h ai.h types.h 


clean :
		-rm -f $(OBJS) chess
