#ifndef SOUNDCLOUD_H
#define SOUNDCLOUD_H

typedef struct sc_track_location_t
{
    char Location[1024];
} sc_track_location_t;

typedef struct sc_track_info_t
{
    char Title[256];
    char StreamURL[256];

    char URI[256];
    char ArtWorkURL[256];
} sc_track_info_t;

typedef struct sc_strems_urls_t
{
    char hls_mp3_128_url[1024];
    char http_mp3_128_url[1024];

    char rtmp_mp3_128_url[1024];
    char preview_mp3_128_url[1024];
} sc_strems_urls_t;

void sc_get_trak_location(char *TrackURL, sc_track_location_t *OutData);
void sc_get_trak_info(char *TrackLocation, sc_track_info_t *OutData);

void sc_get_trak_streams(char *StreamLocation, sc_strems_urls_t *OutData);
void sc_download_track(char *StreamURL, char *FileName = 0);

#endif