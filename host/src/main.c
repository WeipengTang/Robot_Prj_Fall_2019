#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "serial.h"

int main (void){

	UARTInit();
	char out_buf[100];
	char in_buf[100];


	while(1){
		printf ("Enter a string: ");
		fgets (out_buf, 100, stdin);
		UARTSend(out_buf, strlen(out_buf));
		
		sleep (1);

		printf ("reading ...\n");
		memset (in_buf, 0, 100);
		UARTReceive (in_buf, 100);
		
		printf ("got this from port: >>%s<<\n", in_buf);
	}
	// close the file handle for the serial device


	return 0;
}	/* end main */




