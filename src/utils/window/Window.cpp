#include "Window.h"
#include <commctrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"comctl32.lib")

#define WINDOW_NAME "Soundcloud Downloader"
#define CLASS_NAME  "SOUNDLOUD_DOWNLOADER"

#define WINDOW_W    640
#define WINDOW_H    260

#define IDC_BUTTON_ID   1000
#define IDC_EDIT_ID     2000

static LRESULT CALLBACK window_procedure(HWND hWnd, UINT hMsg, WPARAM wParam, LPARAM lParam)
{
    if(hMsg == WM_CREATE)
    {
        INITCOMMONCONTROLSEX CommonControls = { 0 };
        CommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
        CommonControls.dwICC  = ICC_PROGRESS_CLASS;

        ShowWindow(GetConsoleWindow(), SW_HIDE);
        InitCommonControlsEx(&CommonControls);

        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL, 10, 100, 495, 25, hWnd, (HMENU)IDC_EDIT_ID, NULL, NULL);
        CreateWindowExA(0, "BUTTON", "Download", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 510, 100, 100, 25, hWnd, (HMENU)IDC_BUTTON_ID, NULL, NULL);
        CreateWindowExA(0, PROGRESS_CLASS, (LPTSTR) NULL,  WS_CHILD | WS_VISIBLE, 10,  130, 600, 25, hWnd, (HMENU) 0, NULL, NULL);

    }
    else if(hMsg == WM_CLOSE)
    {
        DestroyWindow(hWnd);
    }
    else if(hMsg == WM_DESTROY)
    {
        ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);
        PostQuitMessage(EXIT_SUCCESS);
    }
    else if((hMsg == WM_KEYDOWN) && (wParam == VK_ESCAPE))
    {
        ShowWindow(GetConsoleWindow(), SW_SHOWNORMAL);
        PostQuitMessage(EXIT_SUCCESS);
    }

    return DefWindowProcA(hWnd, hMsg, wParam, lParam);
}

int open_window()
{
    WNDCLASSEXA wndCls;     memset(&wndCls, 0, sizeof(WNDCLASSEX));
    HWND        hWnd;       memset(&hWnd, 0, sizeof(HWND));
    MSG         hMsg;       memset(&hMsg, 0, sizeof(MSG));

    wndCls.cbSize           = sizeof(WNDCLASSEX);
    wndCls.hInstance        = GetModuleHandle(NULL);

    wndCls.lpfnWndProc      = window_procedure;
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
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW,
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