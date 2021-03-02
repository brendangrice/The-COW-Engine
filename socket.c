#include "socket.h"

void
getDomainAndPort(char *domain, char *port)
{
	char *domainandport = malloc(strlen(domain) + strlen(port));
	memset(domain, 0, strlen(domain));
	memset(port, 0, strlen(port));
	fputs("Enter the domain address you want to connect to: ", stdout);
	readInput(domainandport, 30);
	char *portpos = strchr(domainandport, ':');
	if (portpos==NULL) {
		domain=domainandport;
		fputs("Enter the port you want to connect to: ", stdout);
		readInput(port, 10);
	} else {
		domain = strncat(domain, domainandport, portpos-domainandport);
		port = strncat(port, ++portpos, 9);
	}
	free(domainandport);
}

#ifndef _WIN32

#define SOCKETERRORRET(A, B) {perror(A); close(B); return false;}

bool
serverHost(char *portstr, sas *socketinfo, sas *socketblackinfo)
{
	int socket_desc, socket_black;
	struct sockaddr_in server, sockaddr_black; 
	int port = atoi(portstr);

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		puts("Couldn't create socket");
		return false;
	}
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	if ( bind(socket_desc, (struct sockaddr *)&server , sizeof(server)) < 0) {
		puts("Bind failed");
		return false;
	}

	listen(socket_desc , 3);

	const int s_sockaddr_in = sizeof(struct sockaddr_in);

	while ((socket_black = accept(socket_desc, (struct sockaddr *)&sockaddr_black, (socklen_t*)&s_sockaddr_in))<0); // wait for black to connect

	usleep(1000);

	socketinfo->socket_desc = socket_desc;
	socketinfo->server = server;
	socketblackinfo->socket_desc = socket_black;
	socketblackinfo->server = sockaddr_black;

	char *message = "B";
	write(socket_black, message, strlen(message));

	return true;
}

bool
serverConnect(char *domain, char *port, sas *socketinfo)
{
	char ip[strlen(domain)+strlen(port)];
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


#ifdef _WIN32

#define SOCKETERRORRET(A, B) {perror(A); closesocket(B); WSACleanup(); return false;}

bool
serverConnect(char *domain, char *port, sas *socketinfo) 
{
	WSADATA wsa;
	SOCKET socket_desc;
	struct sockaddr_in server;
	struct in_addr **addr_list;
	struct hostent *he;
	char ip[100];

	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return false;
	}
	
	if ( (he = gethostbyname( domain ) ) == NULL ) {
		puts("Error getting hostname");
		WSACleanup();
		return false;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for (int i = 0; addr_list[i] != NULL; i++) 
		strcpy(ip, inet_ntoa(*addr_list[i]) );

	if((socket_desc = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons( atoi(port) );

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
		SOCKETERRORRET("Couldn't connect to the server", socket_desc);
	
	socketinfo->socket_desc = socket_desc;
	socketinfo->server = server;

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

bool
receiveInput(sas *socketinfo, char *msg, U8 msglen)
{
	//Receive a reply from the server
	int recv_size;
	if((recv_size = recv(socketinfo->socket_desc, msg, msglen, 0)) == SOCKET_ERROR)
		SOCKETERRORRET("Server closed connection", socketinfo->socket_desc);

	if (msglen > 1) msg[msglen] = '\0';
	
	return true;
}
	
#endif
