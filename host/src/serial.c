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
#include "serial.h"

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

void UARTReceive(unsigned char *buffer, int maxLen){
	int x = 0;
		while (x < maxLen) {
			char ch;
			if (read (fd, &ch, 1) != -1) {
				if (ch == '\n')
					break;
				buffer[x++] = ch;
			}
		}	
}


