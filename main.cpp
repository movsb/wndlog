#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>

#define IDC_EDIT 101
static UINT WM_SHELLHOOKMESSAGE;    //自定义消息
static HWND hwndMain;
static HWND hwndEdit;
static struct {
    UINT uMsg;
    char* pMsg;
}shell_list[] =
{
    {HSHELL_GETMINRECT, "HSHELL_GETMINRECT"},
    {HSHELL_WINDOWACTIVATED, "HSHELL_WINDOWACTIVATED"},
    {HSHELL_RUDEAPPACTIVATED, "HSHELL_RUDEAPPACTIVATED"},
    {HSHELL_WINDOWREPLACING, "HSHELL_WINDOWREPLACING"},
    {HSHELL_WINDOWREPLACED, "HSHELL_WINDOWREPLACED"},
    {HSHELL_WINDOWCREATED, "HSHELL_WINDOWCREATED"},
    {HSHELL_WINDOWDESTROYED, "HSHELL_WINDOWDESTROYED"},
    {HSHELL_ACTIVATESHELLWINDOW, "HSHELL_ACTIVATESHELLWINDOW"},
    {HSHELL_TASKMAN, "HSHELL_TASKMAN"},
    {HSHELL_REDRAW, "HSHELL_REDRAW"},
    {HSHELL_FLASH, "HSHELL_FLASH"},
    {HSHELL_ENDTASK, "HSHELL_ENDTASK"},
    {HSHELL_APPCOMMAND, "HSHELL_APPCOMMAND"},
    {0, NULL}
};


BOOL SetShellHook(HWND hwndHook)    //需要安装钩子的窗口句柄
{
    WM_SHELLHOOKMESSAGE = RegisterWindowMessage("SHELLHOOK");
    return WM_SHELLHOOKMESSAGE && RegisterShellHookWindow(hwndHook);
}

BOOL UnSetShellHook(HWND hwndHook)  //卸载钩子
{
    return DeregisterShellHookWindow(hwndHook);
}

void AddEditText(char* szMsg, HWND hwndNew) //更新文本框内容
{
    int len;
    char buffer[1024];
    char szCls[128] = {0}, szText[128] = {0};
    GetWindowText(hwndNew, szText, sizeof(szText));
    GetClassName(hwndNew, szCls, sizeof(szCls));
    len = GetWindowTextLength(hwndEdit);
    _snprintf(buffer, sizeof(buffer), "Message:\t%s\r\nClassName:\t%s\r\nWindowName:\t%s\r\n\r\n", szMsg, szCls, szText);
    SendMessage(hwndEdit, EM_SETSEL, len, len);
    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)buffer);
    return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if(uMsg == WM_SHELLHOOKMESSAGE) {
        int i;
        for(i = 0; shell_list[i].pMsg; i++) {
            if(shell_list[i].uMsg == wParam) {
                AddEditText(shell_list[i].pMsg, (HWND)lParam);
                return 0;
            }
        }
        return 0;
    }
    switch(uMsg) {
    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        MoveWindow(hwndEdit, 0, 0, rect.right - rect.left, rect.bottom - rect.top, TRUE);
        return 0;
    }
    case  WM_CREATE:
    {
        RECT rect;
        hwndMain = hwnd;
        GetClientRect(hwnd, &rect);
        hwndEdit = CreateWindowEx(0, WC_EDIT, NULL,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_HSCROLL | WS_VSCROLL,
            0, 0, rect.right - rect.left, rect.bottom - rect.top,
            hwnd, (HMENU)IDC_EDIT, ((CREATESTRUCT*)lParam)->hInstance, NULL);

        if(!SetShellHook(hwnd)) {
            MessageBox(hwnd, "SetShellHook, failed.", NULL, MB_OK);
        }

        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        UnSetShellHook(hwnd);
        DestroyWindow(hwnd);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND hWnd;
    MSG msg;
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = "shell_wnd_class";
    wc.style = CS_HREDRAW | CS_VREDRAW;

    if(!RegisterClassEx(&wc))
        return 1;

    hWnd = CreateWindowEx(WS_EX_TOPMOST, "shell_wnd_class", "Shell Hook Demo", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 460, NULL, NULL, hInstance, NULL);

    if(!hWnd) {
        MessageBox(NULL, "CreateWindowEx Failed!", NULL, MB_OK);
        return 1;
    }

    UpdateWindow(hWnd);
    ShowWindow(hWnd, nCmdShow);

    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
