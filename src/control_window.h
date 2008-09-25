#ifndef CONTROL_WINDOW_H
#define CONTROL_WINDOW_H
// -------------------------------------------------------------------

#include "table_window.h"

class ControlWindow {
 public:
    // TableWindow の instance
    TableWindow *tableWindow;

    // window procedure
    int wndProc(HWND, UINT, WPARAM, LPARAM);

    // メッセージの引数を格納
    HWND hwnd;
    WPARAM wParam;
    LPARAM lParam;

    // コンストラクタ
    ControlWindow(TableWindow *t) {
        tableWindow = t;
    }
};

// -------------------------------------------------------------------
#endif // CONTROL_WINDOW_H

