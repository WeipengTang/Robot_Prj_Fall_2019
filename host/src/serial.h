#ifndef _SERIAL_H
#define _SERIAL_H

int RS232Init (char *device, int baudrate);
int UARTInit(void);
void UARTSend(unsigned char *buffer, int len);
int UARTReceive(unsigned char *buffer, int maxLen);
//int check_ack(unsigned char instr_num);



#endif
