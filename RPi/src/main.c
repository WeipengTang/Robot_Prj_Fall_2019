/*
 * udpserver.c
 *
 * This example shows how to code up a UDP (connectionless)
 * server. Note that we no longer have a listening queue or
 * perform an accept. This is because UDP does not support 
 * the concept of reliable connections - UDP simply provides a
 * lightweight means of sending messages from one system to 
 * another. You, as a programmer, must figure out what to do
 * in case a message is dropped, lost, or delayed!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include "serial.h"
#include "utilities.h"

#define MAXINPUTLEN		 4096
char input[MAXINPUTLEN] = "";

int main(int argc, char *argv[])
{
	int udpSocket = 0;
	int udpPort = 0;
	int status = 0;
	int size = 0;
	unsigned int clientLength = 0;
	struct sockaddr_in serverMachine = { 0 };
	struct sockaddr_in clientMachine = { 0 };

	UARTInit();

	// ensure we have correct arguments

	if (argc < 2) {
		udpPort = 6000;
	}else
		udpPort = atoi(argv[1]);

	// allocate a socket

	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket < 0) {
		printf ("Error: can't get socket!\n");
		return 2;
	}	/* endif */

	// zero out socket structures and fill
	// in with details for our UDP server

	memset (&serverMachine, 0, sizeof (serverMachine));
	memset (&clientMachine, 0, sizeof (clientMachine));

	serverMachine.sin_family = AF_INET;
	serverMachine.sin_addr.s_addr = htonl (INADDR_ANY);
	serverMachine.sin_port = htons (udpPort);

	// bind the socket structure to the socket
	// to establish our server

	status = bind (udpSocket, (struct sockaddr *)&serverMachine, 
			sizeof (serverMachine));
	if (status < 0) {
		printf ("Error: can't bind\n");
		return 3;
	}	/* endif */

	while (1) {

		clientLength = sizeof(clientMachine);
		size = recvfrom (udpSocket, input, MAXINPUTLEN, 0, 
			(struct sockaddr *) &clientMachine, &clientLength);
		if (size < 0) {
			printf ("Error: recvfrom failure\n");
			return 4;
		}	

		printf ("Client IP: %s\n", inet_ntoa(clientMachine.sin_addr));
		printf ("Client Port: %u\n", clientMachine.sin_port);
		printf ("ClientLength: %d size: %d Message received: %s\n", clientLength, size, input);
		UARTSend (input, size);

		
		//for(i=0; i<(0x006FFFFF); i++);
		msleep(50);
		memset(input, 0, MAXINPUTLEN);
		UARTReceive (input, MAXINPUTLEN);
		size = strlen(input)+1;
		clientLength = sizeof (clientMachine);
		status = sendto (udpSocket, input, size, 0, 
			(struct sockaddr *) &clientMachine, clientLength);
		if (status < 0) {
			printf ("Error: sendto error\n");
			return 5;
		}	/* endif */
	}	/* end while loop */

	/* as a server, this should never be reached */

	return 0;
}

