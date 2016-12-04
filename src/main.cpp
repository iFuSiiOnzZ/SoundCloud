#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    n_init_network();
    char Buffer[1024] = { 0 };

    sc_track_location_t TrackLocation = { 0 };
    sc_strems_urls_t StreamsLocation = { 0 };
    sc_track_info_t TrackInfo = { 0 };

    printf("\n");
    
    {
        printf("    Getting track Location for  ...  ");
        sc_get_trak_location(argv[1], &TrackLocation);
        printf("%s\n", "done");

        if(TrackLocation.Location[0] == 0)
        {
            printf("        > %s\n", "No track found the given URL");
            return EXIT_SUCCESS;
        }
    }

    {
        printf("    Getting track info for      ...  ");
        sc_get_trak_info(TrackLocation.Location, &TrackInfo);
        printf("%s\n", "done");
    }

    {
        printf("    Getting stream location for ...  ");
        sc_get_trak_streams(TrackInfo.StreamURL, &StreamsLocation);
        printf("%s\n", "done");
    }

    {
        sprintf_s(Buffer, "%s.mp3", TrackInfo.Title);
        printf("    Downloading as %s ...", Buffer);

        sc_download_track(StreamsLocation.http_mp3_128_url, Buffer);
        printf("%s\n", "done");
    }

    n_close_network();
    return EXIT_SUCCESS;
}