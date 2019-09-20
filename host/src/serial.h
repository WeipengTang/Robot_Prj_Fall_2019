#ifndef _SERIAL_H
#define _SERIAL_H

void UARTInit(void);
void UARTSend(char *buffer, int len);
void UARTReceive(char *buffer, int maxLen);

#endif