#include "utilities.h"
#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>

static char file_path[] = "/home/pi/Documents/Robot_project/camera/image.png";
static int32_t file_size;

int32_t get_file_size(void){
	
	FILE* fp;
	fp = fopen(file_path, "rb");
	
	//check if image is opened properly
	if(fp == NULL){
		printf("Failed to open image file.\n");
		fclose(fp);
		return -1;

	}
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fclose(fp);
	return file_size;

}

int32_t send_image_socket(int client_sockfd){
	FILE* fp;
	char* buffer;
	int sent = 0;
	//int clientLength;
	//char *END_FLAG = "=================END";
	fp = fopen(file_path, "rb");
	
	//check if image is opened properly
	if(fp == NULL){
		printf("Failed to open image file.\n");
		fclose(fp);
		return -1;
	}

	//allocate space for image
	buffer = (char *)malloc(sizeof(char)*file_size);
	if(buffer == NULL){
		printf("Failed to allocate space for file.\n");
		fclose(fp);
		free(buffer);
		return -1;
	}

	//read the file into buffer;
	memset(buffer, 0, file_size);
	fread(buffer, 1, file_size, fp);
	fclose(fp);

	//send out the data in buffer
	//clientLength = sizeof(clientMachine);
	//printf ("Client IP: %s\n", inet_ntoa(clientMachine.sin_addr));
	//printf ("Client Port: %u\n", clientMachine.sin_port);
	printf("file size: %d\n", file_size);
/*	for(int i=0; i < file_size; i++){
		sent += sendto (udpSocket, buffer+i, 1, 0, 
						(struct sockaddr *) &clientMachine, clientLength);
	}
	sendto(udpSocket, END_FLAG, strlen(END_FLAG), 0,
	       (struct sockaddr *) &clientMachine, clientLength);*/
	sent = write(client_sockfd, buffer, file_size);
	printf("Sent file size: %d\n", sent);
	free(buffer);
	return 0;
}