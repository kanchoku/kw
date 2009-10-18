#ifndef ST_TABLE_H
#define ST_TABLE_H
// -------------------------------------------------------------------
// class StTable
//
// ストローク列の逆引き表

//<multishift>
#include <iostream>
#include <fstream>
#include <windows.h>
//</multishift>
#include <map>

#include "block.h"
#include "moji.h"

// -------------------------------------------------------------------
// ストローク列 : EOST 終端の文字列で表現

typedef char STROKE;

#define EOST (STROKE)(0xff) // End Of STroke
#define STROKE_MAX (4)

int strokelen(const STROKE *);  // ストローク長
void strokecpy(STROKE *, const STROKE *); // ストロークのコピー

//<multishift>
// -------------------------------------------------------------------
// 多段シフト

#define MAXPREFS    8           // (多段シフト) prefix の個数の上限
#define MAXPREFLEN  4           // (多段シフト) prefix の長さの上限

struct PREF {
    char mkTbl[3];              // テーブルを代表する文字 (『▲』『▽』など
    int stlen;                  // prefix の長さ
    STROKE st[MAXPREFLEN];      // prefix
    char mkSt[6][3];            // ストロークを表す文字 (『▲』『○』など)
};
//</multishift>

// -------------------------------------------------------------------
// StMap : STroke Map

typedef std::map<MOJI, STROKE *> StMap;

// -------------------------------------------------------------------
// StTable : STroke Table

class StTable {
public:
    STROKE *stroke;

    StMap *stMap;

    //<multishift>
    char defmkSt[6][3];         // 文字ヘルプの記号
    char (*mkSt)[3];
    char defmkTbl[3];           // テーブルを代表する文字
    char *mkTbl;
    int nprefs;                 // (多段シフト) prefix の個数
    struct PREF pref[MAXPREFS]; // (多段シフト) prefix 定義
    STROKE *baseStroke;         // prefix を取り除いた strokes
    //</multishift>

    StTable(Block *);
    ~StTable();

    void init(Block *);
    void initSub(Block *, STROKE *);
    int look(MOJI);

    //<multishift>
    /* readFile()  : 「*.tbl」から prefix 定義を読みこむ。
     * setupPref() : "/▲/26,23/▲○/▲/:/▽/23,26/▽○/▽/" のような引数から
     *               prefix 定義を設定。
     * matchPref() : ストローク列と prefix のマッチング。
     */
    void setupPref(const char *);
    //<127e>
    //void matchPref(STROKE *, STROKE **, char **, char (**)[]);
    void matchPref(STROKE *, STROKE **, char **, char (**)[3]);
    //</127e>
    //</multishift>
};

// -------------------------------------------------------------------
#endif // ST_TABLE_H
