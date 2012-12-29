#ifndef TABLE_WINDOW_H
#define TABLE_WINDOW_H
// -------------------------------------------------------------------
// class TableWindow
// ���C���̃E�B���h�E�֘A�̏���

#include <iostream>
#include <fstream>
#include <windows.h>
#include <imm.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "block.h"
#include "bushu_dic.h"
#include "mg_table.h"
#include "moji.h"
//<v127a - gg>
#include "gg_dic.h"
//</v127a - gg>
#include "st_table.h"
#include "parser.h"
#include "tcode.h"
#include "version.h"

using namespace std;

#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC (0)
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif
#ifndef KEYEVENTF_UNICODE
#define KEYEVENTF_UNICODE     0x0004
#endif
typedef struct mytagGUITHREADINFO
{
    DWORD   cbSize;
    DWORD   flags;
    HWND    hwndActive;
    HWND    hwndFocus;
    HWND    hwndCapture;
    HWND    hwndMenuOwner;
    HWND    hwndMoveSize;
    HWND    hwndCaret;
    RECT    rcCaret;
} MYGUITHREADINFO, *PMYGUITHREADINFO, FAR * LPMYGUITHREADINFO;
struct MYHWINEVENTHOOK__ { int unused; };
typedef struct MYHWINEVENTHOOK__ *MYHWINEVENTHOOK;
typedef VOID (CALLBACK* MYWINEVENTPROC)(
    MYHWINEVENTHOOK hWinEventHook,
    DWORD         event,
    HWND          hwnd,
    LONG          idObject,
    LONG          idChild,
    DWORD         idEventThread,
    DWORD         dwmsEventTime);
#ifndef EVENT_SYSTEM_FOREGROUND
#define EVENT_SYSTEM_FOREGROUND         0x0003
#endif
#ifndef WINEVENT_OUTOFCONTEXT
#define WINEVENT_OUTOFCONTEXT   0x0000
#endif

/* -------------------------------------------------------------------
 * ���b�Z�[�W
 */
#define KANCHOKU_ICONCLK 0x1001

#define ID_MYTIMER 32767

/* -------------------------------------------------------------------
 * ����肷��L�[
 *
 * �L�[�ԍ� 0 �` 97 (== TC_NKEYS * 2 - 1) ���������͂ɗp����L�[�B
 * 0x100�ȍ~���@�\�L�[�ƂȂ��Ă���B
 * �Փ˂��Ȃ��悤�ɋC�����邱�ƁB
 */
#define ACTIVE_KEY (0x100 + 1) // ON/OFF �̐؂�ւ��L�[
#define ACTIVE2_KEY (0x100 + 2) // ON/OFF �̐؂�ւ��L�[�A����2
#define INACTIVE_KEY (0x100 + 3) // OFF �ւ̐؂�ւ��L�[
#define INACTIVE2_KEY (0x100 + 4) // OFF �ւ̐؂�ւ��L�[�A����2
#define ACTIVEIME_KEY (0x100 + 6) // IME�A���ɂ�� ON/OFF �̐؂�ւ� (����Win���])

#define ESC_KEY    (0x100 + 11) // ESC
#define CG_KEY     (0x100 + 12) // C-g

#define BS_KEY     (0x100 + 21) // BS
#define CH_KEY     (0x100 + 22) // C-h

#define RET_KEY    (0x100 + 31) // RET
#define CM_KEY     (0x100 + 32) // C-m
#define CJ_KEY     (0x100 + 33) // C-j

#define TAB_KEY    (0x100 + 41) // TAB
#define CI_KEY     (0x100 + 42) // C-i

#define LT_KEY     (0x100 + 51) // "<"
#define GT_KEY     (0x100 + 52) // ">"


/* -------------------------------------------------------------------
 * ���z���� (1) - �t�H���g�ƃE�B���h�E�̑傫��
 *
 *   MARGIN                   ���� BLOCK
 * �F���d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d ��
 * �F�����������������������������������������������F ��
 * �F��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ���F ��
 * �F����������������������  �����������������������F ��
 * �F��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ���F ��
 * �F����������������������  �����������������������F ��
 * �F��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ���F �� HEIGHT
 * �F����������������������  �����������������������F ��
 * �F��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ���F ��
 * �F�����������������������������������������������F ��
 * �F  ��  ��  ��  ��  ��  ��  ��  ��  ��  ��  ��  �F ��
 * �F  ������������������������������������������  �F ��
 * �F�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�d�F ��
 * ���������������������� WIDTH  ��������������������
 */
#define CHAR_SIZE (styleFontSize)          // �����̑傫��
#define LARGE_CHAR_SIZE (CHAR_SIZE+stylePadding*2)    // �傫�������̑傫��
#define BLOCK_SIZE (CHAR_SIZE+stylePadding*3)         // ���z���Ղ̃L�[�̑傫��
#define MARGIN_SIZE (4)         // ���z���Ղ̓V�n���E�̗]��
#define WIDTH  (MARGIN_SIZE * 2 + BLOCK_SIZE * 11 + 1)  // ���z���Ղ̉���
#define HEIGHT (MARGIN_SIZE * 2 + BLOCK_SIZE * 5 + 1)   // ���z���Ղ̏c��
#define TRUNC_MARK_SIZE (BLOCK_SIZE/3+3)
#define SHIFT_MARK_SIZE (BLOCK_SIZE/5+3)

/* -------------------------------------------------------------------
 * ���z���� (2) - �F
 */
//<v127c>
// [�A�K�X��2:522] 256 �F���ł̖��
// !!! see also table_window.c--handleCreate()--palent
//#define GRAYTONE(x) RGB((x), (x), (x))
#define GRAYTONE(x) PALETTERGB((x), (x), (x))
//</v127c>
// ���[�h ON ���̉��z����
#define COL_ON_LN styleCol[3] // �g
#define COL_ON_K1 styleCol[4] // �L�[
#define COL_ON_K2 styleCol[5] // �L�[ (�e)
#define COL_ON_K3 styleCol[6] // �L�[ (�n�C���C�g)
#define COL_ON_M1 styleCol[7] // �]��
#define COL_ON_M2 styleCol[8] // �]�� (�e)
#define COL_ON_M3 styleCol[9] // �]�� (�n�C���C�g)
// ���[�h OFF ���̉��z����
#define COL_OFF_LN styleCol[0] // �g
#define COL_OFF_K1 styleCol[1] // �L�[
#define COL_OFF_M1 styleCol[2] // �]��
//<v127c>
// �����F�Ɣw�i�F
//#define COL_BLACK       (RGB(0x00, 0x00, 0x00)) // ��
//#define COL_WHITE       (RGB(0xff, 0xff, 0xff)) // ��
//#define COL_GRAY        (RGB(0x80, 0x80, 0x80)) // �D
//#define COL_LT_GRAY     (RGB(0xc0, 0xc0, 0xc0)) // ���D
//#define COL_LT_RED      (RGB(0xff, 0xc0, 0xc0)) // ����
//#define COL_LT_GREEN    (RGB(0xc0, 0xff, 0xc0)) // ����
//#define COL_LT_BLUE     (RGB(0xc0, 0xc0, 0xff)) // ����
//#define COL_LT_YELLOW   (RGB(0xff, 0xff, 0xc0)) // ����
//#define COL_LT_CYAN     (RGB(0xc0, 0xff, 0xff)) // ����
//#define COL_RED         (RGB(0xff, 0x00, 0x00)) // ��
//#define COL_DK_CYAN     (RGB(0x00, 0x80, 0x80)) // �[��
////<v127a - gg>
//#define COL_DK_MAGENTA  (RGB(0x80, 0x00, 0x80)) // �[��
////</v127a - gg>
//#define COLORDEF(r, g, b) RGB((r), (g), (b))
#define COLORDEF(r, g, b) PALETTERGB((r), (g), (b))
#define COL_BLACK       styleCol[18] // ��
#define COL_WHITE       (COLORDEF(0xff, 0xff, 0xff)) // ��
#define COL_GRAY        (COLORDEF(0x80, 0x80, 0x80)) // �D
#define COL_LT_GRAY     styleCol[13] // ���D
#define COL_LT_RED      styleCol[10] // ����
#define COL_LT_GREEN    styleCol[11] // ����
#define COL_LT_BLUE     styleCol[14] // ����
#define COL_LT_YELLOW   styleCol[12] // ����
#define COL_LT_CYAN     styleCol[15] // ����
#define COL_RED         styleCol[19] // ��
#define COL_DK_CYAN     styleCol[16] // �[��
#define COL_DK_MAGENTA  styleCol[17] // �[��
// !!! see also table_window.c--handleCreate()--palent
//</v127c>

/* -------------------------------------------------------------------
 * TableWindow �N���X
 */
class TableWindow {
private:
    // T-Code �ϊ���
    TCode *tc;

    // �C���X�^���X�n���h��
    HINSTANCE instance;
    // �t�H���g
    HFONT hFont;
    HFONT hLFont;
    //<v127c>
    // �p���b�g
    HPALETTE hPalette;
    //</v127c>
    // ���b�Z�[�W�̈������i�[
    HWND hwnd;
    WPARAM wParam;
    WPARAM lParam;

    // �^�X�N�g���C�ɓo�^
    NOTIFYICONDATA nid;

    // WM_KANCHOKU_CHAR�֘A
    HINSTANCE hKanCharDLL;
    void (*lpfnMySetHook)(HHOOK *, HHOOK *);
    int (*lpfnMyEndHook)(void);
    HHOOK hNextMsgHook, hNextCWPHook;
    UINT WM_KANCHOKU_CHAR;
    UINT WM_KANCHOKU_UNICHAR;
    UINT WM_KANCHOKU_NOTIFYVKPROCESSKEY;
    UINT WM_KANCHOKU_NOTIFYIMESTATUS;
    UINT WM_KANCHOKU_SETIMESTATUS;

    HWND hwNewTarget;
    int inSetFocus;
    int bKeepBuffer;
    int bGlobalHotKey;

    enum HOTKEYMODE { OFF, NORMAL, EDITCLAUSE };
    HOTKEYMODE hotKeyMode;
    // �ꊲ�L�΂��k�ߗp�L�[
    // T-Code�L�[�Ƃ��Ԃ��Ă���ꍇ�̂�LT_KEY�AGT_KEY�̑���Ɏg�p
    int tc_lt_key;
    int tc_gt_key;

    // Shift�������o�i���z���Օ\���p�j
    bool isShift;
    bool isShiftPrev;
    // Timeout����p
    int deciSecAfterStroke;

    // �X�^�C���ݒ�
    COLORREF styleCol[20];
    char styleFontName[LF_FACESIZE];
    int styleFontSize;
    int stylePadding;

    // API�̖����I�����N
    HINSTANCE hUser32;
    BOOL (WINAPI *myGetGUIThreadInfo)(DWORD, PMYGUITHREADINFO);
    MYHWINEVENTHOOK (WINAPI *mySetWinEventHook)(DWORD, DWORD, HMODULE, MYWINEVENTPROC, DWORD, DWORD, DWORD);
    BOOL (WINAPI *myUnhookWinEvent)(MYHWINEVENTHOOK);

    // WinEvent
    MYHWINEVENTHOOK hEventHook;
    static VOID CALLBACK WinEventProc(MYHWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
    typedef struct { MYHWINEVENTHOOK h; void *p; } WEH;
    static vector<WEH> weh_map;
public:
    // �R���X�g���N�^
    TableWindow(HINSTANCE i);
    // �f�X�g���N�^
    ~TableWindow();
    // window procedure
    int wndProc(HWND, UINT, WPARAM, LPARAM);

private:
    // �N��/�ҋ@
    void activate();            // �N�� (HotKey �̊��t)
    void inactivate();          // �ҋ@ (HotKey �̉��)
    //void setSpecialHotKey();    // ���� HotKey �̊m�ۂƉ��
    void setMazeHotKey(int);
    void disableHotKey();
    void resumeHotKey();
    void disableGlobalHotKey();
    void resumeGlobalHotKey();
    void setTitleText();        // �^�C�g���o�[�̕�����ݒ�

    // ���b�Z�[�W�n���h��
    int handleCreate();         // WM_CREATE
    int handleDestroy();        // WM_DESTROY
    int handleLButtonDown();    // WM_LBUTTONDOWN
    int handlePaint();          // WM_PAINT
    int handleTimer();          // WM_TIMER
    int handleNotifyVKPROCESSKEY(); // WM_KANCHOKU_NOTIFYVKPROCESSKEY
    int handleNotifyIMEStatus();    // WM_KANCHOKU_NOTIFYIMESTATUS
    int handleForeground(HWND);     // EVENT_SYSTEM_FOREGROUND
    int handleHotKey();         // WM_HOTKEY

    // T-Code �֘A
    void initTC();              // T-Code �ϊ���̏�����
    void output();              // T-Code �ϊ����ʂ̏o��

    // ���z���Ղ̕`��
    void drawFrameOFF(HDC hdc); // OFF ���̉��z���Ղ̘g
    void drawFrame50(HDC hdc);  // �ʏ펞�̉��z���Ղ̘g
    void drawFrame10(HDC hdc);  // �������I�����̉��z���Ղ̘g
    void drawVKB50(HDC hdc, bool isWithBothSide = false);        // �ʏ펞�̉��z����
    void drawVKB10(HDC);        // �������I�����̉��z����
    void drawMiniBuffer(HDC, int, COLORREF, MojiBuffer *);
                                // �~�j�o�b�t�@

    // �G���[����
    void error(char *);         // �G���[��\�����A�I��
    void warn(char *);          // �x����\�����邪�A�p��

    void readTargetWindowSetting(char *); // �o�͐�E�B���h�E���Ƃ̐ݒ�̓Ǎ�
    int getOutputMethod(HWND);  // �w�肵���E�B���h�E�ɑ΂���outputMethod���擾

    void makeStyle();
    void readStyleSetting();
};

// -------------------------------------------------------------------
#endif // TABLE_WINDOW_H
