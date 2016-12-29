#include "Window.h"
#include "../../common/soundcloud.h"

#include <commctrl.h>
#include <Objbase.h>
#include <Shlobj.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"comctl32.lib")

#define WINDOW_NAME "Soundcloud Downloader"
#define CLASS_NAME  "SOUNDLOUD_DOWNLOADER"

#define WINDOW_W    640
#define WINDOW_H    260

#define IDC_BUTTON_DOWNLOAD_ID  1000
#define IDC_BUTTON_FILEPATH_ID  1001

#define IDC_EDIT_URL_ID     2000
#define IDC_EDIT_PATH_ID    2001

#define IDC_PROGRESS_ID 3000

static void set_progress_size(int DataSize, void *UserData)
{
    SendMessage((HWND) UserData, PBM_SETRANGE, 0, MAKELPARAM(0, DataSize));
}

static void set_progress_progress(int DownloadedSize, void *UserData)
{
    SendMessage((HWND) UserData, PBM_SETPOS, DownloadedSize, 0);
}

static int download_track(HWND hWnd)
{
    sc_track_location_t TrackLocation = { 0 };
    sc_strems_urls_t StreamsLocation = { 0 };
    sc_track_info_t TrackInfo = { 0 };

    char Buffer[1024] = { 0 };
    char SavePath[1024] = { 0 };

    GetWindowTextA(GetDlgItem(hWnd, IDC_EDIT_URL_ID), Buffer, 1024);
    GetWindowTextA(GetDlgItem(hWnd, IDC_EDIT_PATH_ID), SavePath, 1024);

    if(!Buffer[0])
    {
        MessageBoxA(hWnd, "Input can not be empty", "", MB_OK);
        return 0;
    }

    sc_get_track_location(Buffer, &TrackLocation);
    if(!TrackLocation.Location[0])
    {
        MessageBoxA(hWnd, "No track found the given URL", "", MB_OK);
        return 0;
    }

    sc_get_track_info(TrackLocation.Location, &TrackInfo);
    sc_get_track_streams(TrackInfo.StreamURL, &StreamsLocation);

    sc_stream_process_t CallBaks =
    {
        set_progress_size,
        set_progress_progress,
        GetDlgItem(hWnd, IDC_PROGRESS_ID)
    };

    sprintf_s(Buffer, "%s.mp3", TrackInfo.Title);
    sc_download_track(StreamsLocation.http_mp3_128_url, strlen(SavePath) ? SavePath : NULL, Buffer, &CallBaks);

    return 1;
}

static int CALLBACK BrowseFolderCallback(HWND hWnd, UINT hMsg, LPARAM lParam, LPARAM lpData)
{
    if (hMsg == BFFM_INITIALIZED)
    {
        LPCTSTR path = (LPCTSTR) lpData;
        SendMessage(hWnd, BFFM_SETSELECTION, true, (LPARAM) path);
    }

    return 0;
}

static int CALLBACK SetFont(HWND child, LPARAM font)
{
    SendMessage(child, WM_SETFONT, font, true);
    return 1;
}


static LRESULT CALLBACK WindowCallback(HWND hWnd, UINT hMsg, WPARAM wParam, LPARAM lParam)
{
    if(hMsg == WM_CREATE)
    {
        INITCOMMONCONTROLSEX CommonControls = { 0 };

        CommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
        CommonControls.dwICC  = ICC_PROGRESS_CLASS;

        ShowWindow(GetConsoleWindow(), SW_HIDE);
        InitCommonControlsEx(&CommonControls);

        HWND hEditDownload = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 70, 495, 25, hWnd, (HMENU)IDC_EDIT_URL_ID, NULL, NULL);
        CreateWindowExA(0, "BUTTON", "Download", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 510, 70, 100, 25, hWnd, (HMENU)IDC_BUTTON_DOWNLOAD_ID, NULL, NULL);

        HWND hEditSavePath = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 100, 495, 25, hWnd, (HMENU)IDC_EDIT_PATH_ID, NULL, NULL);
        CreateWindowExA(0, "BUTTON", "...", WS_TABSTOP | WS_VISIBLE | WS_CHILD, 510, 100, 100, 25, hWnd, (HMENU)IDC_BUTTON_FILEPATH_ID, NULL, NULL);

        CreateWindowExA(0, PROGRESS_CLASS, (LPTSTR) NULL,  WS_CHILD | WS_VISIBLE, 10,  130, 600, 25, hWnd, (HMENU) IDC_PROGRESS_ID, NULL, NULL);
        EnumChildWindows(hWnd, (WNDENUMPROC)SetFont, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));

        wchar_t CurrentPath[MAX_PATH] = { 0 };
        int i = GetModuleFileNameW(NULL, CurrentPath, MAX_PATH);
        while(i && CurrentPath[i] != '\\') CurrentPath[i--] = 0;

        SendMessage(hEditDownload, EM_SETCUEBANNER, TRUE, (LPARAM) L"https://soundcloud.com/thefatrat/thefatrat-monody-feat-laura-brehm-1");
        SendMessage(hEditSavePath, EM_SETCUEBANNER, TRUE, (LPARAM) CurrentPath);
        
        return 0;
    }
    else if(hMsg == WM_CLOSE)
    {
        DestroyWindow(hWnd);
        
        return 0;
    }
    else if(hMsg == WM_DESTROY)
    {
        ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);
        PostQuitMessage(EXIT_SUCCESS);
        
        return 0;
    }
    else if((hMsg == WM_KEYDOWN) && (wParam == VK_ESCAPE))
    {
        ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);
        PostQuitMessage(EXIT_SUCCESS);
        
        return 0;
    }
    else if(hMsg == WM_COMMAND && LOWORD(wParam) == IDC_BUTTON_DOWNLOAD_ID)
    {
        int Ok = download_track(hWnd);
        if(Ok) MessageBoxA(hWnd, "Download complete", "", MB_OK);

        return 0;
    }
    else if(hMsg == WM_COMMAND && LOWORD(wParam) == IDC_BUTTON_FILEPATH_ID)
    {
        CoInitializeEx(0, COINIT_APARTMENTTHREADED);
        BROWSEINFOA bi = { 0 };
        
        bi.lpszTitle = "Browse for folder...";
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.lpfn =  BrowseFolderCallback;

        LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

        if(pidl)
        {
            LPMALLOC lpMalloc = NULL;
            char CurrentPath[512] = { 0 };

            SHGetPathFromIDListA(pidl, CurrentPath);
            SetWindowTextA(GetDlgItem(hWnd, IDC_EDIT_PATH_ID), CurrentPath);
            
            SHGetMalloc(&lpMalloc);
            lpMalloc->Release();
        }

        CoUninitialize();
        return 0;
    }

    return DefWindowProcA(hWnd, hMsg, wParam, lParam);
}

int win32_open_window()
{
    WNDCLASSEXA wndCls;     memset(&wndCls, 0, sizeof(WNDCLASSEX));
    HWND        hWnd;       memset(&hWnd, 0, sizeof(HWND));
    MSG         hMsg;       memset(&hMsg, 0, sizeof(MSG));

    wndCls.cbSize           = sizeof(WNDCLASSEX);
    wndCls.hInstance        = GetModuleHandle(NULL);

    wndCls.lpfnWndProc      = WindowCallback;
    wndCls.lpszClassName    = CLASS_NAME;

    wndCls.style			= 0;
    wndCls.hbrBackground	= (HBRUSH) (COLOR_WINDOW + 1);

    if(!RegisterClassExA(&wndCls))
    {
        MessageBox(NULL, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    int ScreenX = GetSystemMetrics(SM_CXSCREEN);
    int ScreenY = GetSystemMetrics(SM_CYSCREEN);

    int StartX = ScreenX / 2 - WINDOW_W / 2;
    int StartY = ScreenY / 2 - WINDOW_H / 2;

    hWnd = CreateWindowExA
    (
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
        StartX, StartY, WINDOW_W, WINDOW_H, NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if(!hWnd)
    {
        MessageBox(NULL, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    while(GetMessage(&hMsg, NULL, 0U, 0U) > 0)
    {
        TranslateMessage(&hMsg);
        DispatchMessageA(&hMsg);
    }

    return 0;
}