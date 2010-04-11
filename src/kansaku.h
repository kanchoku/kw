#ifndef KANSAKU_H
#define KANSAKU_H
// -------------------------------------------------------------------

#include <windows.h>
#include <vector>

#include "block.h"
#include "moji.h"
#include "parser.h"
#include "st_table.h"
#include "tc.h"
#include "debug.h"

/* -------------------------------------------------------------------
 *
 * normal style
 * ------------
 *     CHAR    VKB
 * ：‥│‥‥‥│‥‥‥‥‥‥‥‥‥‥‥：←ENT
 * ：┌│───↓──────────┐：
 * ：│↓      □□□□□  □□□□□│←─FRAME
 * ：│┌─┐  ■■■■□  □■■■■│：
 * ：││字│  ■■■■□  □■■■■│：
 * ：│└─┘  ■■■■□  □■■■■│：
 * ：└───────────────┘：
 * ：‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥：
 *
 * dot style
 * ---------
 *     CHAR    VKB
 * ：‥│‥‥‥│‥‥‥‥‥‥‥‥‥‥‥：←ENT
 * ：┌│───↓──────────┐：
 * ：│↓      ○                    │←─FRAME
 * ：│┌─┐  ・・・・      ・・・・│：
 * ：││訳│  ・・・・      ・・・・│：
 * ：│└─┘  ・・・・      ・・・●│：
 * ：└───────────────┘：
 * ：‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥：
 */

#define ENT_W 136
#define ENT_H 43
#define ENT_ROWS 15 //16
#define FRAME_X 3
#define FRAME_Y 2
#define FRAME_W 130
#define FRAME_H 39
#define CHAR_X 11
#define CHAR_Y 16
#define CHAR_SIZE 16
#define VKB_X 38
#define VKB_Y 6
#define VKB_S 8
//<multishift>
#define PREF_SIZE 9             // テーブルを代表する記号用の文字
#define PREF_X (CHAR_X + CHAR_SIZE - 2)
#define PREF_Y (CHAR_Y - PREF_SIZE - 2)
#define PREF_COL COL_RED
//</multishift>

//<v127c>
// [連習スレ2:522] 256 色環境での問題
//#define GRAYTONE(x) RGB((x), (x), (x))
#define GRAYTONE(x) PALETTERGB((x), (x), (x))
#define COLORDEF(r, g, b) PALETTERGB((r), (g), (b))
//</127c>

#define COL_BLACK   GRAYTONE(0x00)
//<v127c>
//#define COL_RED     RGB(0xff, 0x00, 0x00)
//#define COL_GREEN   RGB(0x00, 0xff, 0x00)
//#define COL_YELLOW  RGB(0xff, 0xff, 0x00)
//#define COL_GRAY    RGB(0x80, 0x80, 0x80)
//#define COL_BLUE    RGB(0x00, 0x00, 0xff)
//#define COL_CYAN    RGB(0x00, 0xff, 0xff)
//
//#define COL_MAGENTA RGB(0xff, 0x00, 0xff)
//#define COL_ORANGE  RGB(0xff, 0xc0, 0x00)
#define COL_RED     COLORDEF(0xff, 0x00, 0x00)
#define COL_GREEN   COLORDEF(0x00, 0xff, 0x00)
#define COL_YELLOW  COLORDEF(0xff, 0xff, 0x00)
#define COL_GRAY    COLORDEF(0x80, 0x80, 0x80)
#define COL_BLUE    COLORDEF(0x00, 0x00, 0xff)
#define COL_CYAN    COLORDEF(0x00, 0xff, 0xff)

#define COL_MAGENTA COLORDEF(0xff, 0x00, 0xff)
#define COL_ORANGE  COLORDEF(0xff, 0xc0, 0x00)
//</v127c>
#define COL_RP      COL_MAGENTA
#define COL_YR      COL_ORANGE

#define COL_BG      GRAYTONE(0xe0)
#define COL_VKB     GRAYTONE(0xf0)
#define COL_K0P     GRAYTONE(0xc0)
#define COL_K0B     GRAYTONE(0xf0)
#define COL_K1P     GRAYTONE(0x80)
#define COL_K1B     GRAYTONE(0xff)

// -------------------------------------------------------------------
// 大域変数

//
HWND hwnd;
HFONT hFont;
//<multishift>
HFONT hFont2;                   // テーブルを代表する記号用の文字
//</multishift>
HBITMAP hBitmap;
HDC hBuffer;

int bmW;
int bmH;

//<multishift2>
DIR_TABLE dirTable;
//</multishift2>

// 逆引きテーブル
StTable *stTable;

// 対象文字列
PTSTR clipboardStr;

// 各対象文字の文字とストローク列のエントリ
#define ENT_MAX (ENT_ROWS * 7)
int nEnt;
struct ENT {
    MOJI m;
    char s[3];
    STROKE st[STROKE_MAX + 1];  // XXX
    //<multishift>
    char sTbl[3];               // テーブルを代表する記号用の文字
    //</multishift>
} ent[ENT_MAX];

// 仮想鍵盤描画用の一時変数
int vkb[TC_NKEYS];

// オプション (kanchoku.ini で指定)
int OPT_useTTCode;
int OPT_dotStyle;
char *OPT_tableFile;
char *OPT_certain;
//<skipOutset>
int OPT_nonUniq;
int OPT_skipOutset;
//</skipOutset>

// CERTAIN 文字 (文字ヘルプの対象にしない文字)
std::vector <MOJI> *certainMoji;

// -------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int handleCreate(HWND);
int handleClick(HWND, bool);
int handlePaint(HWND);
int handleDestroy(HWND);

void initialize(HWND);
void readDir(DIR_TABLE *, ifstream *);
void readFromClipboard(HWND);
void updateEntData(HWND, bool);
void lookSt(char *, bool);
void makeVKB(STROKE *);
void drawVKB4Ent0(HWND, HDC, int, int, struct ENT *);
void drawVKB4Ent1(HWND, HDC, int, int, struct ENT *);

void error(HWND, char *);
void warn(HWND, char *);

// -------------------------------------------------------------------
#endif // KANSAKU_H
