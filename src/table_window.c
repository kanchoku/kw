#ifdef _MSC_VER //<OKA>
#define for if(0);else for
#endif          //</OKA>
#include "table_window.h"
#include "debug.h"
// -------------------------------------------------------------------

//BOOL CALLBACK CtlProc(HWND, UINT, WPARAM, LPARAM);

// -------------------------------------------------------------------
// コンストラクタ
TableWindow::TableWindow(HINSTANCE i) {
    hFont = 0;
    hLFont = 0;
    instance = i;
    tc = 0;
}

// -------------------------------------------------------------------
// デストラクタ
TableWindow::~TableWindow() {
    DeleteObject(hFont);
    DeleteObject(hLFont);
    delete(tc);
}

// -------------------------------------------------------------------
// window procedure
int TableWindow::wndProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    // window handle や message の引数を取っておく
    hwnd = w;
    wParam = wp;
    lParam = lp;

    bool isShiftNow;
    // 各メッセージの handler を呼ぶ
    switch (msg) {
    case WM_CREATE:
#define ID_MYTIMER 32767
        SetTimer(w, ID_MYTIMER, 500, NULL);
        isShift = !!(GetKeyState(VK_SHIFT) & 0x8000);
        return handleCreate();

    case WM_TIMER:
        isShiftNow = !!(GetKeyState(VK_SHIFT) & 0x8000);
        if ((tc->mode == TCode::NORMAL || tc->mode == TCode::CAND1)
            && !tc->helpMode 
            && tc->isAnyShiftKana
            && isShiftNow != isShift) InvalidateRect(w, NULL, FALSE);
        isShift = isShiftNow;
        return 0;

    case WM_PAINT:
        return handlePaint();

    case WM_LBUTTONUP:          // 左クリックで ON/OFF
        wParam = ACTIVE_KEY;
        return handleHotKey();

    case WM_RBUTTONUP:          // 右クリックでバージョン情報
                                // 名前とくい違ってるけど
        return handleLButtonDown();

    case WM_DESTROY:
        KillTimer(w, ID_MYTIMER);
        return handleDestroy();

    //<record>
    case WM_QUERYENDSESSION:    // Windows の終了オプション選択時
        if (tc != 0) {
            tc->recordOutput();
            tc->statOutput();
        }
        return TRUE;
    //</record>

    case KANCHOKU_ICONCLK:
        if (lParam == WM_LBUTTONDOWN) {
            wParam = ACTIVE_KEY;
            return handleHotKey();
        }
        return 0;

    case WM_HOTKEY:
        return handleHotKey();
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* -------------------------------------------------------------------
 * 起動と待機
 */

// -------------------------------------------------------------------
// 起動
void TableWindow::activate() {
    // アイコン
    nid.hIcon = LoadIcon(instance, "kanmini1");
    Shell_NotifyIcon(NIM_MODIFY, &nid);

    // 表示する
    //<hideOnStartup>
    //if (tc->OPT_offHide == 1) {
    if (tc->OPT_offHide != 2 && !IsWindowVisible(hwnd)) {
    //</hideOnStartup>
        ShowWindow(hwnd, SW_SHOWNA);
    }
    //<v127c - offHide2>
    // offHide=2 の時は (とりあえず) 表示しない
    //</v127c>

    hotKeyMode = NORMAL;
    // HotKey の割付
    for (int i = 0; i < TC_NKEYS; i++) {
        RegisterHotKey(hwnd, i, 0, tc->vkey[i]);
    }
    // HotKey の割付 (シフト打鍵)
    if (tc->OPT_shiftKana != 0 || tc->isAnyShiftKana) {
        for (int i = 0; i < TC_NKEYS; i++) {
            //<v127a - shiftcheck>
            //RegisterHotKey(hwnd, TC_SHIFT(i), MOD_SHIFT, tc->vkey[i]);
            if (tc->isShiftKana[i])
                RegisterHotKey(hwnd, TC_SHIFT(i), MOD_SHIFT, tc->vkey[i]);
            //</v127a - shiftcheck>
        }
    }

    RegisterHotKey(hwnd, ESC_KEY, 0, VK_ESCAPE);
    RegisterHotKey(hwnd, BS_KEY,  0, VK_BACK);
    RegisterHotKey(hwnd, RET_KEY, 0, VK_RETURN);
    RegisterHotKey(hwnd, TAB_KEY, 0, VK_TAB);
    if (tc->OPT_useCtrlKey) {
        RegisterHotKey(hwnd, CG_KEY, MOD_CONTROL, 'G');
        RegisterHotKey(hwnd, CH_KEY, MOD_CONTROL, 'H');
        RegisterHotKey(hwnd, CM_KEY, MOD_CONTROL, 'M');
        RegisterHotKey(hwnd, CJ_KEY, MOD_CONTROL, 'J');
        RegisterHotKey(hwnd, CI_KEY, MOD_CONTROL, 'I');
    }

    // 通常モードにする
    tc->mode = TCode::NORMAL;
    tc->preBuffer->clear();
    tc->postBuffer->clear();   //  後置型交ぜ書き変換用のバッファをクリア
    tc->helpMode = 0;          // ヘルプ非表示
}

// -------------------------------------------------------------------
// 待機
void TableWindow::inactivate() {
    // アイコン
    nid.hIcon = LoadIcon(instance, "kanmini0");
    Shell_NotifyIcon(NIM_MODIFY, &nid);

    // 隠す
    //<v127c - offHide2>
    //if (tc->OPT_offHide == 1) {
    // offHide=2 時も隠す
    if (tc->OPT_offHide) {
    //</v127c>
        ShowWindow(hwnd, SW_HIDE);
    }

    if (tc->OPT_conjugationalMaze == 2) setMazeHotKey(0);
    hotKeyMode = OFF;
    // HotKey の解放
    for (int i = 0; i < TC_NKEYS; i++) {
        UnregisterHotKey(hwnd, i);
    }
    // HotKey の解放 (シフト打鍵)
    if (tc->OPT_shiftKana != 0 || tc->isAnyShiftKana) {
        for (int i = 0; i < TC_NKEYS; i++) {
            //<v127a - shiftcheck>
            //UnregisterHotKey(hwnd, TC_SHIFT(i));
            if (tc->isShiftKana[i])
                UnregisterHotKey(hwnd, TC_SHIFT(i));
            //</v127a - shiftcheck>
        }
    }
    UnregisterHotKey(hwnd, ESC_KEY);
    UnregisterHotKey(hwnd, BS_KEY);
    UnregisterHotKey(hwnd, RET_KEY);
    UnregisterHotKey(hwnd, TAB_KEY);
    if (tc->OPT_useCtrlKey) {
        UnregisterHotKey(hwnd, CG_KEY);
        UnregisterHotKey(hwnd, CH_KEY);
        UnregisterHotKey(hwnd, CM_KEY);
        UnregisterHotKey(hwnd, CJ_KEY);
        UnregisterHotKey(hwnd, CI_KEY);
    }

    // 通常モードにする
    tc->mode = TCode::NORMAL;
    tc->mode = TCode::OFF;
}

// -------------------------------------------------------------------
// EscやBs、Ctrl-Hなど
//void TableWindow::setSpecialHotKey() {
//    if((tc->table == tc->currentBlock) &&
//       (tc->mode == TCode::NORMAL)) {
//        UnregisterHotKey(hwnd, ESC_KEY);
//        UnregisterHotKey(hwnd, BS_KEY);
//        UnregisterHotKey(hwnd, RET_KEY);
//        UnregisterHotKey(hwnd, TAB_KEY);
//    } else {
//        RegisterHotKey(hwnd, ESC_KEY, 0, VK_ESCAPE);
//        RegisterHotKey(hwnd, BS_KEY, 0, VK_BACK);
//        RegisterHotKey(hwnd, RET_KEY, 0, VK_RETURN);
//        RegisterHotKey(hwnd, TAB_KEY, 0, VK_TAB);
//    }
//}

void TableWindow::setMazeHotKey(int onoff) {
    if (!onoff) {
        if (hotKeyMode != EDITCLAUSE) return;
        hotKeyMode = NORMAL;
        if (tc_lt_key == -1 || !tc->isShiftKana[tc_lt_key])
            UnregisterHotKey(hwnd, LT_KEY);
        if (tc_gt_key == -1 || !tc->isShiftKana[tc_gt_key])
            UnregisterHotKey(hwnd, GT_KEY);
    } else {
        if (hotKeyMode == EDITCLAUSE) return;
        hotKeyMode = EDITCLAUSE;
        if (tc_lt_key == -1 || !tc->isShiftKana[tc_lt_key])
            RegisterHotKey(hwnd, LT_KEY, MOD_SHIFT, 0xbc);  //VK_OEM_COMMA
        if (tc_gt_key == -1 || !tc->isShiftKana[tc_gt_key])
            RegisterHotKey(hwnd, GT_KEY, MOD_SHIFT, 0xbe);  //VK_OEM_PERIOD
    }
}

// -------------------------------------------------------------------
// タイトルバーの文字をセット
void TableWindow::setTitleText() {
    //<multishift2>
    char str[256];
    sprintf(str, "漢直窓");
    if (tc->dirTable[DIR_table_name]) {
        strcat(str, " ");
        strcat(str, tc->dirTable[DIR_table_name]);
    }
    //</multishift2>
    if (tc->mode == TCode::OFF) {
        //<multishift2>
        //SetWindowText(hwnd, "漢直窓 - OFF");
        strcat(str, " - OFF");
        SetWindowText(hwnd, str);
        //</multishift2>
    } else {
        //<multishift2>
        ///char str[256];
        //strcpy(str, "漢直窓 - ON");
        strcat(str, " - ON");
        //</multishift2>
        if (tc->hirakataMode || tc->hanzenMode || tc->punctMode || tc->OPT_maze2gg) {
            strcat(str, " [");
            //<hankana>
            //strcat(str, (tc->hirakataMode ? "ア" : "―"));
            //strcat(str, (tc->hanzenMode   ? "全" : "―"));
            //strcat(str, (tc->punctMode    ? "句" : "―"));
            strcat(str, (tc->hirakataMode ? "カ" : "―"));
            strcat(str, "|");
            strcat(str, (tc->hanzenMode   ? "全半" : "――"));
            strcat(str, "|");
            strcat(str, (tc->punctMode    ? "句読" : "――"));
            strcat(str, "|");
            strcat(str, (tc->OPT_maze2gg  ? "習" : "―"));
            //</hankana>
            strcat(str, "]");
        }
        SetWindowText(hwnd, str);
    }
}

/* -------------------------------------------------------------------
 * メッセージハンドラ
 */

// -------------------------------------------------------------------
// WM_CREATE
int TableWindow::handleCreate() {
    // T-Code 変換器の初期化
    initTC();

    // タスクトレイに登録
    //NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 0;
    nid.uFlags = 7; // NIF_ICON | NIF_MESSAGE | NIF_TIP
    nid.uCallbackMessage = KANCHOKU_ICONCLK;
    nid.hIcon = LoadIcon(instance, "kanmini0");
    strcpy(nid.szTip, "漢直窓");
    Shell_NotifyIcon(0 /* NIM_ADD */, &nid);

    // 外枠の大きさを取得
    RECT winRect;
    GetWindowRect(hwnd, &winRect);

    // 中身の大きさを取得
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    // 左上の座標を取得
    int sX = winRect.left;
    int sY = winRect.top;

    // 外枠と中身の差を取得
    int dX = (winRect.right - winRect.left)
        - (clientRect.right - clientRect.left);
    int dY = (winRect.bottom - winRect.top)
        - (clientRect.bottom - clientRect.top);

    // 大きさを更新
    if (tc->OPT_xLoc != -1) { sX = tc->OPT_xLoc; }
    if (tc->OPT_yLoc != -1) { sY = tc->OPT_yLoc; }
    MoveWindow(hwnd, sX, sY, WIDTH + dX, HEIGHT + dY, TRUE);

    // フォントを設定
    LOGFONT lf;
    lf.lfHeight = CHAR_SIZE;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = 0;
    lf.lfItalic = 0;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = SHIFTJIS_CHARSET;
    lf.lfOutPrecision = 0;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;
    strcpy(lf.lfFaceName, "ＭＳ ゴシック");
    hFont = CreateFontIndirect(&lf);

    // 大きいフォントを設定
    lf.lfHeight = LARGE_CHAR_SIZE;
    hLFont = CreateFontIndirect(&lf);

    //<v127c>
    // [連習スレ2:537-538]
    // 使う色をパレットに用意しておく
    // !!! see also table_window.h
    COLORREF palent[] = {
//        COL_ON_LN,
//        COL_ON_K1,
//        COL_ON_K2,
//        COL_ON_M1,
//        COL_ON_M2,
//        COL_OFF_LN,
//        COL_OFF_K1,
//        COL_OFF_M1,
        COL_BLACK       ,
        COL_WHITE       ,
        COL_GRAY        ,
        COL_LT_GRAY     ,
        COL_LT_RED      ,
        COL_LT_GREEN    ,
        COL_LT_BLUE     ,
        COL_LT_YELLOW   ,
        COL_LT_CYAN     ,
        COL_RED         ,
        COL_DK_CYAN     ,
        COL_DK_MAGENTA  ,
        (COLORREF)-1
    };
    int i, n;
    for (n=0; palent[n]!=(COLORREF)-1; n++) ;
    char *work = new char[sizeof (LOGPALETTE) + (n-1) * sizeof (PALETTEENTRY)];
    LOGPALETTE *lpPalette = (LOGPALETTE *)work;
    lpPalette->palVersion = 0x0300;
    lpPalette->palNumEntries = n;
    for (i=0; i < n; i++) {
      lpPalette->palPalEntry[i].peRed = GetRValue(palent[i]);
      lpPalette->palPalEntry[i].peGreen = GetGValue(palent[i]);
      lpPalette->palPalEntry[i].peBlue = GetBValue(palent[i]);
      lpPalette->palPalEntry[i].peFlags = NULL;
    }
    hPalette = CreatePalette(lpPalette);
    delete [] work;
    //</v127c>

    // 起動のための HotKey を登録
    //<OKA> support unmodified hot key
    if (tc->OPT_hotKey)
        RegisterHotKey(hwnd, ACTIVE_KEY, MOD_CONTROL, tc->OPT_hotKey);
    if (tc->OPT_unmodifiedHotKey)
        RegisterHotKey(hwnd, ACTIVE2_KEY, 0, tc->OPT_unmodifiedHotKey);
    //</OKA>

    // 待機状態に
    inactivate();

    return 0;
}

// -------------------------------------------------------------------
// WM_DESTROY
int TableWindow::handleDestroy() {

    inactivate();               // 待機状態にする
    //<OKA> support unmodified hot key
    if (tc->OPT_hotKey)
        UnregisterHotKey(hwnd, ACTIVE_KEY); // HotKey を削除
    if (tc->OPT_unmodifiedHotKey)
        UnregisterHotKey(hwnd, ACTIVE2_KEY); // HotKey を削除
    //</OKA>

    //<record>
    // 記録を出力
    tc->recordOutput();

    // 統計を出力
    tc->statOutput();
    //</record>

    //<v127c>
    DeleteObject(hPalette);
    //</v127c>
    if (lpfnMyEndHook) {
        lpfnMyEndHook();
        FreeLibrary(hKanCharDLL);
        RemoveProp(hwnd, "KanchokuWin_KanCharDLL_NextHook");
    }

    // タスクトレイから削除
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 0;
    Shell_NotifyIcon(2 /* NIM_DELETE */, &nid);

    // 終了
    PostQuitMessage(0);
    return 0;
}

// -------------------------------------------------------------------
// WM_LBUTTONDOWN
int TableWindow::handleLButtonDown() {
    char s[1024];
    sprintf(s,
            "漢直窓 %s\n"
            "\n"
            "    キーボード  %s\n"
            "    テーブル  %s\n"
            "    部首合成  %s\n"
            //<v127a - gg>
            //"    交ぜ書き  %s",
            "    交ぜ書き  %s\n"
            "    熟語ガイド  %s",
            //</v127a - gg>
            VERSION,
            tc->OPT_keyboard,
            tc->OPT_tableFile,
            (tc->OPT_bushu ? tc->OPT_bushu : "(未使用)"),
            //<v127a - gg>
            //(tc->OPT_mazegaki ? tc->OPT_mazegaki : "(未使用)"));
            (tc->OPT_mazegaki ? tc->OPT_mazegaki : "(未使用)"),
            (tc->OPT_gg ? tc->OPT_gg : "(未使用)"));
            //</v127a - gg>
    MessageBoxEx(hwnd,
                 s,
                 "バージョン情報",
                 MB_OK | MB_ICONINFORMATION, LANG_JAPANESE);

    //DialogBox(instance, "ControlBox", hwnd, CtlProc);
    return 0;
}

// -------------------------------------------------------------------
// WM_PAINT
int TableWindow::handlePaint() {
    PAINTSTRUCT ps;
    HDC hdc;

    // 描画開始
    hdc = BeginPaint(hwnd, &ps);
    //<v127c>
    // パレットを使用
    SelectPalette(hdc, hPalette, FALSE);
    RealizePalette(hdc);
    //</v127c>
    // タイトル文字列設定
    setTitleText();

    // OFF 時
    if (tc->mode == TCode::OFF) {
        drawFrameOFF(hdc);
        goto END_PAINT;
    }

    // 文字ヘルプ表示モード
    if (tc->helpMode) {
        drawFrame50(hdc);
        MojiBuffer mb(4);
        mb.clear(); mb.pushSoftN("　", 2); // XXX 全角空白を詰める
        mb.pushSoft(tc->helpBuffer->moji(tc->helpOffset));
        drawMiniBuffer(hdc, 4, COL_LT_YELLOW, &mb);
        drawVKB50(hdc);
        goto END_PAINT;
    }

    // ヒストリ入力モード
    if (tc->mode == TCode::HIST) {
        drawFrame10(hdc);
        drawMiniBuffer(hdc, 5, COL_LT_BLUE, tc->preBuffer);
        drawVKB10(hdc);
        goto END_PAINT;
    }

    // 唯一候補表示モード
    if (tc->mode == TCode::CAND1) {
        drawFrame50(hdc);
        if (tc->currentCand->size() == 1) {
            MojiBuffer mb(strlen((*tc->currentCand)[0])+tc->okuriLen);
            mb.pushSoft((*tc->currentCand)[0]);
            if (tc->okuriLen) mb.pushSoft(tc->preBuffer->string(-tc->okuriLen));
            int ov = mb.length() - 4;
            if (ov > 0) mb.popN(ov);
            drawMiniBuffer(hdc, 4, COL_LT_BLUE, &mb);
        } else {
            drawMiniBuffer(hdc, 4, COL_LT_BLUE, tc->preBuffer);
        }
        drawVKB50(hdc, tc->isAnyShiftKana);
        goto END_PAINT;
    }

    // 少数候補表示モード
    if (tc->mode == TCode::CAND && tc->currentCand->size() <= 10) {
        drawFrame10(hdc);
        drawMiniBuffer(hdc, 5, COL_LT_BLUE, tc->preBuffer);
        drawVKB10(hdc);
        goto END_PAINT;
    }

    // 多数候補表示モード
    if (tc->mode == TCode::CAND && 10 < tc->currentCand->size()) {
        drawFrame50(hdc);
        drawMiniBuffer(hdc, 4, COL_LT_BLUE, tc->preBuffer);
        drawVKB50(hdc);
        goto END_PAINT;
    }

    // 通常入力モード
    if (tc->mode == TCode::NORMAL) {
        drawFrame50(hdc);
        if (0 < tc->preBuffer->length()) {
            drawMiniBuffer(hdc, 4, COL_LT_GREEN, tc->preBuffer);
        }
        drawVKB50(hdc, tc->isAnyShiftKana);
        goto END_PAINT;
    }

    // 描画終了
 END_PAINT:
    EndPaint(hwnd, &ps);
    return 0;
}

// -------------------------------------------------------------------
// WM_HOTKEY
int TableWindow::handleHotKey() {
    int key = wParam;

    // ON/OFF
    if (key == ACTIVE_KEY || key == ACTIVE2_KEY) { //<OKA> support unmodified hot key
        tc->reset();
        tc->preBuffer->clear();
        tc->postBuffer->clear();
        //<v127c - postInPre>
        tc->postInPre = 0;
        tc->postDelete = 0;
        //</v127c>
        tc->clearCandGG();
        if (tc->mode == TCode::OFF) {
            tc->mode = TCode::NORMAL;
            activate();
        } else {
            tc->mode = TCode::OFF;
            inactivate();
        }
        goto DRAW;
    }

    /* ---------------------------------------------------------------
     * 入力
     */
    if (hotKeyMode == EDITCLAUSE) {
        if (key == TC_SHIFT(tc_lt_key)) key = LT_KEY;
        if (key == TC_SHIFT(tc_gt_key)) key = GT_KEY;
    }
    switch (tc->mode) {
    case TCode::NORMAL: tc->keyinNormal(key); break;
    case TCode::CAND:   tc->keyinCand(key);   break;
    case TCode::CAND1:  tc->keyinCand1(key);  break;
    case TCode::HIST:   tc->keyinHist(key);   break;
    default:                    // ここには来ないはずだが
        goto DRAW; break;
    }

    /* ---------------------------------------------------------------
     * 変換
     */
    int check;
    do {
        check = 0;
        while (tc->isReducibleByBushu()) { tc->reduceByBushu(); check = 1; }
        while (tc->isReducibleByMaze())  { tc->reduceByMaze();  check = 1; }
    } while (check != 0);

    if (tc->OPT_conjugationalMaze == 2) {
        setMazeHotKey(tc->mode == TCode::CAND || tc->mode == TCode::CAND1);
    }
    /* ---------------------------------------------------------------
     * 出力
     */
    output();

    /* ---------------------------------------------------------------
     * 描画
     */
 DRAW:
    //<v127c - offHide2>
    // offHide=2 - 補助機能利用時以外は仮想鍵盤を非表示
    // * 表示   - 補助変換・候補選択・文字ヘルプ・ヒストリ入力
    // * 非表示 - 通常のストローク入力
    if (tc->OPT_offHide == 2) {
        if (tc->mode == TCode::OFF
            || tc->mode == TCode::NORMAL
            && tc->helpMode == 0    // helpMode と mode は独立
            && tc->preBuffer->length() == 0 // 補助変換中でない
            && tc->explicitGG == 0) { // 強制練習中でない
            ShowWindow(hwnd, SW_HIDE);
        } else {
            ShowWindow(hwnd, SW_SHOWNA);
        }
    }
    //</v127c>

    // 仮想鍵盤を作成
    tc->makeVKB();

    // タイトル文字列を更新
    //setTitleText();

    // windowを書き直す (ここで仮想鍵盤を表示)
    //<v127c>
    // [連習スレ2:517] キーリピート時の問題
    //RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    InvalidateRect(hwnd, NULL, TRUE);
    //</v127c>

    return 0;
}

/* -------------------------------------------------------------------
 * T-Code 関連
 */

/* -------------------------------------------------------------------
 * initTC()
 * --------
 * 設定ファイル (kanchoku.ini) からオプションを読みこみ、
 * T-Code 変換器を生成する。
 *
 * オプション一覧 - デフォルト値と意味
 * -----------------------------------
 *
 * ファイル kanchoku.ini の kanchoku セクションに指定するオプション
 *
 * - hotKey=<xx>            dc (C-\)    ON/OFF のキー (仮想キーコード)
 * - unmodifiedHotKey=<xx>  (無指定)    無修飾のホットキー <v127d/>
 * - keyboard="file"        106.key     キーボード定義ファイル
 * - tableFile="file"       t.tbl       テーブル定義ファイル
 *
 * - bushu="file"           (無指定)    部首合成辞書ファイル (無指定で不使用)
 * - bushuAlgo={OKA,YAMANOBE}
 *                          (無指定)    部首合成変換アルゴリズム (無指定 = OKA)
 * - mazegaki="file"        (無指定)    交ぜ書き辞書ファイル (無指定で不使用)
 * - conjugationalMaze=[012]
 *                          1           活用語を語幹の読みで変換
 * - gg="file"              (無指定)    熟語補完辞書ファイル (無指定で不使用)
 *
 * - record="file"          (無指定)    入力記録のファイル
 * - stat="file"            (無指定)    入力文字の統計のファイル
 *
 * - shiftKana=[01]         0           シフト打鍵でひらがな/かたかな変換
 * - enableHankakuKana=[01] 0           全/半モード時に半角かたかな変換
 * - outputMethod=[012]     0           文字の送出方法
 *                                      - 0 (WM_CHAR)
 *                                      - 1 (WM_IME_CHAR)
 *                                      - 2 (WM_KANCHOKU_CHAR)
 *                                      -   (フック + ImmSetCompositionString)
 *                                      アプリごとの設定も可 (manual 参照)
 * - useWMIMECHAR=[012]     0           outputMethod の同様 (後方互換)
 * - outputSleep=n          0           仮想キーを output するときの Sleep 量
 *
 * - xLoc=<n>               -1          ウィンドウ初期位置 (-1 で無指定)
 * - yLoc=<n>               -1          〃
 * - offHide=[01]           0           OFF 時にウィンドウ非表示
 *                                      //<v127c - offHide2>
 *                                      =2 指定は通常入力時も非表示
 *                                      //</v127c>
 * - followCaret=[01]       0           ウィンドウがカーソルに追従
 *
 * - hardBS=[01]            0           BS は常に (第二打鍵でも) 文字を消去
 * - useCtrlKey=[01]        0           例えば C-h を BS として扱うなど
 * - useTTCode=[01]         0           三枚表 T-Code スタイルの文字ヘルプ
 * - win95=[01]             0           Windows95 でのフォントのずれを補正
 */
void TableWindow::initTC() {
    // 設定ファイルを開く
    char iniFile[MAX_PATH + 1];
    GetCurrentDirectory(sizeof(iniFile), iniFile);
    strcat(iniFile, "\\kanchoku.ini");

    ifstream *is = new ifstream();

    /* ---------------------------------------------------------------
     * 入力方式関連
     */
    // hotkey
    char hotKey[255];
    GetPrivateProfileString("kanchoku", "hotkey", "dc",
                            hotKey, sizeof(hotKey), iniFile);
    int OPT_hotKey = (int)strtol(hotKey, NULL, 16);
    //<OKA> support unmodified hot key
    GetPrivateProfileString("kanchoku", "UnmodifiedHotKey", "",
                            hotKey, sizeof(hotKey), iniFile);
    int OPT_unmodifiedHotKey = (int)strtol(hotKey, NULL, 16);
    //</OKA>
    //<OKA> support unmodified hot key
    if (OPT_hotKey == 0 && OPT_unmodifiedHotKey == 0) {
        error("「hotkey|unmodifiedHotKey=(16 進キーコード)」の設定がまちがっているようです");
    }
    //</OKA>

    // keyboard file
    char keyFile[255];      // キーボードファイル名
    GetPrivateProfileString("kanchoku", "keyboard", "106.key",
                            keyFile, sizeof(keyFile), iniFile);
    if (keyFile[0] == 0) {
        error("「keyboard=(ファイル名)」の設定がまちがっているようです");
    }

    // table file
    char tableFile[255];    // テーブルファイル名
    GetPrivateProfileString("kanchoku", "tablefile", "t.tbl",
                            tableFile, sizeof(tableFile), iniFile);
    if (tableFile[0] == 0) {
        error("「tablefile=(ファイル名)」の設定がまちがっているようです");
    }

    /* ---------------------------------------------------------------
     * 補助変換関連
     */
    // bushu dictionary
    char bushuFile[255];
    GetPrivateProfileString("kanchoku", "bushu", "",
                            bushuFile, sizeof(bushuFile), iniFile);
    int bushuReady;
    if (bushuFile[0] == 0) { bushuReady = 0; }
    else                   { bushuReady = 1; }
    
    // bushu algorithm
    char bushuAlgo[255];
    int OPT_bushuAlgo;
    GetPrivateProfileString("kanchoku", "bushuAlgo", "",
                            bushuAlgo, sizeof(bushuAlgo), iniFile);
    if (bushuAlgo[0] == '\0' ||  // default to OKA algorithm
        stricmp(bushuAlgo, "OKA") == 0) {
        OPT_bushuAlgo = TC_BUSHU_ALGO_OKA;
    } else if (stricmp(bushuAlgo, "YAMANOBE") == 0) {
        OPT_bushuAlgo = TC_BUSHU_ALGO_YAMANOBE;
    } else {
        warn("bushuAlgo には OKA, YAMANOBE のいずれかを指定してください。");
        OPT_bushuAlgo = TC_BUSHU_ALGO_OKA; // とりあえず OKA algorithm に
    }

    // mazegaki dictionary
    char mazegakiFile[255];
    GetPrivateProfileString("kanchoku", "mazegaki", "",
                            mazegakiFile, sizeof(mazegakiFile), iniFile);
    int mazeReady;
    if (mazegakiFile[0] == 0) { mazeReady = 0; }
    else                      { mazeReady = 1; }

    // conjugationalMaze
    int OPT_conjugationalMaze =
        GetPrivateProfileInt("kanchoku", "conjugationalmaze", 1, iniFile);

    //<v127a - gg>
    // gg dictionary
    char ggFile[255];
    GetPrivateProfileString("kanchoku", "gg", "",
                            ggFile, sizeof(ggFile), iniFile);
    int ggReady;
    if (ggFile[0] == 0) { ggReady = 0; }
    else                { ggReady = 1; }
    //</v127a - gg>

    //<gg-defg>
    // defg
    char OPT_defg[255];
    GetPrivateProfileString("kanchoku", "defg", "",
                            OPT_defg, sizeof(OPT_defg), iniFile);
    //</gg-defg>

    // 
    int OPT_maze2gg =
        GetPrivateProfileInt("kanchoku", "maze2gg", 0, iniFile);

    char prefixautoassign[255];
    GetPrivateProfileString("kanchoku", "prefixautoassign", "",
                            prefixautoassign, sizeof(prefixautoassign), iniFile);

    //<record>
    // record
    char OPT_record[255];
    GetPrivateProfileString("kanchoku", "record", "",
                            OPT_record, sizeof(OPT_record), iniFile);

    // stat
    char OPT_stat[255];
    GetPrivateProfileString("kanchoku", "stat", "",
                            OPT_stat, sizeof(OPT_stat), iniFile);
    //</record>

    /* ---------------------------------------------------------------
     * シフト打鍵
     */
    int OPT_shiftKana =
        GetPrivateProfileInt("kanchoku", "shiftkana", 0, iniFile);
        
    /* ---------------------------------------------------------------
     * 半角かな変換
     */
    int OPT_enableHankakuKana =
        GetPrivateProfileInt("kanchoku", "enableHankakuKana", 
                             0, iniFile);

    /* ---------------------------------------------------------------
     * 出力メッセージ
     */
    // useWMIMECHAR
    int OPT_useWMIMECHAR =
        GetPrivateProfileInt("kanchoku", "usewmimechar", 0, iniFile);
    // outputMethod - useWMIMECHAR の alias で useWMIMECHAR より優先
    // (というか、 useWMIMECHAR の方を OBSOLETE にする予定)
    char OPT_outputMethod[255];
    GetPrivateProfileString("kanchoku", "outputmethod", "",
                            OPT_outputMethod, sizeof(OPT_outputMethod),
                            iniFile);
    if (OPT_outputMethod[0] != '\0') {
        if (stricmp(OPT_outputMethod, "WMCHAR")  == 0 ||
            stricmp(OPT_outputMethod, "WM_CHAR") == 0 ||
            stricmp(OPT_outputMethod, "0")       == 0) {
            OPT_useWMIMECHAR = OUT_WMCHAR;
        } else if (stricmp(OPT_outputMethod, "WMIMECHAR")   == 0 ||
                   stricmp(OPT_outputMethod, "WM_IME_CHAR") == 0 ||
                   stricmp(OPT_outputMethod, "1")           == 0) {
            OPT_useWMIMECHAR = OUT_WMIMECHAR;
        } else if (stricmp(OPT_outputMethod, "WMKANCHOKUCHAR")   == 0 ||
                   stricmp(OPT_outputMethod, "WM_KANCHOKU_CHAR") == 0 ||
                   stricmp(OPT_outputMethod, "2")                == 0) {
            OPT_useWMIMECHAR = OUT_WMKANCHOKUCHAR;
        } else {
            warn("outputMethodに指定されている値が正しくありません。");
            OPT_useWMIMECHAR = OUT_WMCHAR; // とりあえず WM_CHAR に
        }
    }

    int OPT_enableWMKANCHOKUCHAR =
        GetPrivateProfileInt("kanchoku", "enablewmkanchokuchar", 0, iniFile);

    //<v127a - outputsleep>
    // outputSleep
    long OPT_outputSleep =
        GetPrivateProfileInt("kanchoku", "outputSleep", 0, iniFile);
    //</v127a - outputsleep>

    /* ---------------------------------------------------------------
     * 表示関連
     */
    // offHide
    int OPT_offHide = GetPrivateProfileInt("kanchoku", "offhide", 0, iniFile);

    // xLoc, yLoc
    int OPT_xLoc = GetPrivateProfileInt("kanchoku", "xloc", -1, iniFile);
    int OPT_yLoc = GetPrivateProfileInt("kanchoku", "yloc", -1, iniFile);

    /* ---------------------------------------------------------------
     * 個人的な趣味、および実験コード
     */
    // hardBS
    int OPT_hardBS = GetPrivateProfileInt("kanchoku", "hardbs", 0, iniFile);

    // useCtrlKkey
    int OPT_useCtrlKey = GetPrivateProfileInt("kanchoku", "usectrlkey", 0,
                                              iniFile);
    // useTTCode
    //<multishift>
    // !!! `useTTCode=1' option in "kanchoku.ini" is now obsolete.
    // !!! Use `#define prefix /*/*/*/*/' directive 
    // !!! in table file ("*.tbl") instead.
    //</multishift>
    int OPT_useTTCode = GetPrivateProfileInt("kanchoku", "usettcode", 0,
                                             iniFile);

    // win95
    int OPT_win95 = GetPrivateProfileInt("kanchoku", "win95", 0, iniFile);

    // followCaret
    int OPT_followCaret =
        GetPrivateProfileInt("kanchoku", "followcaret", 0, iniFile);

    /* ---------------------------------------------------------------
     */
    // キーボードファイルの読み込み
    int *vkey;
    vkey = new int[TC_NKEYS];
    FILE *fp = fopen(keyFile, "r");
    if (fp == 0) { error("キーボードファイルが開けません"); }
    for (int i = 0; i < TC_NKEYS; i++) {
        int vCode;
        fscanf(fp, "%x,", &vCode);
        vkey[i] = vCode;
    }
    fclose(fp);

    if (OPT_conjugationalMaze == 2) {
        tc_lt_key = tc_gt_key = -1;
        for (int i = 0; i < TC_NKEYS; i++) {
            if (vkey[i] == 0xbc) tc_lt_key = i;  //VK_OEM_COMMA
            if (vkey[i] == 0xbe) tc_gt_key = i;  //VK_OEM_PERIOD
        }
    }

    // テーブルファイルの読み込み
    is->open(tableFile/*, is->nocreate*/);
    if (is->fail()) { error("テーブルファイルが開けません"); }
    // パーズする
    Parser *parser = new Parser(is, hwnd);
    ControlBlock *table = parser->parse();
    is->close();
	is->clear();
    delete(parser);

    // 部首合成変換辞書の読み込み (使用する場合のみ)
    BushuDic *bushuDic = 0;
    if (bushuReady) {
        is->open(bushuFile/*,is->nocreate*/);
        if (is->fail()) {
            warn("部首合成変換辞書が開けませんでした。\n"
                 "部首合成変換の機能は使えません。");
            bushuReady = 0;
        } else {
            bushuDic = new BushuDic;
            bushuDic->readFile(is);
            is->close();
			is->clear();
        }
    }

    // 交ぜ書き辞書の読み込み (使用する場合のみ)
    MgTable *mgTable = 0;
    if (mazeReady) {
        is->open(mazegakiFile/*, is->nocreate*/);
        if (is->fail()) {
            warn("交ぜ書き変換辞書が開けませんでした。\n"
                 "交ぜ書き変換の機能は使えません。");
            mazeReady = 0;
        } else {
            mgTable = new MgTable(hwnd);
            mgTable->readFile(is);
            is->close();
			is->clear();
        }
    }

    //<v127a - gg>
    // 熟語ガイド辞書の読み込み (使用する場合のみ)
    GgDic *ggDic = 0;
    if (ggReady) {
        is->open(ggFile/*, is->nocreate*/);
        if (is->fail()) {
            warn("熟語ガイド辞書が開けませんでした。\n"
                 "熟語ガイドの機能は使えません。");
            ggReady = 0;
        } else {
            ggDic = new GgDic;
            ggDic->readFile(is);
            is->close();
			is->clear();
        }
    }
    //</v127a - gg>

    // T-Code 変換器を生成・初期化
    //<v127a - gg>
    //tc = new TCode(vkey, table, mgTable, bushuDic);
    tc = new TCode(vkey, table, mgTable, bushuDic, ggDic);
    //</v127a - gg>
    tc->OPT_hotKey = OPT_hotKey;
    //<OKA> support unmodified hot key
    tc->OPT_unmodifiedHotKey = OPT_unmodifiedHotKey;
    //</OKA>
    tc->bushuReady = bushuReady;
    tc->mazeReady = mazeReady;
    //<v127a - gg>
    tc->ggReady = ggReady;
    //</v127a - gg>

#define STRDUP(dst, src) do {               \
        dst = new char[strlen(src) + 1];    \
        strcpy(dst, src);                   \
    } while (0)
    STRDUP(tc->OPT_keyboard, keyFile);
    STRDUP(tc->OPT_tableFile, tableFile);
    if (bushuReady != 0) { STRDUP(tc->OPT_bushu, bushuFile); }
    if (mazeReady != 0)  { STRDUP(tc->OPT_mazegaki, mazegakiFile); }
    //<v127a - gg>
    if (ggReady != 0)    { STRDUP(tc->OPT_gg, ggFile); }
    //</v127a - gg>
    //<multishift2>
    {
        is->open(tableFile/*, is->nocreate*/);
        if (!is->fail()) {
            TCode::readDir(&tc->dirTable, is);
            is->close();
			is->clear();
        }
    }
    tc->stTable->setupPref(tc->dirTable[DIR_prefix]);
    //</multishift2>
    //<gg-defg>
    if (OPT_defg[0])     { STRDUP(tc->OPT_defg, OPT_defg); }
    //</gg-defg>
    //<multishift2>
    else if (tc->dirTable[DIR_defguide]) {
        STRDUP(tc->OPT_defg, tc->dirTable[DIR_defguide]);
    }
    //</multishift2>
    tc->OPT_prefixautoassign = 0;
    if (prefixautoassign[0]) {
        tc->OPT_prefixautoassign = new STROKE[strlen(prefixautoassign)+1];
        char *p = prefixautoassign;
        int i;
        for (i = 0; i < 255; i++) {
            int k, n = 1;
            if (sscanf(p, "-%d>%n", &k, &n) > 0) tc->OPT_prefixautoassign[i] = k;
            else if (sscanf(p, "-S%d>%n", &k, &n) > 0) tc->OPT_prefixautoassign[i] = TC_SHIFT(k);
            else break;
            p += n;
        }
        tc->OPT_prefixautoassign[i] = EOST;
    }
//#undef STRDUP
    //<record>
    if (OPT_record[0]) {
        tc->recordSetup(OPT_record);
    }
    if (OPT_stat[0]) {
        tc->statSetup(OPT_stat);
    }
    //</record>

	tc->OPT_bushuAlgo = OPT_bushuAlgo;
    tc->OPT_conjugationalMaze = OPT_conjugationalMaze;
    tc->OPT_shiftKana = OPT_shiftKana;
    //<v127a - shiftcheck>
    if (OPT_shiftKana)
        tc->checkShiftKana(tc->table);
    //</v127a - shiftcheck>
    tc->checkShiftSeq(tc->table);
    for (int i = 0; i < TC_NKEYS; i++) {
        if (tc->isShiftKana[i]) tc->isAnyShiftKana = true;
    }
    tc->OPT_maze2gg = OPT_maze2gg;
    tc->OPT_enableHankakuKana = OPT_enableHankakuKana;
    tc->OPT_useWMIMECHAR = OPT_useWMIMECHAR;
    //<v127a - outputsleep>
    tc->OPT_outputSleep = OPT_outputSleep;
    //</v127a - outputsleep>
    tc->OPT_offHide = OPT_offHide;
    tc->OPT_xLoc = OPT_xLoc;
    tc->OPT_yLoc = OPT_yLoc;
    tc->OPT_hardBS = OPT_hardBS;
    tc->OPT_useCtrlKey = OPT_useCtrlKey;
    //<multishift>
    //tc->OPT_useTTCode = OPT_useTTCode;
    if (OPT_useTTCode) {
        tc->stTable->setupPref("/▲/26,23/▲○/▲/:/▽/23,26/▽○/▽/");
    }
    //</multishift>
    tc->OPT_win95 = OPT_win95;
    tc->OPT_followCaret = OPT_followCaret;

    WM_KANCHOKU_CHAR = 0;
    lpfnMySetHook = NULL;
    lpfnMyEndHook = NULL;
    if (OPT_enableWMKANCHOKUCHAR) {
        WM_KANCHOKU_CHAR = RegisterWindowMessage("WM_KANCHOKU_CHAR");
        hKanCharDLL = LoadLibrary("kanchar.dll");
        if (hKanCharDLL) {
            lpfnMySetHook = (HHOOK (*)(void))GetProcAddress(hKanCharDLL,
                "_MySetHook");
            lpfnMyEndHook = (int (*)(void))GetProcAddress(hKanCharDLL,
                "_MyEndHook");
        }
        if (lpfnMySetHook) {
            hNextHook = lpfnMySetHook();
            SetProp(hwnd, "KanchokuWin_KanCharDLL_NextHook",
                (HANDLE)hNextHook);
        }
        if (!hKanCharDLL) {
            warn("kanchar.dllが読み込めません。");
        } else if (!lpfnMySetHook || !lpfnMyEndHook) {
            warn("GetProcAddressに失敗しました。");
        }
    }

    // 出力先ウィンドウごとの設定の読み込み
    readTargetWindowSetting(iniFile);

    // 待機状態に
    tc->mode = TCode::OFF;
    tc->preBuffer->clear();
    tc->postBuffer->clear();
    tc->helpBuffer->clear();
    tc->helpOffset = 0;
    tc->helpMode = 0;
    tc->hirakataMode = 0;
    tc->hanzenMode = 0;
    tc->punctMode = 0;
}

// -------------------------------------------------------------------
// 出力先ウィンドウごとの設定の読み込み
void TableWindow::readTargetWindowSetting(char *iniFile) {
    char buf[BUFSIZ] = "";
    if (GetPrivateProfileSectionNames(buf, sizeof(buf), iniFile) >= sizeof(buf) - 2) {
        warn("[セクション名]リストの読み込み失敗(内部バッファ長不足)。\n"
             "セクション名を短くしてみてください");
    }
    char className[BUFSIZ];
    char outputMethod[BUFSIZ];
    char *p;
    for (p = buf; *p != '\0' && p < buf + sizeof(buf); p += strlen(p) + 1) {
        if (stricmp(p, "kanchoku") == 0 || stricmp(p, "kansaku") == 0) {
            continue;
        }
        className[0] = '\0';
        GetPrivateProfileString(p, "className", "",
                                className, sizeof(className), iniFile);
        if (className[0] == '\0') {
            continue;
        }
        outputMethod[0] = '\0';
        GetPrivateProfileString(p, "outputMethod", "",
                                outputMethod, sizeof(outputMethod), iniFile);
        int m = tc->OPT_useWMIMECHAR;
        if (stricmp(outputMethod, "WMCHAR")  == 0 ||
            stricmp(outputMethod, "WM_CHAR") == 0 ||
            stricmp(outputMethod, "0")       == 0) {
            m = OUT_WMCHAR;
        } else if (stricmp(outputMethod, "WMIMECHAR")   == 0 ||
                   stricmp(outputMethod, "WM_IME_CHAR") == 0 ||
                   stricmp(outputMethod, "1")           == 0) {
            m = OUT_WMIMECHAR;
        } else if (stricmp(outputMethod, "WMKANCHOKUCHAR")   == 0 ||
                   stricmp(outputMethod, "WM_KANCHOKU_CHAR") == 0 ||
                   stricmp(outputMethod, "2")                == 0) {
            m = OUT_WMKANCHOKUCHAR;
        } else {
            warn("outputMethodに指定されている値が正しくありません。");
            continue;
        }
        char *q;
        STRDUP(q, className);
        tc->OPT_outputMethodMap[q] = m;
    }
}

// -------------------------------------------------------------------
// 出力
void TableWindow::output() {
    if (tc->isComplete() == 0) { return; }

    int len = tc->preBuffer->length();
    if(len == 0) { return; }

    // 入力フォーカスを持つウィンドウを取得
    HWND targetWin = GetForegroundWindow();
    DWORD targetThread = GetWindowThreadProcessId(targetWin, NULL);
    DWORD selfThread = GetCurrentThreadId();
    AttachThreadInput(selfThread, targetThread, TRUE);
    HWND activeWin = GetFocus();

    for (int i = 0; i < len; i++) {
        MOJI m = tc->preBuffer->moji(i);
        int h = MOJI2H(m);
        int l = MOJI2L(m);
        switch (mojitype(m)) {
        case MOJI_VKEY:
            //<v127c>
            // IEの1行フォームの中で-とか\とかの直接入力 [練習スレ2:616]
            //PostMessage(activeWin, WM_KEYDOWN, l, 0);
            PostMessage(activeWin, WM_KEYDOWN, l, 1);
            //</v127c>
            Sleep(tc->OPT_outputSleep); // Firefox1.5でBSで2文字消えるのを回避
            PostMessage(activeWin, WM_KEYUP, l, 0xc0000001);
            //<v127a - outputSleep>
            //Sleep(0);               // 先にVK_BACKを処理してほしい
            Sleep(tc->OPT_outputSleep); // 先にVK_BACKを処理してほしい
            //</v127a - outputSleep>
            // XXX
            if (l == VK_BACK || l == 'H') { tc->postBuffer->pop(); }
            else                          { tc->postBuffer->pushHard(m); }
            break;

        case MOJI_ZENKAKU:
            switch (getOutputMethod(activeWin)) {
            case OUT_WMKANCHOKUCHAR:
                PostMessage(activeWin, WM_KANCHOKU_CHAR,
                            (((unsigned char)h << 8 ) | (unsigned char)l), 1);
                break;
            case OUT_WMIMECHAR:
                PostMessage(activeWin, WM_IME_CHAR,
                            (((unsigned char)h << 8 ) | (unsigned char)l), 1);
                break;
            case OUT_WMCHAR:
                PostMessage(activeWin, WM_CHAR, (unsigned char)h, 1);
                PostMessage(activeWin, WM_CHAR, (unsigned char)l, 1);
                break;
            }
            tc->postBuffer->pushHard(m);
            break;

        case MOJI_HANKANA:      // XXX
            switch (getOutputMethod(activeWin)) {
            case OUT_WMKANCHOKUCHAR:
                PostMessage(activeWin, WM_KANCHOKU_CHAR, (unsigned char)l, 1);
                break;
            case OUT_WMIMECHAR:
                PostMessage(activeWin, WM_IME_CHAR, (unsigned char)l, 1);
                break;
            case OUT_WMCHAR:
                PostMessage(activeWin, WM_CHAR, (unsigned char)l, 1);
                break;
            }
            tc->postBuffer->pushHard(m);
            break;

        case MOJI_ASCII:
            PostMessage(activeWin, WM_CHAR, (unsigned char)l, 1);
            tc->postBuffer->pushHard(m);
            break;

        default: // ここには来ないはず
            break;
        } // switch mojitype(m)
    } // for i
    tc->preBuffer->clear();

    // カーソルに追従
    // thanx to 816 in 『【原理】T-Code連習マラソン【主義】』
    // <http://pc.2ch.net/test/read.cgi/unix/1014523030/>
    POINT ptCaret;
    if (tc->OPT_followCaret
        && GetCaretPos(&ptCaret)
        && (ptCaret.x || ptCaret.y)
        && ClientToScreen(activeWin, &ptCaret)) {
        // 外枠の大きさを取得
        RECT winRect;
        GetWindowRect(hwnd, &winRect);
        // 中身の大きさを取得
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        // 外枠と中身の差を取得
        int dX = (winRect.right - winRect.left)
            - (clientRect.right - clientRect.left);
        int dY = (winRect.bottom - winRect.top)
            - (clientRect.bottom - clientRect.top);
        int sX = ptCaret.x - (winRect.right - winRect.left) / 2;
        int sY = ptCaret.y + (winRect.bottom - winRect.top) / 5;
        MoveWindow(hwnd, sX, sY, WIDTH + dX, HEIGHT + dY, TRUE);
    }

    // スレッドを切り離す
    AttachThreadInput(selfThread, targetThread, FALSE);
}

// -------------------------------------------------------------------
// 指定したウィンドウに対するoutputMethodを取得
int TableWindow::getOutputMethod(HWND hWnd) {
    int om = tc->OPT_useWMIMECHAR;
    char s[BUFSIZ];
    GetClassName(hWnd, s, sizeof(s));
    if (tc->OPT_outputMethodMap.find(s) != tc->OPT_outputMethodMap.end()) {
        om = tc->OPT_outputMethodMap[s];
    }
    return om;
}

// -------------------------------------------------------------------
// オリジナル版の TableWindow::outputString() の中のコメント
// -------------------------------------------------------------------
//   とりあえずimmを使うやり方は失敗したが、
//   もしIME化した場合は、この辺が活きるかも。
//    HIMC hImc = ImmGetContext(activeWin);
//    HIMC hImc = (HIMC)GetWindowLong(activeWin, 0); //0 = IMMGWL_IMC
//    HIMC hImc = ImmCreateContext();
//    if(hImc == NULL) {
//  exit(1);
//    }
//    ImmSetCompositionString(0, SCS_SETSTR,pt,strlen(pt), pt, strlen(pt));
//    HIMC oldImc = ImmAssociateContext(activeWin,hImc);
//    if(oldImc == NULL) {
//  exit(1);
//    }
//    ImmSetCompositionString(hImc, SCS_SETSTR,pt,strlen(pt), pt, strlen(pt));
//    if(oldImc == NULL) {
//  exit(1);
//    }
//    PostMessage(activeWin, WM_IME_COMPOSITION, 0, GCS_RESULTSTR);
// -------------------------------------------------------------------

/* -------------------------------------------------------------------
 * 仮想鍵盤の描画
 */

// drawFrame*() とか、呼ばれるたびに描画するんじゃなく
// bitmap に保存しといと転送するようにすべきかも

// -------------------------------------------------------------------
// 仮想鍵盤の枠 (OFF 時)
void TableWindow::drawFrameOFF(HDC hdc) {
    HBRUSH brK1 = CreateSolidBrush(COL_OFF_K1);
    HBRUSH brM1 = CreateSolidBrush(COL_OFF_M1);
    HPEN pnLN = CreatePen(PS_SOLID, 1, COL_OFF_LN);
    HPEN pnM1 = CreatePen(PS_SOLID, 1, COL_OFF_M1);
    int x, y;

    // 保存
    HGDIOBJ pnSave = SelectObject(hdc, pnLN);
    HGDIOBJ brSave = SelectObject(hdc, brM1);

    // 外枠
    SelectObject(hdc, pnM1);
    SelectObject(hdc, brM1);
    Rectangle(hdc, 0, 0, WIDTH, HEIGHT);

    // 柱
    SelectObject(hdc, pnLN);
    SelectObject(hdc, brM1);
    x = MARGIN_SIZE + BLOCK_SIZE * 5;
    y = MARGIN_SIZE;
    Rectangle(hdc, x, y, x + BLOCK_SIZE + 1, y + BLOCK_SIZE * 4 + 1);

    // キー
    for (int j = 0; j < 5; j++) {
        y = MARGIN_SIZE + BLOCK_SIZE * j;
        for (int i = 0; i < 10; i++) {
            x = MARGIN_SIZE + BLOCK_SIZE * i;
            if (j == 4) { x += BLOCK_SIZE / 2; }
            else if (4 < i) { x += BLOCK_SIZE; }
            SelectObject(hdc, pnLN);
            SelectObject(hdc, brK1);
            Rectangle(hdc, x, y, x + BLOCK_SIZE + 1, y + BLOCK_SIZE + 1);
        }
    }

    // 後始末
    SelectObject(hdc, brSave);
    SelectObject(hdc, pnSave);
    DeleteObject(brK1);
    DeleteObject(brM1);
    DeleteObject(pnLN);
    DeleteObject(pnM1);
}

// -------------------------------------------------------------------
// 仮想鍵盤の枠 (50 鍵)
void TableWindow::drawFrame50(HDC hdc) {
    HBRUSH brK1 = CreateSolidBrush(COL_ON_K1);
    HBRUSH brM1 = CreateSolidBrush(COL_ON_M1);
    HPEN pnLN = CreatePen(PS_SOLID, 1, COL_ON_LN);
    HPEN pnK2 = CreatePen(PS_SOLID, 1, COL_ON_K2);
    HPEN pnM1 = CreatePen(PS_SOLID, 1, COL_ON_M1);
    HPEN pnM2 = CreatePen(PS_SOLID, 1, COL_ON_M2);
    int x, y;

    // 保存
    HGDIOBJ pnSave = SelectObject(hdc, pnLN);
    HGDIOBJ brSave = SelectObject(hdc, brM1);

    // 外枠
    SelectObject(hdc, pnM1);
    SelectObject(hdc, brM1);
    Rectangle(hdc, 0, 0, WIDTH, HEIGHT);
    // ハイライト
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    x = MARGIN_SIZE; y = MARGIN_SIZE + BLOCK_SIZE * 4;
    MoveToEx(hdc, x, y + 1, NULL);
    LineTo(hdc, x + BLOCK_SIZE / 2 - 1, y + 1);
    x = MARGIN_SIZE + BLOCK_SIZE / 2; y = MARGIN_SIZE + BLOCK_SIZE * 5;
    MoveToEx(hdc, x, y + 1, NULL);
    x += BLOCK_SIZE * 10;
    LineTo(hdc, x + 1, y + 1);
    y = MARGIN_SIZE + BLOCK_SIZE * 4;
    LineTo(hdc, x + 1, y + 1);
    x = MARGIN_SIZE + BLOCK_SIZE * 11;
    LineTo(hdc, x + 1, y + 1);
    y = MARGIN_SIZE;
    LineTo(hdc, x + 1, y - 1);
    // 影
    SelectObject(hdc, pnM2);
    x = MARGIN_SIZE + BLOCK_SIZE * 11; y = MARGIN_SIZE;
    MoveToEx(hdc, x, y - 1, NULL);
    x = MARGIN_SIZE;
    LineTo(hdc, x - 1, y - 1);
    y = MARGIN_SIZE + BLOCK_SIZE * 4;
    LineTo(hdc, x - 1, y + 1);
    x = MARGIN_SIZE + BLOCK_SIZE / 2; y = MARGIN_SIZE + BLOCK_SIZE * 4;
    MoveToEx(hdc, x - 1, y + 2, NULL);
    y = MARGIN_SIZE + BLOCK_SIZE * 5;
    LineTo(hdc, x - 1, y + 1);

    // 柱
    SelectObject(hdc, pnLN);
    SelectObject(hdc, brM1);
    x = MARGIN_SIZE + BLOCK_SIZE * 5;
    y = MARGIN_SIZE;
    Rectangle(hdc, x, y, x + BLOCK_SIZE + 1, y + BLOCK_SIZE * 4 + 1);
    // ハイライト
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    MoveToEx(hdc, x + BLOCK_SIZE - 2, y + 1, NULL);
    LineTo(hdc, x + 1, y + 1);
    LineTo(hdc, x + 1, y + BLOCK_SIZE * 4 - 1);
    // 影
    SelectObject(hdc, pnM2);
    MoveToEx(hdc, x + BLOCK_SIZE - 1, y + 2, NULL);
    LineTo(hdc, x + BLOCK_SIZE - 1, y + BLOCK_SIZE * 4 - 1);
    LineTo(hdc, x + 1, y + BLOCK_SIZE * 4 - 1);

    // キー
    for (int j = 0; j < 5; j++) {
        y = MARGIN_SIZE + BLOCK_SIZE * j;
        for (int i = 0; i < 10; i++) {
            x = MARGIN_SIZE + BLOCK_SIZE * i;
            if (j == 4) { x += BLOCK_SIZE / 2; }
            else if (4 < i) { x += BLOCK_SIZE; }
            SelectObject(hdc, pnLN);
            SelectObject(hdc, brK1);
            Rectangle(hdc, x, y, x + BLOCK_SIZE + 1, y + BLOCK_SIZE + 1);
            // ハイライト
            SelectObject(hdc, GetStockObject(WHITE_PEN));
            MoveToEx(hdc, x + BLOCK_SIZE - 2, y + 1, NULL);
            LineTo(hdc, x + 1, y + 1);
            LineTo(hdc, x + 1, y + BLOCK_SIZE - 1);
            // 影
            SelectObject(hdc, pnK2);
            MoveToEx(hdc, x + BLOCK_SIZE - 1, y + 2, NULL);
            LineTo(hdc, x + BLOCK_SIZE - 1, y + BLOCK_SIZE - 1);
            LineTo(hdc, x + 1, y + BLOCK_SIZE - 1);
        }
    }

    // 後始末
    SelectObject(hdc, brSave);
    SelectObject(hdc, pnSave);
    DeleteObject(brK1);
    DeleteObject(brM1);
    DeleteObject(pnLN);
    DeleteObject(pnK2);
    DeleteObject(pnM1);
    DeleteObject(pnM2);
}

// -------------------------------------------------------------------
// 仮想鍵盤の枠 (10 鍵)
void TableWindow::drawFrame10(HDC hdc) {
    HBRUSH brK1 = CreateSolidBrush(COL_ON_K1);
    HBRUSH brM1 = CreateSolidBrush(COL_ON_M1);
    HPEN pnLN = CreatePen(PS_SOLID, 1, COL_ON_LN);
    HPEN pnK2 = CreatePen(PS_SOLID, 1, COL_ON_K2);
    HPEN pnM1 = CreatePen(PS_SOLID, 1, COL_ON_M1);
    HPEN pnM2 = CreatePen(PS_SOLID, 1, COL_ON_M2);
    int x, y;

    // 保存
    HGDIOBJ pnSave = SelectObject(hdc, pnLN);
    HGDIOBJ brSave = SelectObject(hdc, brM1);

    // 外枠
    SelectObject(hdc, pnM1);
    SelectObject(hdc, brM1);
    Rectangle(hdc, 0, 0, WIDTH, HEIGHT);
    // ハイライト
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    x = MARGIN_SIZE; y = MARGIN_SIZE + BLOCK_SIZE * 5;
    MoveToEx(hdc, x, y + 1, NULL);
    x = MARGIN_SIZE + BLOCK_SIZE * 11;
    LineTo(hdc, x + 1, y + 1);
    y = MARGIN_SIZE;
    LineTo(hdc, x + 1, y - 1);
    // 影
    SelectObject(hdc, pnM2);
    x = MARGIN_SIZE + BLOCK_SIZE * 11; y = MARGIN_SIZE;
    MoveToEx(hdc, x, y - 1, NULL);
    x = MARGIN_SIZE;
    LineTo(hdc, x - 1, y - 1);
    y = MARGIN_SIZE + BLOCK_SIZE * 5;
    LineTo(hdc, x - 1, y + 1);

    // 柱
    SelectObject(hdc, pnLN);
    SelectObject(hdc, brM1);
    x = MARGIN_SIZE + BLOCK_SIZE * 5;
    y = MARGIN_SIZE;
    Rectangle(hdc, x, y, x + BLOCK_SIZE + 1, y + BLOCK_SIZE * 5 + 1);
    // ハイライト
    SelectObject(hdc, GetStockObject(WHITE_PEN));
    MoveToEx(hdc, x + BLOCK_SIZE - 2, y + 1, NULL);
    LineTo(hdc, x + 1, y + 1);
    LineTo(hdc, x + 1, y + BLOCK_SIZE * 5 - 1);
    // 影
    SelectObject(hdc, pnM2);
    MoveToEx(hdc, x + BLOCK_SIZE - 1, y + 2, NULL);
    LineTo(hdc, x + BLOCK_SIZE - 1, y + BLOCK_SIZE * 5 - 1);
    LineTo(hdc, x + 1, y + BLOCK_SIZE * 5 - 1);

    // キー
    for (int i = 0; i < 10; i++) {
        x = MARGIN_SIZE + BLOCK_SIZE * i;
        y = MARGIN_SIZE;
        if (4 < i) { x += BLOCK_SIZE; }
        SelectObject(hdc, pnLN);
        SelectObject(hdc, brK1);
        Rectangle(hdc, x, y, x + BLOCK_SIZE + 1, y + BLOCK_SIZE * 5 + 1);
        // ハイライト
        SelectObject(hdc, GetStockObject(WHITE_PEN));
        MoveToEx(hdc, x + BLOCK_SIZE - 2, y + 1, NULL);
        LineTo(hdc, x + 1, y + 1);
        LineTo(hdc, x + 1, y + BLOCK_SIZE * 5 - 1);
        // 影
        SelectObject(hdc, pnK2);
        MoveToEx(hdc, x + BLOCK_SIZE - 1, y + 2, NULL);
        LineTo(hdc, x + BLOCK_SIZE - 1, y + BLOCK_SIZE * 5 - 1);
        LineTo(hdc, x + 1, y + BLOCK_SIZE * 5 - 1);
    }

    // 後始末
    SelectObject(hdc, brSave);
    SelectObject(hdc, pnSave);
    DeleteObject(brK1);
    DeleteObject(brM1);
    DeleteObject(pnLN);
    DeleteObject(pnK2);
    DeleteObject(pnM1);
    DeleteObject(pnM2);
}

// -------------------------------------------------------------------
// 仮想鍵盤のキー (50 鍵)
void TableWindow::drawVKB50(HDC hdc, bool isWithBothSide) {
    HBRUSH brR = CreateSolidBrush(COL_LT_RED);
    HBRUSH brG = CreateSolidBrush(COL_LT_GREEN);
    HBRUSH brB = CreateSolidBrush(COL_LT_BLUE);
    HBRUSH brC = CreateSolidBrush(COL_LT_CYAN);
    HBRUSH brY = CreateSolidBrush(COL_LT_YELLOW);
    HBRUSH brL = CreateSolidBrush(COL_LT_GRAY);
    HBRUSH brW = CreateSolidBrush(COL_ON_K1);
    HBRUSH brSP = CreateSolidBrush(COL_DK_CYAN);
    HBRUSH brGG = CreateSolidBrush(COL_DK_MAGENTA);
    HBRUSH brNO = CreateSolidBrush(COL_BLACK);

    HGDIOBJ brSave = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    HGDIOBJ pnSave = SelectObject(hdc, GetStockObject(NULL_PEN));
    HGDIOBJ fnSave = SelectObject(hdc, hFont);

    SetBkMode(hdc, TRANSPARENT);
    for (int y = 0; y < 5; y++) {
        int py = MARGIN_SIZE + BLOCK_SIZE * y;
        for (int x = 0; x < 10; x++) {
            int k = y * 10 + x;
            if (TC_NKEYS <= k) { goto END; }

            int px = MARGIN_SIZE + BLOCK_SIZE * x;
            if (y == 4) { px += BLOCK_SIZE / 2; }
            else if (5 <= x) { px += BLOCK_SIZE; }

            switch (tc->vkbBG[k]) {
            case TC_BG_ST1: SelectObject(hdc, brR); break;
            case TC_BG_ST2: SelectObject(hdc, brG); break;
            case TC_BG_ST3: SelectObject(hdc, brY); break;
            case TC_BG_STF: SelectObject(hdc, brL); break;
            case TC_BG_STW: SelectObject(hdc, brB); break;
            case TC_BG_STX: SelectObject(hdc, brC); break;
            //<multishift>
            //case TC_BG_ST1R: SelectObject(hdc, brR); break;
            //case TC_BG_ST2R: SelectObject(hdc, brG); break;
            //case TC_BG_STWR: SelectObject(hdc, brB); break;
            //case TC_BG_ST1L: SelectObject(hdc, brR); break;
            //case TC_BG_ST2L: SelectObject(hdc, brG); break;
            //case TC_BG_STWL: SelectObject(hdc, brB); break;
            //</multishift>
            default: SelectObject(hdc, GetStockObject(NULL_BRUSH)); break;
            }
            Rectangle(hdc, px + 2, py + 2, px + BLOCK_SIZE, py + BLOCK_SIZE);

            if (tc->vkbCorner[k] & TC_MK_SH1) {
                SelectObject(hdc, brW);
                POINT tri[3];
                tri[0].x = tri[1].x = px + 2;
                tri[1].y = tri[2].y = py + BLOCK_SIZE-1;
                tri[2].x = px + 2 + 8;
                tri[0].y = py + BLOCK_SIZE-1 - 8;
                Polygon(hdc, tri, 3);
            }
            if (isWithBothSide && isShift && tc->vkbFace[k]) {
                switch (tc->vkbFG[k]) {
                case TC_FG_SPECIAL: SelectObject(hdc, brSP); break;
                case TC_FG_GG: SelectObject(hdc, brGG); break;
                case TC_FG_NORMAL: SelectObject(hdc, brNO); break;
                }
                POINT tri[3];
                tri[0].x = tri[1].x = px + 2;
                tri[1].y = tri[2].y = py + BLOCK_SIZE-1;
                tri[2].x = px + 2 + 6;
                tri[0].y = py + BLOCK_SIZE-1 - 6;
                Polygon(hdc, tri, 3);
            }
            if (tc->vkbCorner[k] & TC_MK_SH2) {
                SelectObject(hdc, brW);
                POINT tri[3];
                tri[2].x = tri[1].x = px + BLOCK_SIZE-1;
                tri[1].y = tri[0].y = py + BLOCK_SIZE-1;
                tri[0].x = px + BLOCK_SIZE-1 - 8;
                tri[2].y = py + BLOCK_SIZE-1 - 8;
                Polygon(hdc, tri, 3);
            }
            if (tc->vkbCorner[k] & TC_MK_SH3) {
                SelectObject(hdc, brW);
                POINT tri[3];
                tri[0].x = tri[1].x = px + BLOCK_SIZE-1;
                tri[1].y = tri[2].y = py + 2;
                tri[2].x = px + BLOCK_SIZE-1 - 8;
                tri[0].y = py + 2 + 8;
                Polygon(hdc, tri, 3);
            }
            if (isWithBothSide && !isShift && tc->vkbFace[TC_SHIFT(k)]) {
                switch (tc->vkbFG[TC_SHIFT(k)]) {
                case TC_FG_SPECIAL: SelectObject(hdc, brSP); break;
                case TC_FG_GG: SelectObject(hdc, brGG); break;
                case TC_FG_NORMAL: SelectObject(hdc, brNO); break;
                }
                POINT tri[3];
                tri[0].x = tri[1].x = px + BLOCK_SIZE-1;
                tri[1].y = tri[2].y = py + 2;
                tri[2].x = px + BLOCK_SIZE-1 - 6;
                tri[0].y = py + 2 + 6;
                Polygon(hdc, tri, 3);
            }

            switch (tc->vkbFG[isWithBothSide&&isShift?TC_SHIFT(k):k]) {
            case TC_FG_SPECIAL: SetTextColor(hdc, COL_DK_CYAN); break;
            case TC_FG_STROKE:  SetTextColor(hdc, COL_DK_CYAN); break;
            //<v127a - gg>
            case TC_FG_GG:      SetTextColor(hdc, COL_DK_MAGENTA); break;
            //</v127a - gg>
            case TC_FG_NORMAL:
            default:            SetTextColor(hdc, COL_BLACK); break;
            }
            char *s = tc->vkbFace[isWithBothSide&&isShift?TC_SHIFT(k):k];
            if (s && *s) {
                int dx = tc->OPT_win95 ? 0 : 1;
                if (strlen(s) <= 1) {
                    TextOut(hdc, px + 3 + dx + (CHAR_SIZE / 4), py + 3, s, 1);
                } else {
                    TextOut(hdc, px + 3 + dx, py + 3, s, 2);
                }
            }
        }
    }
 END:
    SelectObject(hdc, brSave);
    SelectObject(hdc, pnSave);
    SelectObject(hdc, fnSave);

    DeleteObject(brR);
    DeleteObject(brG);
    DeleteObject(brB);
    DeleteObject(brC);
    DeleteObject(brY);
    DeleteObject(brL);
    DeleteObject(brW);
    DeleteObject(brSP);
    DeleteObject(brGG);
    DeleteObject(brNO);
}

// -------------------------------------------------------------------
// 仮想鍵盤のキー (10 鍵)
void TableWindow::drawVKB10(HDC hdc) {
    HBRUSH br = CreateSolidBrush(COL_LT_CYAN);

    HGDIOBJ brSave = SelectObject(hdc, br);
    HGDIOBJ pnSave = SelectObject(hdc, GetStockObject(NULL_PEN));
    HGDIOBJ fnSave = SelectObject(hdc, hFont);

    for (int x = 0; x < 10; x++) {
        int k = 20 + x;
        int px = MARGIN_SIZE + BLOCK_SIZE * x;
        int py = MARGIN_SIZE;
        if (5 <= x) { px += BLOCK_SIZE; }
        if (tc->vkbBG[k] == TC_BG_HISTPTR) {
            Rectangle(hdc, px + 2, py + 2,
                      px + BLOCK_SIZE, py + BLOCK_SIZE * 5);
        }
        if (tc->vkbFG[k] == TC_FG_HISTREF) {
            SetTextColor(hdc, COL_RED);
        } else {
            SetTextColor(hdc, COL_BLACK);
        }
        SetBkMode(hdc, TRANSPARENT);

        char *s = tc->vkbFace[k];
        int dx = tc->OPT_win95 ? 0 : 1;
        for (int y = 0; s && *s && y < 6; y++) {
            py = MARGIN_SIZE + (CHAR_SIZE + 1) * y + 5;
            if (IS_ZENKAKU(*s)) {
                TextOut(hdc, px + 3 + dx, py, s, 2); s += 2;
            } else {
                TextOut(hdc, px + 3 + dx + (CHAR_SIZE / 4), py, s, 1); s += 1;
            }
        }
    }

// END:
    SelectObject(hdc, brSave);
    SelectObject(hdc, pnSave);
    SelectObject(hdc, fnSave);

    DeleteObject(br);
}

// -------------------------------------------------------------------
// ミニバッファ
void TableWindow::drawMiniBuffer(HDC hdc, int height, COLORREF col,
                                 MojiBuffer *mb) {
    HBRUSH br = CreateSolidBrush(col);

    HGDIOBJ brSave = SelectObject(hdc, br);
    HGDIOBJ pnSave = SelectObject(hdc, GetStockObject(NULL_PEN));
    HGDIOBJ fnSave = SelectObject(hdc, hLFont);

    // background
    int px = MARGIN_SIZE + BLOCK_SIZE * 5;
    int py = MARGIN_SIZE;
    Rectangle(hdc, px + 2, py + 2, px + BLOCK_SIZE, py + BLOCK_SIZE * height);

    // text
    px = MARGIN_SIZE + BLOCK_SIZE * 5 + 1;
    py = MARGIN_SIZE + BLOCK_SIZE * 0 + 1;
    SetTextColor(hdc, COL_BLACK);
    SetBkMode(hdc, TRANSPARENT);

    int offset = 0;
    if (height < mb->length()) { offset = mb->length() - height; }
    for (int y = 0; y < height && offset < mb->length();
         y++, offset++, py += BLOCK_SIZE) {
        char s[3]; s[0] = 0;
        MOJI m = mb->moji(offset);
        int dx = tc->OPT_win95 ? 0 : 1;
        switch (mojitype(m)) {
        case MOJI_SPECIAL:
            SetTextColor(hdc, COL_DK_CYAN);
            if (m == MOJI_BUSHU) {
                TextOut(hdc, px + dx, py + 1, "◆", 2);
            } else if (m == MOJI_MAZE) {
                TextOut(hdc, px + dx, py + 1, "◇", 2);
            }
            break;
        case MOJI_ZENKAKU:
            moji2strcat(s, m);
            SetTextColor(hdc, COL_BLACK);
            TextOut(hdc, px + dx, py + 1, s, 2);
            break;
        case MOJI_HANKANA:
        case MOJI_ASCII:
            moji2strcat(s, m);
            SetTextColor(hdc, COL_BLACK);
            TextOut(hdc, px + dx + (LARGE_CHAR_SIZE / 4), py + 1, s, 1);
            break;
        default:
            // XXX VKey とか
            SetTextColor(hdc, COL_DK_CYAN);
            TextOut(hdc, px + dx, py + 1, "・", 2);
            break;
        }
    }
    // under construction

    SelectObject(hdc, brSave);
    SelectObject(hdc, pnSave);
    SelectObject(hdc, fnSave);

    DeleteObject(br);
}

/* -------------------------------------------------------------------
 * エラー処理
 */

// -------------------------------------------------------------------
// エラーを表示し、終了
void TableWindow::error(char *mes) {
    MessageBoxEx(hwnd, mes, "漢直窓 - エラー",
                 MB_OK | MB_ICONERROR, LANG_JAPANESE);
    PostQuitMessage(0);
}

// -------------------------------------------------------------------
// 警告を表示するが、継続する
void TableWindow::warn(char *mes) {
    // アイコンの選択は正しいだろうか?
    MessageBoxEx(hwnd, mes, "漢直窓 - 警告",
                 MB_OK | MB_ICONEXCLAMATION, LANG_JAPANESE);
}

// -------------------------------------------------------------------
