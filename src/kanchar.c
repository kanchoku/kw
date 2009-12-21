
#include <windows.h>
#include <imm.h>

#include "kanchar.h"

UINT WM_KANCHOKU_CHAR=0;
UINT WM_KANCHOKU_UNICHAR=0;

HINSTANCE hInst;
HHOOK hMsgHook=0;
HWND hWnd;



int WINAPI DllMain(HINSTANCE hInstance, DWORD fdReason, PVOID pvReserved)
{
    hInst = hInstance;
    switch (fdReason) {
    case DLL_PROCESS_ATTACH:
        hInst = hInstance;
        WM_KANCHOKU_CHAR = RegisterWindowMessage("WM_KANCHOKU_CHAR");
        WM_KANCHOKU_UNICHAR = RegisterWindowMessage("WM_KANCHOKU_UNICHAR");
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


EXPORT BOOL IsHooking()
{
    if (hMsgHook)
        return TRUE;
    else
        return FALSE;
}

EXPORT HHOOK MySetHook()
{
    if (!hMsgHook) {
        hMsgHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)msgHookProc, hInst, NULL);
    }
    return hMsgHook;
}

EXPORT int MyEndHook()
{
    if (hMsgHook) {
        if (UnhookWindowsHookEx(hMsgHook)) hMsgHook = NULL;
    }
    return IsHooking();
}


BOOL CALLBACK FindWindowProc(HWND hw, LPARAM lp)
{
    HANDLE h = GetProp(hw, "KanchokuWin_KanCharDLL_NextHook");
    if (h) {
        hMsgHook = (HHOOK)h;
        return FALSE;
    }
    return TRUE;
}

EXPORT LRESULT CALLBACK msgHookProc(int nCode, WPARAM wp, LPARAM lp)
{
    MSG *pcwp;
    if (!hMsgHook) {
        EnumWindows(&FindWindowProc, NULL);
    }
    if (nCode == HC_ACTION && wp == PM_REMOVE) {
        pcwp = (MSG *)lp;
        if (pcwp->message == WM_KANCHOKU_CHAR || pcwp->message == WM_KANCHOKU_UNICHAR) {
            if (pcwp->lParam & 0xFF > 0) {
                //メッセージキューに並びなおす（WordでのBS暴走対策）
                //VK_BACKの時にWM_KEYDOWN/KEYUPをPostMessageするなら
                //それに組み合わせるにはlParam=1でPostMessage
                PostMessage(pcwp->hwnd, pcwp->message, pcwp->wParam, pcwp->lParam-1);
                pcwp->message = WM_NULL;
                return CallNextHookEx(hMsgHook, nCode, wp, lp);
            }
        }
        if (pcwp->message == WM_KANCHOKU_CHAR) {
            HWND activeWin = pcwp->hwnd;
            char mbc[4];
            HIMC hImc;
            int bIME;
            DWORD dwConv, dwSent;
            if ((pcwp->wParam >> 8) & 0xff) {
                mbc[0] = (pcwp->wParam >> 8) & 0xff;
                mbc[1] = pcwp->wParam & 0xff;
                mbc[2] = 0;
            } else {
                mbc[0] = pcwp->wParam & 0xff;
                mbc[1] = 0;
            }
            hImc = ImmGetContext(activeWin);
            if(hImc == NULL) {
              return(0);
            }
            bIME = ImmGetOpenStatus(hImc);
            if (!bIME) {
                ImmSetOpenStatus(hImc, TRUE);
            }
            ImmGetConversionStatus(hImc, &dwConv, &dwSent);
            /*if (0xa1 <= pcwp->wParam && pcwp->wParam <= 0xdf) {
                ImmSetConversionStatus(hImc, IME_CMODE_NATIVE | IME_CMODE_KATAKANA, IME_SMODE_NONE);
            } else if (0x829f <= pcwp->wParam && pcwp->wParam <= 0x82f1) {
                ImmSetConversionStatus(hImc, IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE, IME_SMODE_NONE);
            } else*/ if (0x8340 <= pcwp->wParam && pcwp->wParam <= 0x8396) {
                //全角カタカナはこうしないとひらがなに@MSIME2k
                ImmSetConversionStatus(hImc, IME_CMODE_NATIVE | IME_CMODE_KATAKANA | IME_CMODE_FULLSHAPE, IME_SMODE_NONE);
            } else {
                ImmSetConversionStatus(hImc, (((pcwp->wParam >> 8) & 0xff) ? IME_CMODE_FULLSHAPE : 0), IME_SMODE_NONE);
            }
            ImmSetCompositionString(hImc, SCS_SETSTR, (LPVOID)mbc, strlen(mbc), (LPVOID)mbc, strlen(mbc));
            ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
            ImmSetConversionStatus(hImc, dwConv, dwSent);
            if (!bIME) {
                ImmSetOpenStatus(hImc, FALSE);
            }
            ImmReleaseContext(activeWin, hImc);
            pcwp->message = WM_NULL;
        }
        if (pcwp->message == WM_KANCHOKU_UNICHAR) {
            HWND activeWin = pcwp->hwnd;
            WCHAR wc[4];
            HIMC hImc;
            int bIME;
            DWORD dwConv, dwSent;
            wc[0] = pcwp->wParam;
            wc[1] = 0;
            hImc = ImmGetContext(activeWin);
            if(hImc == NULL) {
              return(0);
            }
            bIME = ImmGetOpenStatus(hImc);
            if (!bIME) {
                ImmSetOpenStatus(hImc, TRUE);
            }
            ImmGetConversionStatus(hImc, &dwConv, &dwSent);
            {
                ImmSetConversionStatus(hImc, IME_CMODE_FULLSHAPE, IME_SMODE_NONE);
            }
            ImmSetCompositionStringW(hImc, SCS_SETSTR, (LPVOID)wc, wcslen(wc)*sizeof(WCHAR), (LPVOID)wc, wcslen(wc)*sizeof(WCHAR));
            ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
            ImmSetConversionStatus(hImc, dwConv, dwSent);
            if (!bIME) {
                ImmSetOpenStatus(hImc, FALSE);
            }
            ImmReleaseContext(activeWin, hImc);
            pcwp->message = WM_NULL;
        }
    }
    return CallNextHookEx(hMsgHook, nCode, wp, lp);
}


