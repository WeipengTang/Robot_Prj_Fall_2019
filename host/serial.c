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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// we need the following for serial port support ...
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>



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

int
RS232Init (char *device, int baudrate)
{
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



/*
 * int main (int argc, char *argv[]);
 *
 * tests out serial port access via above API
 *
 * arguments to main()
 * 1: baud rate 2: serial port (tty device)
 *
 * we assume that comms are 8 bits, no parity, 1 stop
 * defaults will be 9600 baud, "/dev/ttyS0", as
 * for most PC based Linux installations, the serial
 * ports will be listed as devices /dev/ttySx, where
 * x is 0 for COM1, 1 for COM2, and so forth.
 */

int
main (int argc, char *argv[])
{
	int baud;
	int fd, x;
	char *port;
	char buf[1000];

	// on our Linux system, the standard serial port is 
	// device "/dev/ttyS0" - so assume this is the default

	if (argc < 2) {
		port = "/dev/ttyS1";
		baud = 9600;
	} else if (argc < 3) {
		port = "/dev/ttyS1";
		baud = atoi (argv[1]);
	} else {
		port = argv[2];
		baud = atoi (argv[1]);
	}	/* endif */

	printf ("attempt to open %s with baud rate %d\n", port, baud);
	fd = RS232Init (port, baud);
	if (fd == -1) {
		printf ("Error: cannot open serial port %s\n", port);
		return 1;
	}	/* endif */
	printf ("successful open of %s with baud rate %d\n", port, baud);

	/*
	 * send a bunch of characters to the serial port, wait
	 * one second for the other side to send a response, 
	 * and attempt to read something back
	 */
	//while(1){
		printf ("Enter a string: ");
		fgets (buf, 1000, stdin);
		//buf[6] = '\n';
		write (fd, buf, strlen(buf));

		sleep (1);

		printf ("reading ...\n");
		memset (buf, 0, 1000);
		x = 0;
		while (x < 999) {
			char ch;

			// read one char at a time, and ignore
			// read errors (the port is configured as 
			// non-blocking, so we're not stuck waiting
			// for a byte to arrive!

			if (read (fd, &ch, 1) != -1) {
				if (ch == '\n')
					break;
				buf[x++] = ch;
			}
		}	/* end while */
		printf ("got this from port: >>%s<<\n", buf);
		
	//}
	// close the file handle for the serial device

	close (fd);
	return 0;
}	/* end main */


