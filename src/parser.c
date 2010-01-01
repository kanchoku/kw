#include "parser.h"
#include "moji.h"
#include "tc.h"
#include "debug.h"

// モードひとつ分読む
//ただし現状はファイル末尾まで同じモードとみなしている
ControlBlock *Parser::parse() {
	ControlBlock *node = new ControlBlock();
    setNextToken();
    while (currentToken) {
        switch(currentToken) {
        case LBRACE:
            parseControl(node);
            break;
        case ARROW:
            parseRoute(node);
            break;
        default:
            parseError();
        }
        setNextToken();
    }
    return node;
}

// control block をひとつ読む
void Parser::parseControl(ControlBlock *node) {
    checkToken(LBRACE);         // 最初の '{'
    setNextToken();

    for (int i = 0; i < TC_NKEYS; i++) {
      for (int j = 0; j < 2; j++) {
        int k = (j?TC_SHIFT(i):i);
        if (currentToken == LBRACE || currentToken == ARROW) {
            if (!node->block[k]) node->block[k] = new ControlBlock();
            if (node->block[k]->kind() != CONTROL_BLOCK) {
                delete(node->block[k]);
                node->block[k] = new ControlBlock();
            }
        }
        switch(currentToken) {
        case LBRACE:            // '{' : ネストした control block
            parseControl((ControlBlock *)node->block[k]);
            setNextToken();
            if (currentToken != ARROW) break;
        case ARROW:             // '-n>' : control block の指定位置に移動
            while (currentToken == ARROW) {
                parseRoute((ControlBlock *)node->block[k]);
                setNextToken();
            }
            break;
        case RBRACE:            // '}' が来たらブランク
        case COMMA:             // ',' が来てもブランク
		case SLASH:             // '/' が来てもブランク
            break;
        case STRING:            // "str" : 文字列ブロック
            if (node->block[k]) delete(node->block[k]);
            node->block[k] = new StringBlock(buffer);
            setNextToken();
            break;
        case SPECIAL:           // @c : 特殊ブロック
            if (node->block[k]) delete(node->block[k]);
            node->block[k] = new SpecialBlock(buffer[0]);
            setNextToken();
            break;
        case 0:                 // 途中でファイルが終わった場合 : エラー
            parseError();
            break;
        }
        if (currentToken == SLASH) {
            setNextToken();
            continue;
        }
        break;
      }

        if (i != (TC_NKEYS - 1)) {
            // control ブロックの最後のひとつでない限り、次にはコンマが必要
            checkToken(COMMA);
            setNextToken();
        }
    }

    checkToken(RBRACE);         // 最初の '}'
    return ;
}

// control block の中に移動
void Parser::parseRoute(ControlBlock *node) {
    checkToken(ARROW);
    int k = buffer[0];
    setNextToken();

        if (currentToken == LBRACE || currentToken == ARROW) {
            if (!node->block[k]) node->block[k] = new ControlBlock();
            if (node->block[k]->kind() != CONTROL_BLOCK) {
                delete(node->block[k]);
                node->block[k] = new ControlBlock();
            }
        }
        switch(currentToken) {
        case LBRACE:            // '{' : ネストした control block
            parseControl((ControlBlock *)node->block[k]);
            break;
        case ARROW:             // -n> : control block の指定位置に移動
            parseRoute((ControlBlock *)node->block[k]);
            break;
        case STRING:            // "str" : 文字列ブロック
            if (node->block[k]) delete(node->block[k]);
            node->block[k] = new StringBlock(buffer);
            break;
        case SPECIAL:           // @c : 特殊ブロック
            if (node->block[k]) delete(node->block[k]);
            node->block[k] = new SpecialBlock(buffer[0]);
            break;
        case RBRACE:            // '}' が来たらエラー
        case COMMA:             // ',' が来てもエラー
        case SLASH:             // '/' が来てもエラー
        case 0:                 // 途中でファイルが終わった場合 : エラー
            parseError();
            break;
        }

    return ;
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
    if (is->eof()) return NULL;

    // '#' または ';' 以降、行末までコメント
    if (c == '#' || c == ';') {
        do {
            is ->get(c);
        } while (!is->eof() && c != 0 && c != '\n' && c != '\r');
        if (is->eof()) return NULL;
    }

    // 一文字のトークンシリーズ
    switch (c) {
    case '{': return LBRACE;
    case '}': return RBRACE;
    case ',': return COMMA;
    case '/': return SLASH;

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

    // 矢印
    if (c == '-') {
        is->get(c);
        int count = 0;
        while (!is->eof() && count < BUFFER_SIZE && c != '>') {
            if (is->eof() || c == 0) {
                parseError();
            }
            buffer[count] = c;
            count++;
            is->get(c);
        }
        buffer[count] = 0;
        int k;
        if (1 <= sscanf(buffer, "%d", &k)) ;
        else if (1 <= sscanf(buffer, "S%d", &k)) k = TC_SHIFT(k);
        else parseError();
        buffer[0] = k;
        return ARROW;
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
    while (!is->eof() && count < BUFFER_SIZE && c != '"') {
        if (is->eof() || c == 0) {
            parseError();
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
