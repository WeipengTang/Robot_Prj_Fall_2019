/*
 * serial.c
 *
 * This program will demonstrate reading/writing to a Linux
 * serial port. Most PCs still have RS232 serial ports
 * on their backplane - connect up any serial device to one
 * of your ports, and use this as a base for an application
 * that will communicate to that device.
 *
 */

// we need the following for serial port support ...
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "serial.h"
#include "utilities.h"

/*
 * int RS232Init (char *device, int baudrate);
 *
 * this function will open the specified device,
 * at the specified baud rate. This code is typical
 * of most Linux environments in terms of serial
 * port initialization. Note that serial ports
 * often classified as "tty" devices (teletypewriter), as
 * back in the good old days of early Unix, serial ports
 * were used to connect "dumb terminals", and these dumb
 * terminals were often teletypewriter devices (keyboard
 * terminals with a printer, and no screen!!)
 */

static int fd;
extern pthread_mutex_t lock;
extern Robot_control current_robot_control;

int UARTInit(void){
	
char port[] = "/dev/ttyUSB0";
int baud = 9600;

	printf ("attempt to open %s with baud rate %d\n", port, baud);
	fd = RS232Init (port, baud);
	if (fd == -1) {
		printf ("Error: cannot open serial port %s\n", port);
		return 1;
	}	/* endif */
	printf ("successful open of %s with baud rate %d\n", port, baud);
	return 0;
}

int RS232Init (char *device, int baudrate){
	
	int fd;
	struct termios config;
	
	// translate "human readable" baudrate
	// into Linux OS flag for baudrate,
	// as the baud rate value is NOT the same
	// as the configuration flag!

	switch (baudrate) {
	case 300: baudrate = B300; break;
	case 600: baudrate = B600; break;
	case 1200: baudrate = B1200; break;
	case 2400: baudrate = B2400; break;
	case 4800: baudrate = B4800; break;
	case 9600: baudrate = B9600; break;
	case 19200: baudrate = B19200; break;
	case 38400: baudrate = B38400; break;
	case 57600: baudrate = B57600; break;
	case 115200: baudrate = B115200; break;
	default: baudrate = B9600;
	}	/* end switch */

	// open the device (in the /dev folder are devices)
	// and check for errors

	fd = open (device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd <= 0) {
		return -1;
	}

	// configure the terminal device attributes (raw in/out)
	// (you might want to research tcsetattr() for details
	// on configuring terminal (serial port) settings)
	// note: the serial port is configured to be non-blocking
	// for input purposes, hence, when you attempt to read,
	// there may be no data - pay attention to the return value
	// of read() as a result!

	memset (&config, 0, sizeof (config));
	cfsetispeed (&config, baudrate);
	cfsetospeed (&config, baudrate);
	config.c_cflag |= CS8 | CLOCAL | CREAD;
	config.c_iflag = IGNPAR | ICRNL;
	config.c_oflag = 0;
	config.c_lflag = 0;
	config.c_cc[VMIN] = 0;
	config.c_cc[VTIME] = 1;

	tcflush (fd, TCIFLUSH);
	tcsetattr (fd, TCSANOW, &config);

	// return file handle to serial device

	return fd;
}	/* end RS232Init */

void UARTSend(unsigned char *buffer, int len){

	write(fd, buffer, len);
}

int UARTReceive(unsigned char *buffer, int maxLen){
	int x = 0;
	char ch;

		do{		
			if (read (fd, &ch, 1) == -1) {
				return (-1);
			}
			else
				buffer[x++] = ch;
			
		}while((ch !=  '\n') && (x <maxLen));
	return 0;
}

int check_ack(unsigned char instr_num){
	unsigned char buf[3];
	buf[2] = 0;
	char ch;
	read(fd, &ch, 1);
	buf[0] = (unsigned char)ch;
	read(fd, &ch, 1);
	buf[1] = (unsigned char)ch;

	//printf("ack message: %s\n", buf);
	if((buf[0] == 0x06) && (buf[1] == instr_num)){
		printf("ack passed.\n");
		return 0;
	}
	else
		return -1;

}
void *com_manager(void *arg){
	unsigned int i;
	static unsigned int count = 33;
	while(1){
		ping_robot (count);
		for(i=0; i<(0x00FFFFFF);i++);
		verify_robot_ping (count);
		count++;
		if(count >= 128)
			count=33;
		for(i=0; i<(0x0FFFFFFF);i++);
	}
	
}
void ping_robot(unsigned int num){
	char out_buffer[100];
	memset(out_buffer, 0, 100);
	if(current_robot_control.command_option == 0){
	out_buffer[0] = (char)33;
	out_buffer[1] = (char)num;//command number
	out_buffer[2] = (char)33;//command option
	sprintf(out_buffer+3, "$%d$%d$%d$%d$%d$%d$%d$%d$", current_robot_control.stepper_target,
	        								  current_robot_control.stepper_speed,
	        								  current_robot_control.servo_angle,
	        								  current_robot_control.DCM_Left_DIR,
	        								  current_robot_control.DCM_Left_SPD,
	        								  current_robot_control.DCM_Right_DIR,
	        								  current_robot_control.DCM_Right_SPD,
	        								  current_robot_control.LCD_index);
	out_buffer[strlen(out_buffer)+1] = '\n';
	UARTSend ((unsigned char*)out_buffer, (strlen(out_buffer)+2));
	}
	else if(current_robot_control.command_option == 1){
	out_buffer[0] = (char)34;
	out_buffer[1] = (char)num;//command number
	out_buffer[2] = (char)current_robot_control.command_option;
	out_buffer[3] = '\0';
	out_buffer[4] = '\n';
	current_robot_control.command_option = 0;	
	UARTSend ((unsigned char*)out_buffer, (strlen(out_buffer)+2));
	sleep(12);	
	}
	

}
unsigned int verify_robot_ping(unsigned int num){
	unsigned char buffer[100];
	static int error_check=0;
	//int i;
	memset(buffer, 0, 100);
	if(UARTReceive (buffer, 100)!=0)
		error_check++;
	else
		error_check = 0;
	update_robot_info(unsigned char* buffer);
	void print_info_ts(void);

	//printf("%s", buffer);

	if(error_check > 5)
		printf("Robot disconnected\n");
	
	return 0;
}


void update_robot_info(unsigned char* buffer){
		//char temp_buffer[50];		
		//UARTString(temp_buffer);
		char temp_num[5];
		int i=3;
		int j=0;
		int k=0;
		int data_array[8];
		
		do{
			if(buffer[i] == '$')
			{
				j=i+1;
				do{
					j++;

				}while(buffer[j]!='$');

				memcpy(temp_num, temp_buffer+i+1, j-i-1);
				temp_num[j-i-1] = '\0';
				data_array[k] = atoi(temp_num);
				k++;
			}
				i = j;
		}while(k<8);
				
				Robot_info.servo_angle = (int)data_array[0];
				Robot_info.stepper_angle = (int)data_array[1];
				Robot_info.left_direction = (int)data_array[2];
				Robot_info.left_speed = (int)data_array[3];
				Robot_info.right_direction = (int)data_array[4];
				Robot_info.right_speed = (int)data_array[5];
				Robot_info.LCD_index = (int)data_array[6];
				Robot_info.sync_num = (int)data_array[7];
				

}

void print_info_ts(void)
{
clear_sc(); 
printf("Stepper Position: %d\n",  Robot_info.stepper_angle);
printf("Servo Angle: %d\n",  Robot_info.servo_angle);
printf("Left Motor Speed: %d\n",  Robot_info.left_speed);
printf("Left Motor Direction: %d\n",  Robot_info.left_direction);
printf("Right Motor Speed: %d\n",  Robot_info.right_Speed);
printf("Right Motor Direction: %d\n",  Robot_info.right_direction);
printf("Command Number: %d\n",  Robot_info.sync_num);

}







