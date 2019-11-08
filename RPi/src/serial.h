#ifndef _SERIAL_H
#define _SERIAL_H

int RS232Init (char *device, int baudrate);
int UARTInit(void);
void UARTSend(char *buffer, int len);
int UARTReceive(char *buffer, int maxLen);




#endif
