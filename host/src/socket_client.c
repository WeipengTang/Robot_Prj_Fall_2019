#include "socket_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


//char input[MAXINPUTLEN] = "";
struct sockaddr_in serverMachine = { 0 };
int udpSocket = 0;

int socket_client_init(void){
	char ip[100] = "10.192.27.229";
	int udpPort = 6000;
	

	struct hostent *host;

	/*printf ("Enter server IP address: ");
	gets (ip);*/
	if ((host = gethostbyname (ip)) == NULL) {
		printf ("Error: can't get host information!\n");
		return 1;
	}

/*	printf ("Enter server port number: ");
	gets (input);
	udpPort = atoi (input);*/
	
	// allocate a socket

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket < 0) {
		printf ("Error: can't get socket!\n");
		return 2;
	}	

	//Set the socket to non-blocking mode
	int flags = fcntl(udpSocket, F_GETFL, 0);
	fcntl(udpSocket, F_SETFL, O_NONBLOCK|flags);

	
	// zero out socket structures and fill
	// in with details for our UDP server

	memset (&serverMachine, 0, sizeof (serverMachine));

	serverMachine.sin_family = AF_INET;
	memcpy (&serverMachine.sin_addr, host->h_addr, host->h_length);
	serverMachine.sin_port = htons (udpPort);
	
	return 0;

}

int socket_send(char *buffer, int len){
	ssize_t status = 0;
	int size = 0;
	
	size = sizeof (serverMachine);
	status = sendto (udpSocket, buffer, len, 0, 
		(struct sockaddr *) &serverMachine, size);
	//printf ("status: %d size: %d\n", status, size);
	if (status < 0) {
		printf ("Error: sendto error (short write)\n");
		return -1;
	}
	return size;

}

int socket_receive(char *buffer, int maxLen){

	ssize_t size = 0;
	unsigned int serverLength = 0;
	memset (buffer, 0, maxLen);
	size = recvfrom (udpSocket, buffer, maxLen, 0, 
		(struct sockaddr *) &serverMachine, &serverLength);
	//printf("receive size: %d\n", size);
	if (size < 2) {
		printf ("Error: recvfrom failure\n");
		return -1;
	}

	return size;
}
