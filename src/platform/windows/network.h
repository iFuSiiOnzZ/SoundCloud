#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SOCKET_NO_ERROR     ( 0)

#define SOCKET_ERROR_OPEN   (-1)
#define SOCKET_ERROR_SEND   (-2)
#define SOCKET_ERROR_RECV   (-3)

#define SOCKET_ERROR_HOST_TO_IP   (-4)

#define SOCKET_RECV_NO_DATA   (  0)
#define SOCKET_RECV_MORE_DATA (-24)

typedef struct url_data_t
{
    char Protocol[8];
    char HostName[64];
    char Params[1024];
} url_data_t;

int n_init_network();
int n_close_network();

SOCKET n_open_socket(char *Host, int Port);
void n_close_socket(SOCKET s);

int n_send_data(SOCKET s, char *DataBuffer, int BufferSize);
int n_recv_data(SOCKET s, char *DataBuffer, int BufferSize);

void n_parse_url(char *URL, url_data_t *OutURLData);
int n_host_to_ip(char *HostName, char *HostIP);



#endif