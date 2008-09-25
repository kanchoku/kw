#include "parser.h"
#include "moji.h"
#include "tc.h"
#include "debug.h"

// モードひとつ分読む
ControlBlock *Parser::parse() {
    setNextToken();
    return parseControl();      // モードは control block ひとつからなる
}

// control block をひとつ読む
ControlBlock *Parser::parseControl() {
    checkToken(LBRACE);         // 最初の '{'
    setNextToken();

    ControlBlock *node = new ControlBlock(); // control block を用意
    for (int i = 0; i < TC_NKEYS; i++) {
        switch(currentToken) {
        case LBRACE:            // '{' : ネストした control block
            node->block[i] = parseControl();
            setNextToken();
            break;
        case RBRACE:            // '}' が来たらブランク
        case COMMA:             // ',' が来てもブランク
            node->block[i] = 0;
            break;
        case STRING:            // "str" : 文字列ブロック
            node->block[i] = new StringBlock(buffer);
            setNextToken();
            break;
        case SPECIAL:           // @c : 特殊ブロック
            node->block[i] = new SpecialBlock(buffer[0]);
            setNextToken();
            break;
        case 0:                 // 途中でファイルが終わった場合 : エラー
            parseError();
            break;
        }

        if (i != (TC_NKEYS - 1)) {
            // control ブロックの最後のひとつでない限り、次にはコンマが必要
            checkToken(COMMA);
            setNextToken();
        }
    }

    checkToken(RBRACE);         // 最初の '}'
    return node;
}

// 現在のトークンを決め打ち
void Parser::checkToken(int target) {
    if (currentToken != target) {
        parseError();           // 違ったらエラー
    }
}

// トークンひとつ読んで currentToken にセット
void Parser::setNextToken() {
    currentToken = getToken();
}

// トークンを読む
int Parser::getToken() {
    char c;

    is->get(c);

    // '#' または ';' 以降、行末までコメント
    if (c == '#' || c == ';') {
        do {
            is ->get(c);
        } while (c != 0 && c != '\n' && c != '\r');
    }

    // 一文字のトークンシリーズ
    switch (c) {
    case '{': return LBRACE;
    case '}': return RBRACE;
    case ',': return COMMA;

    case '\n':                  // ^J  : スキップだけど行番号を増やす
        lineNumber++;
        // go down
    case ' ':                   // SPC : スキップ
    case '\t':                  // TAB : スキップ
    case '\r':                  // ^M  : スキップ
    case '\f':                  // ^L  : スキップ
        return getToken();

    case 0:
        return NULL;
    }

    // 特殊
    if (c == '@') {
        is->get(c);
        buffer[0] = c;
        return SPECIAL;
    }

    // この時点で、残りは文字列のみ。違ったらエラー。
    if (c != '"') {
        parseError();
    }

    // '"' が来るまで読みこんで、buffer に格納。
    // 「"」自身は「"\""」と表記することで指定できる。
    // 「\」自身は「"\\"」と表記する。
    // 「\」は、単に次の一文字をエスケープするだけで、
    // 「"\n"」「"\t"」「"\ooo"」は未対応。
    is->get(c);
    int count = 0;
    while (c != '"') {
        if (c == 0) {
            exit(1);            // 後でちゃんとエラーに
        }
        if (IS_ZENKAKU(c)) {      // 先頭の 1 バイトを見て判断
            // Shift-JIS の 2 バイト文字
            buffer[count] = c; count++; is->get(c);
            buffer[count] = c; count++; is->get(c);
        } else {
            // 1 バイト文字
            if (c == '\\') {
                // 最初の「\」は、単に読みとばす
                is->get(c);
            }
            buffer[count] = c; count++; is->get(c);
        }
    }
    buffer[count] = 0;

    return STRING;
}

// 読みこみに失敗した場合
void Parser::parseError() {
    sprintf(buffer, "テーブルファイルの %d 行目がまちがっているようです",
            lineNumber);
    MessageBoxEx(hwnd, buffer, "エラー",
                 MB_OK | MB_ICONERROR, LANG_JAPANESE);
    exit(1);
}

// -------------------------------------------------------------------
