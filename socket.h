#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "types.h"

#ifndef SOCKET_H
#define SOCKET_H

typedef struct {
	int socket_desc; // how large is socket_desc? U8/U16??? -> test on windows
	struct sockaddr_in server;
} sas; 

bool serverConnect(char *domain, char *port, sas *socketinfo);
bool receiveInput(sas *socketinfo, char *msg, U8 msglen);
bool sendOutput(sas *socketinfo, char *msg, U8 msglen);


#endif
