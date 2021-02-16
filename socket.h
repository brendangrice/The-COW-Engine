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

#ifdef _WIN32 

#ifndef WIN32_LEAN_AND_MEAN // reduces overhead
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h> // Windows includes
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#endif // _WIN32

// TODO more descriptive names
typedef struct {
	U64 socket_desc; // llu on windows
	struct sockaddr_in server;
} sas; 

void getDomainAndPort(char *domain, char *port);
bool serverHost(char *portstr, sas *socketinfo, sas *socketblackinfo);
bool serverConnect(char *domain, char *port, sas *socketinfo);
bool receiveInput(sas *socketinfo, char *msg, U8 msglen);
bool sendOutput(sas *socketinfo, char *msg, U8 msglen);


#endif // SOCKET_H
