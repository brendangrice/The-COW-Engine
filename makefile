OBJS		= main.o moves.o socket.o
CFLAGS		= -g -Wall

chess :		$(OBJS)
	$(CC) -o chess $(OBJS)

moves.o :	moves.h types.h

main.o :	main.h moves.h socket.h types.h

socket.o : 	socket.h main.h types.h

clean :
		-rm -f $(OBJS) chess
