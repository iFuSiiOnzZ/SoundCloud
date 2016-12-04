#include <time.h>
#include "soundcloud.h"

#define SOUNDCLOUD_USER_ID "fDoItMDbsbZz8dY16ZzARCZmzgHBPotA"
#define SOUNDCLOUD_NETWORK_IP "93.184.220.127"
#define SOUNDCLOUD_NETWORK_PT 80

#define DATA_BUFFER_SIZE 16384

void sc_get_track_location(char *TrackURL, sc_track_location_t *OutData)
{
    char TemplateURL[] =
        "GET /resolve?url=%s&client_id=%s HTTP/1.1\r\n"
        "Host: api.soundcloud.com\r\n"
        "Connection: close\r\n"
        "\r\n";

    SOCKET s = 0;
    char DataBuffer[DATA_BUFFER_SIZE] = { 0 };
    
    if((s = n_open_socket(SOUNDCLOUD_NETWORK_IP, SOUNDCLOUD_NETWORK_PT)) == SOCKET_ERROR_OPEN)
    {
        printf("Connect error %d (%s: %d)\n", WSAGetLastError(), __FILE__, __LINE__);
        return;
    }

    int w = sprintf_s(DataBuffer, TemplateURL, TrackURL, SOUNDCLOUD_USER_ID);
    n_send_data(s, DataBuffer, w);

    int r = n_recv_data(s, DataBuffer, DATA_BUFFER_SIZE);
    DataBuffer[r] = 0;

    char *ContentStart = strstr(DataBuffer, "\r\n\r\n");

    if(ContentStart)
    {
        ContentStart += 4;

        JS_TOKENIZER Tokenizer = { ContentStart };
        JS_NODE *pRootNode = json_root();

        json_parser(pRootNode, &Tokenizer);
        json_sanitize(pRootNode);

        char *Location = json_value(pRootNode, "root.location");
        if(!Location) return;

        strcpy_s(OutData->Location, Location);
        json_clear(pRootNode);
    }

    n_close_socket(s);
}

void sc_get_track_info(char *TrackLocation, sc_track_info_t *OutData)
{
    char TemplateURL[] =
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n";

    SOCKET s = 0;
    url_data_t URLData = { 0 };

    char HostIP[64] = { 0 };
    char DataBuffer[DATA_BUFFER_SIZE] = { 0 };

    n_parse_url(TrackLocation, &URLData);
    n_host_to_ip(URLData.HostName, HostIP);
    
    if((s = n_open_socket(HostIP, SOUNDCLOUD_NETWORK_PT)) == SOCKET_ERROR_OPEN)
    {
        printf("Connect error %d (%s: %d)\n", WSAGetLastError(), __FILE__, __LINE__);
        return;
    }

    int w = sprintf_s(DataBuffer, TemplateURL, URLData.Params, URLData.HostName);
    n_send_data(s, DataBuffer, w);

    int r = n_recv_data(s, DataBuffer, DATA_BUFFER_SIZE);
    DataBuffer[r] = 0;

    char *ContentStart = strstr(DataBuffer, "\r\n\r\n");

    if(ContentStart)
    {
        ContentStart += 4;

        JS_TOKENIZER Tokenizer = { ContentStart };
        JS_NODE *pRootNode = json_root();

        json_parser(pRootNode, &Tokenizer);
        json_sanitize(pRootNode);

        strcpy_s(OutData->Title, json_value(pRootNode, "root.title"));
        strcpy_s(OutData->StreamURL, json_value(pRootNode, "root.stream_url"));

        strcpy_s(OutData->URI, json_value(pRootNode, "root.uri"));
        strcpy_s(OutData->ArtWorkURL, json_value(pRootNode, "root.artwork_url"));

        json_clear(pRootNode);
    }

    n_close_socket(s);   
}

void sc_get_track_streams(char *StreamLocation, sc_strems_urls_t *OutData)
{
    char TemplateURL[] =
        "GET %ss?client_id=%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n";

    SOCKET s = 0;
    url_data_t URLData = { 0 };

    char HostIP[64] = { 0 };
    char DataBuffer[DATA_BUFFER_SIZE] = { 0 };

    n_parse_url(StreamLocation, &URLData);
    n_host_to_ip(URLData.HostName, HostIP);
    
    if((s = n_open_socket(HostIP, SOUNDCLOUD_NETWORK_PT)) == SOCKET_ERROR_OPEN)
    {
        printf("Connect error %d (%s: %d)\n", WSAGetLastError(), __FILE__, __LINE__);
        return;
    }

    int w = sprintf_s(DataBuffer, TemplateURL, URLData.Params, SOUNDCLOUD_USER_ID, URLData.HostName);
    n_send_data(s, DataBuffer, w);

    int r = n_recv_data(s, DataBuffer, DATA_BUFFER_SIZE);
    DataBuffer[r] = 0;

    char *ContentStart = strstr(DataBuffer, "\r\n\r\n");

    if(ContentStart)
    {
        ContentStart += 4;
        char *jsString = 0;

        std::string s(ContentStart);
        s = replace_all(s, "\\u0026", "&");

        JS_TOKENIZER Tokenizer = { (char *) s.c_str() };
        JS_NODE *pRootNode = json_root();

        json_parser(pRootNode, &Tokenizer);
        json_sanitize(pRootNode);

        jsString = json_value(pRootNode, "root.hls_mp3_128_url");
        if(jsString) strcpy_s(OutData->hls_mp3_128_url, jsString);

        jsString = json_value(pRootNode, "root.http_mp3_128_url");
        if(jsString) strcpy_s(OutData->http_mp3_128_url, jsString);

        jsString = json_value(pRootNode, "root.rtmp_mp3_128_url");
        if(jsString) strcpy_s(OutData->rtmp_mp3_128_url, jsString);

        jsString = json_value(pRootNode, "root.preview_mp3_128_url");
        if(jsString) strcpy_s(OutData->preview_mp3_128_url, jsString);

        json_clear(pRootNode);
    }



    n_close_socket(s);   
}

void sc_download_track(char *StreamURL, char *FileName /* = 0 */)
{
    char TemplateURL[] =
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n";

    SOCKET s = 0;
    url_data_t URLData = { 0 };

    char HostIP[64] = { 0 };
    char DataBuffer[DATA_BUFFER_SIZE] = { 0 };

    n_parse_url(StreamURL, &URLData);
    n_host_to_ip(URLData.HostName, HostIP);
    
    if((s = n_open_socket(HostIP, SOUNDCLOUD_NETWORK_PT)) == SOCKET_ERROR_OPEN)
    {
        printf("Connect error %d (%s: %d)\n", WSAGetLastError(), __FILE__, __LINE__);
        return;
    }

    int w = sprintf_s(DataBuffer, TemplateURL, URLData.Params, URLData.HostName);
    n_send_data(s, DataBuffer, w);

    FILE *pFile = NULL;
    int r = 0, header = 0;

    time_t now = time(0);
    struct tm tstruct = { 0 };

    char TimeBuffer[64] = { 0 };
    localtime_s(&tstruct, &now);

    strftime(TimeBuffer, sizeof(TimeBuffer), "%d%m%Y_%H%M%S", &tstruct);
    sprintf_s(DataBuffer, "%s.mp3", TimeBuffer);

    fopen_s(&pFile, FileName ? (const char *) FileName : DataBuffer, "wb");

    while(true)
    {
        r = n_recv_data(s, DataBuffer, DATA_BUFFER_SIZE);

        if(!header)
        {
            char *ContentStart = strstr(DataBuffer, "\r\n\r\n");
            if(ContentStart) fwrite(ContentStart, r - (ContentStart - DataBuffer), 1, pFile);

            if(ContentStart) header = 1;
            continue;
        }
  
        if(r == SOCKET_ERROR_RECV || r == SOCKET_RECV_NO_DATA)
        {
            break;
        }
        else if(r == SOCKET_RECV_MORE_DATA)
        {
            fwrite(DataBuffer, DATA_BUFFER_SIZE, 1, pFile);
        }
        else
        {
            fwrite(DataBuffer, r, 1, pFile);
        }
    }

    fclose(pFile);
    n_close_socket(s);   
}