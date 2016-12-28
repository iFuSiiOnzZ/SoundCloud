#include "network.h"
#include "../../common/utils.h"

int n_init_network()
{
    WSADATA WsaData = { 0 };
    int r = WSAStartup(MAKEWORD(2, 2), &WsaData);

    return r;
}

int n_close_network()
{
    WSACleanup();
    return 0;
}

SOCKET n_open_socket(char *Host, int Port)
{
    SOCKET s = 0;
    if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        return SOCKET_ERROR_OPEN;
    }

    struct sockaddr_in server = { 0 };
    server.sin_addr.s_addr = inet_addr(Host);

    server.sin_port = htons(Port);
    server.sin_family = AF_INET;

    return connect(s, (struct sockaddr *) &server, sizeof(server)) < 0 ? SOCKET_ERROR_OPEN : s;
}

void n_close_socket(SOCKET s)
{
    shutdown(s, SD_BOTH);
    closesocket(s);
}

int n_send_data(SOCKET s, char *DataBuffer, int BufferSize)
{
    int BytesSend = 0;
    int BytesWritten = 0;

    while(true)
    {
        BytesSend = send(s, DataBuffer + BytesWritten, BufferSize - BytesWritten, 0);

        if(BytesSend < 0)
        {
            return SOCKET_ERROR_SEND;
        }
        else if(BytesSend == 0)
        {
            break;
        }
        else
        {
            BytesWritten += BytesSend;
        }
    }

    return BytesSend;
}

int n_recv_data(SOCKET s, char *DataBuffer, int BufferSize)
{
    int BytesReceived = 0;
    int BytesRead = 0;

    while(true)
    {
        BytesReceived = recv(s, DataBuffer + BytesRead, BufferSize - BytesRead, 0);

        if(BytesReceived < 0)
        {
            return SOCKET_ERROR_RECV;
        }
        else if(BytesReceived == 0)
        {
            break;
        }
        else 
        {        
            BytesRead += BytesReceived;
        }

        if(BytesRead == BufferSize)
        {
            return SOCKET_RECV_MORE_DATA;
        }
    }

    return BytesRead;
}

int n_host_to_ip(char *HostName, char *HostIP)
{
    struct hostent *RemoteHost = { 0 };
    RemoteHost = gethostbyname(HostName);

    if(!RemoteHost)
    {
        return SOCKET_ERROR_HOST_TO_IP;
    }

    struct in_addr **AddrList = NULL;
    AddrList = (struct in_addr **) RemoteHost->h_addr_list;

    for(int i = 0; AddrList[i] != NULL; i++) 
    {
        strcpy(HostIP , inet_ntoa(*AddrList[i]) );
        return SOCKET_NO_ERROR;
    }

    return SOCKET_ERROR_HOST_TO_IP;
}

void n_parse_url(char *URL, url_data_t *OutData)
{
    if(!URL || !OutData) return;
    char *Protocol = strstr(URL, "://");
    bzero(OutData, sizeof(*OutData));

    if(Protocol)
    {
        char *p = URL, *s = URL;
        while(p != Protocol) OutData->Protocol[p - s] = *p++;
    }

    char *StringIndex = Protocol ? Protocol + 3 : URL;
    char *StringStart = StringIndex;

    while(*StringIndex != '/' && *StringIndex != '?')
    {
        OutData->HostName[StringIndex - StringStart] = *StringIndex++;
    }

    int StartWithParam = 0;
    StringStart = StringIndex;
    if(*StringIndex == '?') OutData->Params[0] = '/', StartWithParam = 1;

    while(*StringIndex)
    {
        OutData->Params[StringIndex - StringStart + (StartWithParam ? 1 : 0)] = *StringIndex++;
    }
}