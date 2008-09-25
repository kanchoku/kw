// 入力フォーカスを持つウィンドウのクラス名を調べるためのプログラム
#include <windows.h>

#define ACTIVE_KEY 1
#define HOTKEY 0xdc // <C-\>
#define BUFSIZ 1024

char *appName = "wininfo";
HINSTANCE hInstance;
HWND hWndEdit;

int handleCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    hWndEdit = CreateWindow("EDIT",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY, 
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            hwnd,
            0,
            hInstance,
            NULL);
    SetWindowText(hWndEdit, "<CTRL+\\>キーで,入力フォーカスを持つWindowのClass名を取得");

    RegisterHotKey(hwnd, ACTIVE_KEY, MOD_CONTROL, HOTKEY);
    return 0;
}

int handleDestroy(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    UnregisterHotKey(hwnd, ACTIVE_KEY);
    PostQuitMessage(0);
    return 0;
}

int handleHotKey(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    char str[BUFSIZ];

    // 入力フォーカスを持つウィンドウを取得
    HWND targetWin = GetForegroundWindow();
    DWORD targetThread = GetWindowThreadProcessId(targetWin, NULL);
    DWORD selfThread = GetCurrentThreadId();
    AttachThreadInput(selfThread, targetThread, TRUE);
    HWND activeWin = GetFocus();

    if (GetClassName(activeWin, str, sizeof(str)) > 0) {
        SetWindowText(hWndEdit, str);
        SendMessage(hWndEdit, EM_SETSEL, 0, -1); // select all text
    } else {
        SetWindowText(hWndEdit, "GetClassName()が失敗");
    }
    AttachThreadInput(selfThread, targetThread, FALSE); // スレッドを切り離す
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        return handleCreate(hwnd, wParam, lParam);

    case WM_SETFOCUS:
        SetFocus(hWndEdit);
        return 0;

    case WM_SIZE:
        MoveWindow(hWndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;

    case WM_DESTROY:
        return handleDestroy(hwnd, wParam, lParam);

    case WM_HOTKEY:
        return handleHotKey(hwnd, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance,
                   PSTR cmdLine, int cmdShow)
{
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;

    hInstance = instance;
    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = appName;
    wndclass.hIconSm = NULL;
    RegisterClassEx(&wndclass);

    hwnd = CreateWindow(appName,
                          "wininfo",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          600, 60,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
