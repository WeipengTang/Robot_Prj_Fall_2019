#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <time.h>
#include <unistd.h>
#include "client.h"
#include "controller.h"

void delay(unsigned int number_of_seconds);
void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int size = 0;
    int read_size, write_size;
    int recv_size;
    int sockfd, portno, n;
	struct timeval tv;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    controllerinit();

    struct info send;
    struct info *sendptr;
    sendptr = &send;
    struct motorctrl motorsnd; 

    sendptr->Motor[0] = 0;
    sendptr->Motor[1] = 0;
    sendptr->servo = 0;
    sendptr->stepper = 0;
    sendptr->image = 0;

    int structsize = sizeof(send);
    printf("%d\n", structsize);

	tv.tv_sec = 1;
	tv.tv_usec = 0;
    char buffer[1024];
	char input[1024];
    char* p_array;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
        }


	while(1){
		//printf("Please enter the message: \n");
		bzero(buffer,1024);
		bzero(input,1024);
        fflush(stdout);
		//fgets(sendptr->lcdstring,32,stdin);
        controller(sendptr, &motorsnd);
        setdirection(sendptr, &motorsnd);
        //strcpy (buffer, "image");
        memcpy(buffer, sendptr, structsize);
        
		n = write(sockfd,buffer,structsize);
    	printf("Sent: %d\n", structsize);

        if (sendptr->image != 0){
            printf("Recieving image\n");
            FILE *image;

            image = fopen("image.png", "wb");
            if(image == NULL){
                printf("Error opening image file");
                return -1;
            }
            read(sockfd, &size, sizeof(int));

            printf("%d", size);
            p_array = (char*) malloc (sizeof(char)* size);

            while(recv_size < size){
            //Read Picture Byte Array


                bzero(p_array,size);
                read_size = read(sockfd, p_array, size);

            //Convert it Back into Picture
                write_size = fwrite(p_array, 1, read_size, image);

                if(read_size != write_size) {
                    printf("error in read write\n");    }
                recv_size += read_size;

            }
            recv_size = 0;
            fclose(image);
            free(p_array);
            fflush(stdout);
            //delay(1000);

        }
        else{
            n = read(sockfd,input,1024);		
            printf("Response from server: %s\n",input);
        }
        usleep(50000);
    }

    return 0;
}


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}
  
