#include "socket.h"

#ifndef _WIN32

#define SOCKETERRORRET(A, B) {perror(A); close(B); return false;}

bool 
serverConnect(char *domain, char *port, sas *socketinfo) 
{
	char ip[100];
	struct hostent *he;
	int socket_desc;
	struct in_addr **addr_list;
	struct sockaddr_in server;
	
	if ( (he = gethostbyname( domain ) ) == NULL ) {
		puts("Error getting hostname");
		return false;
	}
	
	addr_list = (struct in_addr **) he->h_addr_list;

	for(int i=0; addr_list[i] != NULL; i++) strcpy(ip, inet_ntoa(*addr_list[i]));
	

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) SOCKETERRORRET("Error making a socket", socket_desc);
	
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons( atoi(port) );

	if(connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0 )
		SOCKETERRORRET("Error connecting", socket_desc);
	

	socketinfo->socket_desc = socket_desc;
	socketinfo->server = server;

	return true;
}

bool
receiveInput(sas *socketinfo, char *msg, U8 msglen)
{
	if(recv(socketinfo->socket_desc, msg, msglen, 0)<=0) 
		SOCKETERRORRET("Connection closed", socketinfo->socket_desc);

	if (strlen(msg) == 0) SOCKETERRORRET("Bad input", socketinfo->socket_desc);

	if (msglen > 1) msg[msglen] = '\0';

	return true;
}

bool
sendOutput(sas *socketinfo, char *msg, U8 msglen)
{
	if (msglen > 1) msg[msglen] = '\0';
	if (send(socketinfo->socket_desc , msg , strlen(msg) , 0) < 0)
	       	SOCKETERRORRET("Error sending", socketinfo->socket_desc);
	
	return true;
}

#endif
