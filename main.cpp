#include <stdio.h>
#include <stdlib.h>

#include<winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE         (32 * 1024 * 1024)
#define AUX_BUFFER_SIZE     (8 * 1024)

void GetString(char *pjSon, char *pMP3Url, char *pStr)
{
    char *pMP3Start = strstr(pjSon, pStr);
    pMP3Start += strlen(pStr);
    int i = 0;

    while(*pMP3Start != '\"')
    {
        pMP3Url[i++] = *pMP3Start++;
    }
}

void GetHost(char *pHeader, char *pHostName)
{
    char *pHostStart = strstr(pHeader, "//");
    pHostStart += 2;
    int i = 0;

    while(*pHostStart != '/' && *pHostStart != '?')
    {
        pHostName[i++] = *pHostStart++;
    }
}

SOCKET OpenSocket(char *pIP, short Port)
{
    SOCKET s = 0;
    if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        return -1;
    }

    struct sockaddr_in server = { 0 };
    server.sin_addr.s_addr = inet_addr(pIP);//inet_addr("151.101.16.246"); //inet_addr("http://audio-mp3-fa.spotify.com");

    server.sin_port = htons(Port);
    server.sin_family = AF_INET;

    return connect(s, (struct sockaddr *) &server, sizeof(server)) < 0 ? -1 : s;
}

int main(int argc, char *argv[])
{
    WSADATA wsa = { 0 };
    SOCKET s = 0;

    char *pBuffer = (char *) malloc(BUFFER_SIZE);
    memset(pBuffer, 0, BUFFER_SIZE);

    char *pAuxBuffer = (char *) malloc (AUX_BUFFER_SIZE);
    memset(pAuxBuffer, 0, AUX_BUFFER_SIZE);

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    /**
        Get track id
    */
    printf("Get track id ... ");

    int w = sprintf_s(pBuffer, BUFFER_SIZE, "GET /resolve?url=%s&client_id=bb9d0bcd8f2b1d18d954f7c93d531161 HTTP/1.1\r\n", argv[1]);
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "Host: %s\r\n", "api.soundcloud.com");
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "\r\n");

    if((s = OpenSocket("93.184.220.127", 80)) <= 0)
    {
        printf("Connect error %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    if(send(s, pBuffer, w, 0) < 0)
    {
        printf("Send failed %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    memset(pBuffer, 0, BUFFER_SIZE);
    int rc = 0, i = 0, ReceiveTimeout = 300;

    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&ReceiveTimeout, sizeof(int));
    while((rc = recv(s, pBuffer + i, BUFFER_SIZE - i, 0)) > 0) i += rc;

    char *pDataStart = strstr(pBuffer, "\r\n\r\n") + 4;
    int HeaderSize = (int) (pDataStart + 4 - pBuffer);

    shutdown(s, SD_BOTH); closesocket(s);
    memset(pAuxBuffer, 0, AUX_BUFFER_SIZE);

    GetString(pDataStart, pAuxBuffer, "\"location\":\"");
    printf("done (%s)\n", pAuxBuffer);

    /**
        Get track info
    */
    printf("Get track stream ... ");

    w = sprintf_s(pBuffer, BUFFER_SIZE,  "GET %s&client_id=bb9d0bcd8f2b1d18d954f7c93d531161 HTTP/1.1\r\n", strstr(pAuxBuffer, ".com") + 4);
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "Host: %s\r\n", "api.soundcloud.com");
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "\r\n");


    if((s = OpenSocket("93.184.220.127", 80)) <= 0)
    {
        printf("Connect error %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    if(send(s, pBuffer, w, 0) < 0)
    {
        printf("Send failed %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    memset(pBuffer, 0, BUFFER_SIZE); i = 0;
    while((rc = recv(s, pBuffer + i, BUFFER_SIZE - i, 0)) > 0) i += rc;

    pDataStart = strstr(pBuffer, "\r\n\r\n") + 4;
    HeaderSize = (int) (pDataStart + 4 - pBuffer);

    shutdown(s, SD_BOTH); closesocket(s);
    memset(pAuxBuffer, 0, AUX_BUFFER_SIZE);

    GetString(pDataStart, pAuxBuffer, "\"stream_url\":\"");
    printf("done (%s)\n", pAuxBuffer);

    /**
        Get track stream location 
    */
    printf("Get stream location ... ");

    w = sprintf_s(pBuffer, BUFFER_SIZE,  "GET %s?client_id=bb9d0bcd8f2b1d18d954f7c93d531161 HTTP/1.1\r\n", strstr(pAuxBuffer, ".com") + 4);
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "Host: %s\r\n", "api.soundcloud.com");
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "\r\n");


    if((s = OpenSocket("93.184.220.127", 80)) <= 0)
    {
        printf("Connect error %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    if(send(s, pBuffer, w, 0) < 0)
    {
        printf("Send failed %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    memset(pBuffer, 0, BUFFER_SIZE); i = 0;
    while((rc = recv(s, pBuffer + i, BUFFER_SIZE - i, 0)) > 0) i += rc;

    shutdown(s, SD_BOTH); closesocket(s);
    memset(pAuxBuffer, 0, AUX_BUFFER_SIZE);

    GetString(pDataStart, pAuxBuffer, "\"location\":\"");
    printf("done (%s)\n", pAuxBuffer);

    /**
        Get MP3
    */
    printf("Get MP3 ... ");

    char pHostName[256] = { 0 };
    GetHost(pAuxBuffer, pHostName);

    w = sprintf_s(pBuffer, BUFFER_SIZE, "GET %s HTTP/1.1\r\n", strstr(pAuxBuffer, ".com") + 4);
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "Host: %s\r\n", pHostName);
    w += sprintf_s(pBuffer + w, BUFFER_SIZE - w, "\r\n");

   
    struct hostent *pRemoteHost = gethostbyname(pHostName);
    char *pHostIp = inet_ntoa (*((struct in_addr *) pRemoteHost->h_addr_list[0]));

    if((s = OpenSocket(pHostIp, 80)) <= 0)
    {
        printf("Connect error %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    if(send(s, pBuffer, (int) w, 0) < 0)
    {
        printf("Send failed\n");
        return EXIT_FAILURE;
    }

    i = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&ReceiveTimeout, sizeof(int));
    while((rc = recv(s, pBuffer + i, BUFFER_SIZE - i, 0)) > 0) i += rc;

    shutdown(s, SD_BOTH); closesocket(s);
    WSACleanup();

    pDataStart = strstr(pBuffer, "\r\n\r\n") + 4;
    HeaderSize = (int) (pDataStart + 4 - pBuffer);

    FILE *pFile = NULL;
    fopen_s(&pFile, "music.mp3", "wb");

    char *p = pBuffer + HeaderSize;
    int sz = i - HeaderSize;

    fwrite(pDataStart, sz, 1, pFile);
    fclose(pFile);

    printf("done\n");
    return EXIT_SUCCESS;
}