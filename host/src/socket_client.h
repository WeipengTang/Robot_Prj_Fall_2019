#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

int socket_client_init(void);
int socket_send(char *buffer, int len);
int socket_receive (char *buffer, int maxLen);
void request_image(void);
void receive_image(void);
#endif