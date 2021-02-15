OBJS		= main.o moves.o
CFLAGS		= -g -Wall -D DEBUG


chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h types.h

main.o :	main.h types.h


clean :
		-rm -f $(OBJS) chess
