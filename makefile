OBJS		= main.o moves.o
CFLAGS		= -g -Wall


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h

main.o :	main.h types.h


clean :
		rm $(OBJS) chess
