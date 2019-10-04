#ifndef _SERIAL_H
#define _SERIAL_H

int RS232Init (char *device, int baudrate);
int UARTInit(void);
void UARTSend(unsigned char *buffer, int len);
int UARTReceive(unsigned char *buffer, int maxLen);
int check_ack(unsigned char instr_num);
void *com_manager(void* arg);
void ping_robot(unsigned int num);
unsigned int verify_robot_ping(unsigned int num);
void update_robot_info(unsigned char* buffer);
void print_info_ts(void);


#endif
