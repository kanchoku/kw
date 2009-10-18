// -------------------------------------------------------------------
// class Parser
// 文字テーブルをファイルから読むクラス

#ifndef PARSER_H
#define PARSER_H
// -------------------------------------------------------------------

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include "block.h"

using namespace std;

// トークンの種類
#define LBRACE  (1)             // {
#define RBRACE  (2)             // }
#define COMMA   (3)             // ,
#define STRING  (4)             // "str"
#define SPECIAL (5)             // @c
#define SLASH   (6)             // /
#define ARROW   (7)             // -n>

// バッファの大きさ
#define BUFFER_SIZE 8192

// Parser クラス (テーブルファイルを読みこむ)
class Parser {
 public:
    HWND hwnd;                  // window handle
    istream *is;                // 入力ストリーム
    char *buffer;               // 文字列バッファ
    int currentToken;           // 最後に読んだトークン
    int lineNumber;             // 今読んでる行数

    // コンストラクタ
    Parser(istream *i, HWND h) {
        is = i;
        buffer = new char[BUFFER_SIZE];
        lineNumber = 1;
        hwnd = h;
    }

    // デストラクタ
    ~Parser() {
        delete(buffer);
    }

    ControlBlock *parse();      // モードひとつ分読む
    void parseControl(ControlBlock *); // control block をひとつ読む
    void parseRoute(ControlBlock *); // control block の中に移動

    int getToken();             // トークンひとつ読む
    void setNextToken();        // トークンひとつ読んで currentToken にセット
    void checkToken(int);       // トークン決め打ち、違ったらエラー
    void parseError();          // メッセージボックスでエラーを出して終了
};

// -------------------------------------------------------------------
#endif // PARSER_H
