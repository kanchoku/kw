#ifndef TABLE_WINDOW_H
#define TABLE_WINDOW_H
// -------------------------------------------------------------------
// class TableWindow
// メインのウィンドウ関連の処理

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

/* -------------------------------------------------------------------
 * メッセージ
 */
#define KANCHOKU_ICONCLK 0x1001

#define ID_MYTIMER 32767

/* -------------------------------------------------------------------
 * 横取りするキー
 *
 * キー番号 0 ～ 97 (== TC_NKEYS * 2 - 1) が漢直入力に用いるキー。
 * 0x100以降が機能キーとなっている。
 * 衝突しないように気をつけること。
 */
#define ACTIVE_KEY (0x100 + 1) // ON/OFF の切り替えキー
#define ACTIVE2_KEY (0x100 + 2) // ON/OFF の切り替えキー、その2
#define ACTIVEIME_KEY (0x100 + 6) // IME連動による ON/OFF の切り替え (漢直Winが従)

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
 * 仮想鍵盤 (1) - フォントとウィンドウの大きさ
 *
 *   MARGIN                   ←→ BLOCK
 * ：↓‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥ ↑
 * ：┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐： │
 * ：│  │  │  │  │  │  │  │  │  │  │  │： │
 * ：├─┼─┼─┼─┼─┤  ├─┼─┼─┼─┼─┤： │
 * ：│  │  │  │  │  │  │  │  │  │  │  │： │
 * ：├─┼─┼─┼─┼─┤  ├─┼─┼─┼─┼─┤： │
 * ：│  │  │  │  │  │  │  │  │  │  │  │： │ HEIGHT
 * ：├─┼─┼─┼─┼─┤  ├─┼─┼─┼─┼─┤： │
 * ：│  │  │  │  │  │  │  │  │  │  │  │： │
 * ：└┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┴┬┘： │
 * ：  │  │  │  │  │  │  │  │  │  │  │  ： │
 * ：  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘  ： │
 * ：‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥‥： ↓
 * ←────────── WIDTH  ─────────→
 */
#define CHAR_SIZE (styleFontSize)          // 文字の大きさ
#define LARGE_CHAR_SIZE (CHAR_SIZE+stylePadding*2)    // 大きい文字の大きさ
#define BLOCK_SIZE (CHAR_SIZE+stylePadding*3)         // 仮想鍵盤のキーの大きさ
#define MARGIN_SIZE (4)         // 仮想鍵盤の天地左右の余白
#define WIDTH  (MARGIN_SIZE * 2 + BLOCK_SIZE * 11 + 1)  // 仮想鍵盤の横幅
#define HEIGHT (MARGIN_SIZE * 2 + BLOCK_SIZE * 5 + 1)   // 仮想鍵盤の縦幅
#define TRUNC_MARK_SIZE (BLOCK_SIZE/3+3)
#define SHIFT_MARK_SIZE (BLOCK_SIZE/5+3)

/* -------------------------------------------------------------------
 * 仮想鍵盤 (2) - 色
 */
//<v127c>
// [連習スレ2:522] 256 色環境での問題
// !!! see also table_window.c--handleCreate()--palent
//#define GRAYTONE(x) RGB((x), (x), (x))
#define GRAYTONE(x) PALETTERGB((x), (x), (x))
//</v127c>
// モード ON 時の仮想鍵盤
#define COL_ON_LN styleCol[3] // 枠
#define COL_ON_K1 styleCol[4] // キー
#define COL_ON_K2 styleCol[5] // キー (影)
#define COL_ON_K3 styleCol[6] // キー (ハイライト)
#define COL_ON_M1 styleCol[7] // 余白
#define COL_ON_M2 styleCol[8] // 余白 (影)
#define COL_ON_M3 styleCol[9] // 余白 (ハイライト)
// モード OFF 時の仮想鍵盤
#define COL_OFF_LN styleCol[0] // 枠
#define COL_OFF_K1 styleCol[1] // キー
#define COL_OFF_M1 styleCol[2] // 余白
//<v127c>
// 文字色と背景色
//#define COL_BLACK       (RGB(0x00, 0x00, 0x00)) // 黒
//#define COL_WHITE       (RGB(0xff, 0xff, 0xff)) // 白
//#define COL_GRAY        (RGB(0x80, 0x80, 0x80)) // 灰
//#define COL_LT_GRAY     (RGB(0xc0, 0xc0, 0xc0)) // 薄灰
//#define COL_LT_RED      (RGB(0xff, 0xc0, 0xc0)) // 薄赤
//#define COL_LT_GREEN    (RGB(0xc0, 0xff, 0xc0)) // 薄緑
//#define COL_LT_BLUE     (RGB(0xc0, 0xc0, 0xff)) // 薄青
//#define COL_LT_YELLOW   (RGB(0xff, 0xff, 0xc0)) // 薄黄
//#define COL_LT_CYAN     (RGB(0xc0, 0xff, 0xff)) // 薄水
//#define COL_RED         (RGB(0xff, 0x00, 0x00)) // 赤
//#define COL_DK_CYAN     (RGB(0x00, 0x80, 0x80)) // 深水
////<v127a - gg>
//#define COL_DK_MAGENTA  (RGB(0x80, 0x00, 0x80)) // 深紫
////</v127a - gg>
//#define COLORDEF(r, g, b) RGB((r), (g), (b))
#define COLORDEF(r, g, b) PALETTERGB((r), (g), (b))
#define COL_BLACK       styleCol[18] // 黒
#define COL_WHITE       (COLORDEF(0xff, 0xff, 0xff)) // 白
#define COL_GRAY        (COLORDEF(0x80, 0x80, 0x80)) // 灰
#define COL_LT_GRAY     styleCol[13] // 薄灰
#define COL_LT_RED      styleCol[10] // 薄赤
#define COL_LT_GREEN    styleCol[11] // 薄緑
#define COL_LT_BLUE     styleCol[14] // 薄青
#define COL_LT_YELLOW   styleCol[12] // 薄黄
#define COL_LT_CYAN     styleCol[15] // 薄水
#define COL_RED         styleCol[19] // 赤
#define COL_DK_CYAN     styleCol[16] // 深水
#define COL_DK_MAGENTA  styleCol[17] // 深紫
// !!! see also table_window.c--handleCreate()--palent
//</v127c>

/* -------------------------------------------------------------------
 * TableWindow クラス
 */
class TableWindow {
private:
    // T-Code 変換器
    TCode *tc;

    // インスタンスハンドル
    HINSTANCE instance;
    // フォント
    HFONT hFont;
    HFONT hLFont;
    //<v127c>
    // パレット
    HPALETTE hPalette;
    //</v127c>
    // メッセージの引数を格納
    HWND hwnd;
    WPARAM wParam;
    WPARAM lParam;

    // タスクトレイに登録
    NOTIFYICONDATA nid;

    // WM_KANCHOKU_CHAR関連
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

    enum HOTKEYMODE { OFF, NORMAL, EDITCLAUSE };
    HOTKEYMODE hotKeyMode;
    // 語幹伸ばし縮め用キー
    // T-Codeキーとかぶっている場合のみLT_KEY、GT_KEYの代わりに使用
    int tc_lt_key;
    int tc_gt_key;

    // Shift押下検出（仮想鍵盤表示用）
    bool isShift;
    bool isShiftPrev;
    // Timeout判定用
    int deciSecAfterStroke;

    // スタイル設定
    COLORREF styleCol[20];
    char styleFontName[LF_FACESIZE];
    int styleFontSize;
    int stylePadding;

public:
    // コンストラクタ
    TableWindow(HINSTANCE i);
    // デストラクタ
    ~TableWindow();
    // window procedure
    int wndProc(HWND, UINT, WPARAM, LPARAM);

private:
    // 起動/待機
    void activate();            // 起動 (HotKey の割付)
    void inactivate();          // 待機 (HotKey の解放)
    //void setSpecialHotKey();    // 特殊 HotKey の確保と解放
    void setMazeHotKey(int);
    void disableHotKey();
    void resumeHotKey();
    void setTitleText();        // タイトルバーの文字列設定

    // メッセージハンドラ
    int handleCreate();         // WM_CREATE
    int handleDestroy();        // WM_DESTROY
    int handleLButtonDown();    // WM_LBUTTONDOWN
    int handlePaint();          // WM_PAINT
    int handleTimer();          // WM_TIMER
    int handleNotifyVKPROCESSKEY(); // WM_KANCHOKU_NOTIFYVKPROCESSKEY
    int handleNotifyIMEStatus();    // WM_KANCHOKU_NOTIFYIMESTATUS
    int handleHotKey();         // WM_HOTKEY

    // T-Code 関連
    void initTC();              // T-Code 変換器の初期化
    void output();              // T-Code 変換結果の出力

    // 仮想鍵盤の描画
    void drawFrameOFF(HDC hdc); // OFF 時の仮想鍵盤の枠
    void drawFrame50(HDC hdc);  // 通常時の仮想鍵盤の枠
    void drawFrame10(HDC hdc);  // 少数候補選択時の仮想鍵盤の枠
    void drawVKB50(HDC hdc, bool isWithBothSide = false);        // 通常時の仮想鍵盤
    void drawVKB10(HDC);        // 少数候補選択時の仮想鍵盤
    void drawMiniBuffer(HDC, int, COLORREF, MojiBuffer *);
                                // ミニバッファ

    // エラー処理
    void error(char *);         // エラーを表示し、終了
    void warn(char *);          // 警告を表示するが、継続

    void readTargetWindowSetting(char *); // 出力先ウィンドウごとの設定の読込
    int getOutputMethod(HWND);  // 指定したウィンドウに対するoutputMethodを取得

    void makeStyle();
    void readStyleSetting();
};

// -------------------------------------------------------------------
#endif // TABLE_WINDOW_H
