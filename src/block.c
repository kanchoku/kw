#include "block.h"
#include "tc.h"
#include "debug.h"
// -------------------------------------------------------------------

StringBlock::StringBlock(char *s) {
    //<v127a - gg>
    flagGG = 0;
    //</v127a - gg>
    if (s == 0) {               // 文字列がない場合
        str = 0;
    } else {                    // 文字列がある場合 - 文字列を保存する
        length = strlen(s);
        int l = length + 1;
        str = new char[l];
        strcpy(str, s);

        // 2 バイトの face を用意する
        // 2 バイト以下の場合は、空白で埋める
        // 2 バイト以上の場合は、str をそのまま使う
#if 0
        if (length < 2) {
            face = new char[3];
            if (length == 0) { face[0] = ' '; }
            else             { face[0] = str[0]; }
            face[1] = ' ';
            face[2] = 0;
        } else {
            face = str;
        }
#endif
        // ……と思ったが、1 バイトは 1 バイトのままという仕様に変更
        switch (length) {
        case 0: face = NULL; break;
        case 1: face = new char[2]; strcpy(face, str); break;
        default:
            face = new char[3];
            strncpy(face, str, 2); face[2] = 0;
            break;
        }
    }
}

StringBlock::~StringBlock() {
    if (str != face) { delete(face); }
    delete(str);
}

// -------------------------------------------------------------------

ControlBlock::ControlBlock() {
    //<v127a - gg>
    faceGG = NULL;
    //</v127a - gg>
    /* ぶらさがるブロックを格納する領域を確保し 0 に初期化
     */
    block = new Block *[TC_NKEYS*2];
    for (int i = 0; i < TC_NKEYS*2; i++) { block[i] = 0; }
}

ControlBlock::~ControlBlock() {
    /* ぶらさがってるブロックを全部消す
     */
    for (int i = 0; i < TC_NKEYS*2; i++) { delete(block[i]); }
}

// -------------------------------------------------------------------

SpecialBlock::SpecialBlock(char c) {
    switch (c) {
    case 's':                   // A/B モード切り替え (OBSOLETE)
        function = F_SWITCH_MODE; break;
    case 'K':                   // ひらがな/かたかな
        function = F_HIRAKATA; break;
    case 'Z':                   // 半角/全角
        function = F_HANZEN; break;
    case 'p':                   // 句読点切り替え
        function = F_PUNCT; break;

    case 'b':                   // 前置型の部首合成変換
        function = F_BUSHU_PRE; break;
    case 'm':                   // 前置型の交ぜ書き変換
        function = F_MAZE_PRE; break;
    case '!':                   // ヒストリ入力
        function = F_HIST; break;

    case 'q':                   // 変換の中断
        function = F_QUIT; break;

    case 'B':                   // 後置型の部首合成変換
        function = F_BUSHU_POST; break;
    case '1': function = F_MAZE_POST1; break; // 後置型の交ぜ書き変換 (1 文字)
    case '2': function = F_MAZE_POST2; break; // 後置型の交ぜ書き変換 (2 文字)
    case '3': function = F_MAZE_POST3; break; // 後置型の交ぜ書き変換 (3 文字)
    case '4': function = F_MAZE_POST4; break; // 後置型の交ぜ書き変換 (4 文字)
    case '5': function = F_MAZE_POST5; break; // 後置型の交ぜ書き変換 (5 文字)
    case '6': function = F_MAZE_POST6; break; // 後置型の交ぜ書き変換 (6 文字)
    case '7': function = F_MAZE_POST7; break; // 後置型の交ぜ書き変換 (7 文字)
    case '8': function = F_MAZE_POST8; break; // 後置型の交ぜ書き変換 (8 文字)
    case '9': function = F_MAZE_POST9; break; // 後置型の交ぜ書き変換 (9 文字)
    case 'D':                   // 後置型の濁点
        function = F_DAKUTEN; break;
    case 'P':                   // 後置型の半濁点
        function = F_HANDAKUTEN; break;

    case 'h':                   // 文字ヘルプ (前の文字)
        function = F_HELP_BACKW; break;
    case 'H':                   // 文字ヘルプ (次の文字)
        function = F_HELP_FORW; break;

    case 'v':                   // 第一打鍵のキー
        function = F_VERB_FIRST; break;

    default:                    // その他 (未実装のもの)
        function = 0; break;
    }
}

char *SpecialBlock::getFace() {
    switch (function) {
    case F_SWITCH_MODE: return "替";
    case F_HIRAKATA:    return "ア";
    case F_HANZEN:      return "全";
    case F_PUNCT:       return "句";

    case F_BUSHU_PRE:   return "◆";
    case F_MAZE_PRE:    return "◇";
    case F_HIST:        return "◎";

    case F_QUIT:        return "×";

    case F_BUSHU_POST:  return "部";
    case F_MAZE_POST1:  return "変";
    case F_MAZE_POST2:  return "変";
    case F_MAZE_POST3:  return "変";
    case F_MAZE_POST4:  return "変";
    case F_MAZE_POST5:  return "変";
    case F_MAZE_POST6:  return "変";
    case F_MAZE_POST7:  return "変";
    case F_MAZE_POST8:  return "変";
    case F_MAZE_POST9:  return "変";
    case F_DAKUTEN:     return "゛";
    case F_HANDAKUTEN:  return "゜";

    case F_HELP_BACKW:  return "≪";
    case F_HELP_FORW:   return "≫";

    case F_VERB_FIRST:  return "・";

    default:            return "○";
    }
}

// -------------------------------------------------------------------

void *StringBlock::accept(BlockVisitor *v) {
    return v->visitStringBlock(this);
}

void *ControlBlock::accept(BlockVisitor *v) {
    return v->visitControlBlock(this);
}

void *SpecialBlock::accept(BlockVisitor *v) {
    return v->visitSpecialBlock(this);
}

// -------------------------------------------------------------------
// EOF
