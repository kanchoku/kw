#include "control_window.h"

// -------------------------------------------------------------------
// window procedure

int ControlWindow::wndProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    // window handle ‚â message ‚Ìˆø”‚ğæ‚Á‚Ä‚¨‚­
    hwnd = w;
    wParam = wp;
    lParam = lp;

    // message ‚Ìí—Ş‚Å•ªŠò
    switch (msg) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// -------------------------------------------------------------------

