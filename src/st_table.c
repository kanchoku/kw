#ifdef _MSC_VER //<OKA>
#pragma warning(disable:4786)
#define for if(0);else for
#endif          //</OKA>
#include "st_table.h"
#include "tc.h"
#include "debug.h"
// -------------------------------------------------------------------

// ストローク列 strk の長さを返す (EOST は含まない)
int strokelen(const STROKE *st) {
    int i;
    for (i = 0; *(st + i) != EOST; i++) {}
    return i;
}

// ストローク列のコピー (EOST も含む)
void strokecpy(STROKE *dst, const STROKE *src) {
    while (1) {
        *dst = *src;
        if (*src == EOST) { break; }
        dst++; src++;
    }
}

// -------------------------------------------------------------------
// コンストラクタとデストラクタ

StTable::StTable(Block *rootBlock) {
    stMap = new StMap();
    stroke = 0;

    init(rootBlock);

    //<multishift>
    // 文字ヘルプ用の記号の初期化
    strcpy(defmkSt[0], TC_MK_ST1);
    strcpy(defmkSt[1], TC_MK_ST2);
    strcpy(defmkSt[2], TC_MK_ST3);
    strcpy(defmkSt[3], TC_MK_STF);
    strcpy(defmkSt[4], TC_MK_STW);
    strcpy(defmkSt[5], TC_MK_STX);
    strcpy(defmkTbl, "");
    nprefs = 0;
    baseStroke = 0;
    //</multishift>
}

StTable::~StTable() {
    delete(stMap);
}

// -------------------------------------------------------------------
// 初期化 : 逆引きテーブルを作る

void StTable::init(Block *rootBlock) {
    // 空のストローク列を一時的に生成
    STROKE *nullSt = new STROKE(EOST);
    initSub(rootBlock, nullSt);
    delete nullSt;
}

void StTable::initSub(Block *currentBlock, STROKE *currentSt) {
    char *str, *check;
    MOJI moji;
    int len;
    STROKE *st;

    // ブランクの場合は何もせず返る
    if (currentBlock == 0) { return; }

    switch (currentBlock->kind()) {
    case STRING_BLOCK:      // 文字列ブロックの場合
        // 1 文字の定義であれば登録する
        str = ((StringBlock *)currentBlock)->str;
        moji = str2moji(str, &check);
        if (*str == 0 || *check != 0) { return; }
        // ストローク列を複製
        len = strokelen(currentSt);
        st = new STROKE[len + 1];
        memcpy(st, currentSt, len + 1);
        (*stMap)[moji] = st;
        return;

    case CONTROL_BLOCK:     // コントロールブロックの場合
        // ネストに入る
        for (int key = 0; key < TC_NKEYS*2; key++) {
            Block *nextBlock = (((ControlBlock *)currentBlock)->block)[key];
            // 新しいストローク列を一時的に生成
            len = strokelen(currentSt);
            st = new STROKE[len + 2];
            strokecpy(st, currentSt);
            st[len] = key; st[len + 1] = EOST;
            // 再帰
            initSub(nextBlock, st);
            delete [] st;
        }
        return;

    case SPECIAL_BLOCK:     // 特殊ブロックの場合 : 何もしない
    default:
        return;
    }
}

// -------------------------------------------------------------------
// 検索

int StTable::look(MOJI moji) {
    // マップ内にあるかチェック
    if (stMap->find(moji) == stMap->end()) {
        return 0;
    }

    // ストローク列取り出し
    stroke = (*stMap)[moji];

    //<multishift>
    matchPref(stroke, &baseStroke, &mkTbl, &mkSt);
    //</multishift>

    return 1;
}

//<multishift>
// -------------------------------------------------------------------
// 多段シフト

void StTable::setupPref(const char *src) {
    char *spec;
    char *ent;
    char smkTbl[256], sst[256], smkSt1[256], smkSt2[256]; // XXX
    int k, n, i;
    char *p;
    MOJI m;

    nprefs = 0;
    if (src == 0) { return; }

    spec = new char[strlen(src) + 1];
    strcpy(spec, src);

    for (ent = strtok(spec, ":");
         // (Shift-JIS の 2 バイト目が「/」「:」になることはないらしい)
         ent && nprefs < MAXPREFS;
         ent = strtok(NULL, ":")) {
        // フォーマットチェック
        // (Shift-JIS の 2 バイト目が「/」「:」になることはないらしい)
        if (sscanf(ent, "/%[^/]/%[^/]/%[^/]/%[^/]/",
                   smkTbl, sst, smkSt1, smkSt2) != 4) { continue; }
        // チェック OK。定義開始。
        struct PREF *q = &(pref[nprefs++]);

        // mkTbl
        q->mkTbl[0] = '\0'; moji2strcat(q->mkTbl, STR2MOJI(smkTbl));

        // stlen, st
        // 解析しやすいように「,」を付加
        sst[strlen(sst) + 1] = '\0'; sst[strlen(sst)] = ',';
        for (q->stlen = 0, p = sst;
             q->stlen < MAXPREFLEN;
             p += n, q->stlen++) {
            if (1 <= sscanf(p, "%d,%n", &k, &n)) ;
            else if (1 <= sscanf(p, "S%d,%n", &k, &n)) k = TC_SHIFT(k);
            else break;
            q->st[q->stlen] = k;
        }

        // mkSt
        // 記号の初期化
        for (i = 0; i < 6; i++) { strcpy(q->mkSt[i], defmkSt[i]); }

        // 記号その一 (単独打鍵)
        for (i = 0, p = smkSt1;
             *p && (m = str2moji(p, &p)) != 0 && i < 4;
             i++) {
            q->mkSt[i][0] = '\0';
            moji2strcat(q->mkSt[i], m);
        }

        // 記号その二 (連続打鍵)
        for (i = 0, p = smkSt2;
             *p && (m = str2moji(p, &p)) != 0 && i < 2;
             i++) {
            q->mkSt[i + 4][0] = '\0';
            moji2strcat(q->mkSt[i + 4], m);
        }

    }

    delete [] spec;
}
//</multishift>

//<multishift>
void StTable::matchPref(STROKE *st, STROKE **pst, 
                        char **pmkTbl, char (**pmkSt)[3]) {
    for (int i = 0; i < nprefs; i++) {
        struct PREF *q = &(pref[i]);

        for (int j = 0; j < q->stlen; j++) {
            if (q->st[j] != st[j]) { goto MATCHPREF_UNMATCH; }
        }
        // マッチした
        if (pst) { *pst = st + q->stlen; }
        if (pmkSt) { *pmkSt = q->mkSt; }
        if (pmkTbl) { *pmkTbl = q->mkTbl; }
        return;

    MATCHPREF_UNMATCH:
        continue;
    }
    // マッチなし
    if (pst) { *pst = st; }
    if (pmkSt) { *pmkSt = defmkSt; }
    if (pmkTbl) { *pmkTbl = defmkTbl; }
}
//</multishift>

// -------------------------------------------------------------------
