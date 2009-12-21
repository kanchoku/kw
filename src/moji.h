#ifndef MOJI_H
#define MOJI_H
/* -------------------------------------------------------------------
 * MOJI 型
 * -------
 *
 * 文字は MOJI (== unsigned short) 型で扱う。
 * 通常の文字のほかに、仮想キーや、特殊文字 (部首合成や交ぜ書き変換の開始点)
 * も、MOJI 型で扱うことにしている。
 * 文字とそれに対応する MOJI 型の表現 m は、次のとおり。
 *
 * - ASCII 文字 c   : m = (MOJI)c
 * - 半角かな k     : m = (MOJI)k        (0x80 <= k)
 * - 全角 (h l)     : m = B2MOJI(h, l)   (0x80 <= h)
 *
 * 半角かなをちゃんと扱うのなら、濁点や半濁点を、直前の文字とともに
 * まとめて 1 文字とするのがいいかも知れない。
 * 全角かな/半角かな の変換をする時とか。
 * <hankana>
 * ……と思ったが、
 * やはり、濁点や半濁点も常に独立した文字として扱うことにした。
 * </hankana>
 */

typedef unsigned short MOJI;

/* -------------------------------------------------------------------
 * char などとの相互変換
 */

// 全角文字 (の 1 バイト目) かどうか
#define MSBOFF(c) ((c) & 0x7f)
#if 1  //<OKA> This may be better...
#define IS_ZENKAKU(c) (((c) & 0x80) && \
                       (0x01 <= MSBOFF(c) && MSBOFF(c) <= 0x1f || \
                        0x60 <= MSBOFF(c) && MSBOFF(c) <= 0x7c))
#else  //<OKA> ...than below.
#define IS_ZENKAKU(c) (((c) & 0x80) && \
                       (0x01 <= MSBOFF(c) & MSBOFF(c) <= 0x1f || \
                        0x60 <= MSBOFF(c) & MSBOFF(c) <= 0x7c))
#endif //</OKA>

// 半角かな文字かどうか
#define IS_HANKANA(c) (((c) & 0x80) && \
                       (0x20 <= MSBOFF(c) && MSBOFF(c) <= 0x5f))

// 2 文字の char から MOJI 型を生成
#if 1  //<OKA> This may be better...
#define B2MOJI(h, l) MOJI((unsigned)((h) & 0xff) << 8 | (unsigned)((l) & 0xff))
#else  //<OKA> ...than below.
#define B2MOJI(h, l) ((unsigned)((h) & 0xff) << 8 | (unsigned)((l) & 0xff))
#endif //</OKA>

//<multishift>
// 文字列 s の先頭の 1 文字を MOJI 型に変換
#define STR2MOJI(s) B2MOJI(*(s), *((s) + 1))
//</multishift>

// MOJI から h と l を取り出す
#define MOJI2H(m) (((m) >> 8) & 0xff)
#define MOJI2L(m) ((m) & 0xff)

// MOJI の種類を返す
int mojitype(MOJI);

// char 文字列の先頭の 1 文字を MOJI 型で取り出す
MOJI str2moji(const char *, char **);

// MOJI を、文字列バッファに追加して書きこむ (strcat のように)
char *moji2strcat(char *, MOJI);

/* -------------------------------------------------------------------
 * MOJI 型の各種変換
 * - mojiHirakata() : ひらがな/かたかな変換
 * - mojiHanzen()   : 半角/全角変換 (ASCII 文字)
 * - mojiDaku()     : 清音/濁音変換
 * - mojiHandaku()  : 清音/半濁音変換
 * - mojiPunct()    : 句読点変換 (「、。」/「，．」)
 * - mojiHankana()  : <hankana/> 全角かな → 半角かな の変換
 */
MOJI mojiHirakata(MOJI);
MOJI mojiHanzen(MOJI);
MOJI mojiDaku(MOJI);
MOJI mojiHandaku(MOJI);
MOJI mojiPunct(MOJI);
//<hankana>
int mojiHankana(MOJI, MOJI *, MOJI *);
//</hankana>

/* -------------------------------------------------------------------
 * 文字の種類
 */
#define MOJI_UNKNOWN -1
#define MOJI_ASCII    0         // ASCII 文字
#define MOJI_HANKANA 'K'        // 半角かな
#define MOJI_ZENKAKU 'Z'        // 全角
#define MOJI_UNICODE 'U'        // ユニコード

#define MOJI_SPECIAL '@'        // 特殊
#define MOJI_VKEY    '!'        // 仮想キー

/* -------------------------------------------------------------------
 * 特殊文字
 */
#define MOJI_BUSHU B2MOJI('@', 'b') // 前置部首合成の変換開始点のマーカ
#define MOJI_MAZE  B2MOJI('@', 'm') // 前置交ぜ書きの変換開始点のマーカ

/* -------------------------------------------------------------------
 * MojiBuffer
 * ----------
 * 固定サイズの MOJI 型のバッファ
 * pushHard() を使うことにより、リングバッファとして使える。
 */
class MojiBuffer {
    int size;                   // バッファの大きさ
    MOJI *buf;                  // size の大きさを持つ配列
    int beg;                    // 内容の先頭位置 (0 .. (bufferSize - 1))
    int len;                    // 内容の長さ     (0 .. bufferSize)
    //int pt;                   // 内容の中を動くポインタ (0 .. (len - 1))
    char *str;                  // 内容を文字列として返す時に使う一時バッファ

public:
    MojiBuffer(int);
    ~MojiBuffer();

    void clear();               // 内容の消去
    int length();               // 文字数
    int isEmpty();              // バッファが空かどうか
    int isFull();               // バッファが一杯かどうか
    void pushSoft(MOJI);        // 挿入 (size を超えない)
    void pushSoft(char *);      // 〃
    void pushSoftN(MOJI, int);  // 〃 (同じものを N 個)
    void pushSoftN(char *, int); // 〃
    void pushHard(MOJI);        // 挿入 (超過分は上書き)
    void pushHard(char *);      // 〃
    MOJI pop();                 // 末尾の 1 文字の取り出し
    MOJI popN(int);             // 末尾の N 文字の取り出し
    MOJI moji(int);             // 指定位置の文字
    char *string(int, int);     // 指定位置・指定長さの文字列
    char *string(int);          // 指定位置の文字列
    char *string();             // 文字列として返す
};

// -------------------------------------------------------------------
#endif // MOJI_H
