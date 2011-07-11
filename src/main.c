#include <windows.h>
#include <string.h>

#include "control_window.h"
#include "table_window.h"

TableWindow *tableWindow;
std::vector<TableWindow::WEH> TableWindow::weh_map;
ControlWindow *controlWindow;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//BOOL CALLBACK CtlProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance,
                   PSTR cmdLine, int cmdShow) {
    static char *appName = "kanchoku";

    // ウィンドウのスタイルを登録
    WNDCLASSEX wndclass;
    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = instance;
    wndclass.hIcon = LoadIcon(instance, "kanchoku");
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = appName;
    ////wndclass.hIconSm = LoadIcon(instance, "kanmini");
    wndclass.hIconSm = LoadIcon(instance, "kanmini0");
                                // 小アイコン
                                // ON/OFF で変更する予定
                                // →変更した
    RegisterClassEx(&wndclass);

    // メインのウィンドウを処理するクラス
    tableWindow = new TableWindow(instance);

    // ダイアログボックスを処理するクラス
    controlWindow = new ControlWindow(tableWindow);

    // ウィンドウを作る
    HWND hwnd;
    hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                          appName,
                          "漢直窓",
                          WS_DLGFRAME | WS_SYSMENU,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          176, 100,
                          NULL,
                          NULL,
                          instance,
                          NULL);

    // ウィンドウを表示
    //<hideOnStartup>
    //ShowWindow(hwnd, cmdShow);
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    //</hideOnStartup>
    UpdateWindow(hwnd);
    //<hideOnStartup>
    // ショートカットのプロパティの実行時の大きさで最小化が指定されてる場合
    if (cmdShow == SW_SHOWMINNOACTIVE)
        ShowWindow(hwnd, SW_HIDE);
    //</hideOnStartup>

    // メッセージループ
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 終了
    return msg.wParam;
}

// window procedure
// 単に table window に処理を回すだけ
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return tableWindow->wndProc(hwnd, msg, wParam, lParam);
}

// window procedure - ダイアログ
// 単に dialog window に処理を回すだけ
//BOOL CALLBACK CtlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//    return controlWindow->wndProc(hwnd, msg, wParam, lParam);
//}

// -------------------------------------------------------------------
