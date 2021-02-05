#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#ifndef _WIN32

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#endif

#include "types.h"

#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32 // Windows includes

#ifndef WIN32_LEAN_AND_MEAN // reduces overhead
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

typedef struct {
	int socket_desc; // how large is socket_desc? U8/U16??? -> test on windows
	struct sockaddr_in server;
} sas; 

bool serverConnect(char *domain, char *port, sas *socketinfo);
bool receiveInput(sas *socketinfo, char *msg, U8 msglen);
bool sendOutput(sas *socketinfo, char *msg, U8 msglen);


#endif
