#ifndef _COM_MANAGE_H
#define _COM_MANAGE_H

void *com_manager(void* arg);
void ping_robot(unsigned int num);
unsigned int verify_robot_ping(unsigned int num);
void update_robot_info(unsigned char* buffer);
void print_info_ts(void);

#endif 