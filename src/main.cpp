
typedef struct args_t
{
    char *SongURL;
    char *SavePath;
} args_t;

static void CreateFolder(char *pFolder)
{
    if (!pFolder) return;

    char *pFound = strchr(pFolder, '/');
    char pNextFolder[MAX_PATH] = { 0 };

    while(pFound)
    {
        strncpy_s(pNextFolder, pFolder, pFound - pFolder);
        CreateDirectoryA(pNextFolder, NULL);
        pFound = strchr(pFound + 1, '/');
    }

    CreateDirectoryA(pFolder, NULL);
}

static void ToUnixFilePath(char *pFilePath)
{
    if(!pFilePath) return;
    size_t sz = strlen(pFilePath);

    for(size_t i = 0; i < sz; ++i)
    {
        if(pFilePath[i] == '\\')
        {
            pFilePath[i] = '/';
        }
    }
}

static int ParseArgs(int argc, char *argv[], args_t *OutData)
{
    int bSc = 0;
    int bSp = 0;

    for(int i = 1; i < argc; ++i)
    {
        if(!my_strcmp(argv[i], "-s"))
        {
            OutData->SongURL = argv[i + 1];
            bSc = 1; ++i;
        }

        if(!my_strcmp(argv[i], "-p"))
        {
            OutData->SavePath = argv[i + 1];
            bSp = 1; ++i;
        }
    }

    return bSc;
}

static void Help()
{
    printf("\n");
    printf("    ProgramName.exe -s song_url [-p save_path]\n");
    printf("        > -s soundcloud song url\n");
    printf("        > -p path to save the mp3\n");

    printf("\n");
    printf("        NOTE: Parameters inside [...] are optional\n");
    printf("        EXAMPLE: SoundCloud.exe -s https://soundcloud.com/thefatrat/thefatrat-monody-feat-laura-brehm-1\n");
}


int main(int argc, char *argv[])
{
    args_t Args = { 0 };
    int r = ParseArgs(argc, argv, &Args);

    if(!r)
    {
        Help();
        return EXIT_SUCCESS;
    }

    n_init_network();
    char Buffer[1024] = { 0 };

    ToUnixFilePath(Args.SavePath);
    CreateFolder(Args.SavePath);

    sc_track_location_t TrackLocation = { 0 };
    sc_strems_urls_t StreamsLocation = { 0 };
    sc_track_info_t TrackInfo = { 0 };

    printf("\n");
    
    {
        printf("    Getting track Location for  ...  ");
        sc_get_track_location(Args.SongURL, &TrackLocation);
        printf("%s\n", "done");

        if(TrackLocation.Location[0] == 0)
        {
            printf("        > %s\n", "No track found the given URL");
            return EXIT_SUCCESS;
        }
    }

    {
        printf("    Getting track info for      ...  ");
        sc_get_track_info(TrackLocation.Location, &TrackInfo);
        printf("%s\n", "done");
    }

    {
        printf("    Getting stream location     ...  ");
        sc_get_track_streams(TrackInfo.StreamURL, &StreamsLocation);
        printf("%s\n", "done");
    }

    {
        sprintf_s(Buffer, "%s.mp3", TrackInfo.Title);
        printf("    Downloading as %s ...", Buffer);

        sc_download_track(StreamsLocation.http_mp3_128_url, Args.SavePath, Buffer);
        printf("%s\n", "done");
    }

    n_close_network();
    return EXIT_SUCCESS;
}