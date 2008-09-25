// -------------------------------------------------------------------
// class Block
//
// 文字テーブルを格納するためのクラス

#ifndef BLOCK_H
#define BLOCK_H
// -------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

// -------------------------------------------------------------------
// クラスの種類

#define STRING_BLOCK  1
#define CONTROL_BLOCK 2
#define SPECIAL_BLOCK 3

// -------------------------------------------------------------------
// 特殊ブロックの機能

/* モード類 */
#define F_SWITCH_MODE   10      // @s 替 A/B モードの切り替え (OBSOLETE)
#define F_HIRAKATA      11      // @K ア ひらがな/かたかな
#define F_HANZEN        12      // @Z 全 半角/全角
#define F_PUNCT         13      // @p 句 句読点切り替え
//#define F_HANKANA     14      // @k 半 半角かたかな
/* 前置型 */
#define F_BUSHU_PRE     21      // @b ◆ 前置型の部首合成
#define F_MAZE_PRE      22      // @m ◇ 前置型の交ぜ書き変換
#define F_HIST          31      // @! ◎ ヒストリ入力
/* 取消 */
#define F_QUIT          99      // @q × 変換の中断
/* 後置型 */
#define F_BUSHU_POST    41      // @B 部 後置型の部首合成
#define F_MAZE_POST1    51      // @1 変 後置型の交ぜ書き変換 (1 文字)
#define F_MAZE_POST2    52      // @2 変 後置型の交ぜ書き変換 (2 文字)
#define F_MAZE_POST3    53      // @3 変 後置型の交ぜ書き変換 (3 文字)
#define F_MAZE_POST4    54      // @4 変 後置型の交ぜ書き変換 (4 文字)
#define F_MAZE_POST5    55      // @5 変 後置型の交ぜ書き変換 (5 文字)
#define F_MAZE_POST6    56      // @6 変 後置型の交ぜ書き変換 (6 文字)
#define F_MAZE_POST7    57      // @7 変 後置型の交ぜ書き変換 (7 文字)
#define F_MAZE_POST8    58      // @8 変 後置型の交ぜ書き変換 (8 文字)
#define F_MAZE_POST9    59      // @9 変 後置型の交ぜ書き変換 (9 文字)
#define F_DAKUTEN       61      // @D ゛ 後置型の濁点
#define F_HANDAKUTEN    62      // @P ゜ 後置型の半濁点
/* 文字ヘルプ */
#define F_HELP_BACKW    71      // @h ≪ 文字ヘルプ (前の文字)
#define F_HELP_FORW     72      // @H ≫ 文字ヘルプ (次の文字)
/* キー */
#define F_VERB_FIRST    82      // @v ・ 第一打鍵のキー

// -------------------------------------------------------------------
// Block

// visitor クラス
class BlockVisitor;

// Block の一般形 (抽象クラス)
class Block {
 public:
    virtual char *getFace() = 0; // 表示用文字列
    virtual ~Block() {};
    virtual int kind() = 0;     // クラスの種類
    virtual void *accept(BlockVisitor *) = 0;
};

// StringBlock - 文字列を入力するブロック
class StringBlock : public Block {
 public:
    int length;                 // 文字列の長さ
    char *str;                  // 文字列
    char *face;                 // 表示用文字列
    //<v127a - gg>
    int flagGG;                 // 熟語ガイド対象
    //</v127a - gg>
    char *getFace() { return face; }

    StringBlock(char * = 0);
    ~StringBlock();
    int kind() { return STRING_BLOCK; }
    void *accept(BlockVisitor *);
};

// ControlBlock - 続きがあるブロック
class ControlBlock : public Block {
 public:
    Block **block;
    //<v127a - gg>
    char *faceGG;
    //</v127a - gg>
    char *getFace() { return (char *)"□"; }

    ControlBlock();
    ~ControlBlock();
    int kind() { return CONTROL_BLOCK; }
    void *accept(BlockVisitor *);
};

// SpecialBlock - 何かの機能のブロック
class SpecialBlock : public Block {
 public:
    int function;           // ブロックの機能
    char *getFace();
    SpecialBlock(char);
    ~SpecialBlock() {};
    int kind() { return SPECIAL_BLOCK; }
    void *accept(BlockVisitor *);
};

class BlockVisitor {
 public:
    virtual void *visitStringBlock(StringBlock *) = 0;
    virtual void *visitControlBlock(ControlBlock *) = 0;
    virtual void *visitSpecialBlock(SpecialBlock *) = 0;
};

// -------------------------------------------------------------------
#endif // BLOCK_H
