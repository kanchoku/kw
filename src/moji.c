#include <string.h>

#include "moji.h"
#include "debug.h"

/* -------------------------------------------------------------------
 * 文字 m の種類を返す
 */
int mojitype(MOJI m) {
    int h = MOJI2H(m);
    int l = MOJI2L(m);

    if (IS_ZENKAKU(h)) { return MOJI_ZENKAKU; }

    switch (h) {
    case 0:
    if (IS_HANKANA(l)) { return MOJI_HANKANA; }
    else { return MOJI_ASCII; }

    case MOJI_VKEY:
    return MOJI_VKEY;

    case MOJI_SPECIAL:
    return MOJI_SPECIAL;

    default:
    return MOJI_UNKNOWN;
    }
}

/* -------------------------------------------------------------------
 * char 文字列 src の先頭の 1 文字を MOJI 型で取り出して返す。
 * pnext が NULL でなければ、*pnext に次の文字位置を設定する。
 */
MOJI str2moji(const char *src, char **pnext) {
    MOJI moji;
    const char *p = src;

    if (IS_ZENKAKU(*p)) {
        moji = B2MOJI(*p, *(p + 1));
        p++;
    } else {
        moji = B2MOJI(0, *p);
    }
    if (*p != '\0') { p++; }
    if (pnext != 0) { *pnext = (char *)p; }
    return moji;
}

/* -------------------------------------------------------------------
 * 文字 moji を、文字列 dst の後ろに連結し、dst を返す。
 * strlen(dst) の増加量は最大で 2。
 * 領域のチェックは行わない。
 */
char *moji2strcat(char *dst, MOJI moji) {
    unsigned h, l;
    int len;

    h = MOJI2H(moji);
    l = MOJI2L(moji);
    len = strlen(dst);
    if (h) {
        dst[len] = h; len++; dst[len] = '\0';
    }
    dst[len] = l; len++; dst[len] = '\0';
    return dst;
}

/* -------------------------------------------------------------------
 * MOJI 型の各種変換
 */
#define MS(s) B2MOJI(*(s), *((s) + 1))
#define MC(c) B2MOJI(0, (c))

/* ひらがな/かたかな の連想テーブル。
 * 0 で終端すること。
 */
static MOJI tblHirakata[][2] = {
    {MS("ぁ"), MS("ァ")}, {MS("あ"), MS("ア")}, {MS("ぃ"), MS("ィ")},
    {MS("い"), MS("イ")}, {MS("ぅ"), MS("ゥ")}, {MS("う"), MS("ウ")},
    {MS("ぇ"), MS("ェ")}, {MS("え"), MS("エ")}, {MS("ぉ"), MS("ォ")},
    {MS("お"), MS("オ")},
    {MS("か"), MS("カ")}, {MS("が"), MS("ガ")}, {MS("き"), MS("キ")},
    {MS("ぎ"), MS("ギ")}, {MS("く"), MS("ク")}, {MS("ぐ"), MS("グ")},
    {MS("け"), MS("ケ")}, {MS("げ"), MS("ゲ")}, {MS("こ"), MS("コ")},
    {MS("ご"), MS("ゴ")},
    {MS("さ"), MS("サ")}, {MS("ざ"), MS("ザ")}, {MS("し"), MS("シ")},
    {MS("じ"), MS("ジ")}, {MS("す"), MS("ス")}, {MS("ず"), MS("ズ")},
    {MS("せ"), MS("セ")}, {MS("ぜ"), MS("ゼ")}, {MS("そ"), MS("ソ")},
    {MS("ぞ"), MS("ゾ")},
    {MS("た"), MS("タ")}, {MS("だ"), MS("ダ")}, {MS("ち"), MS("チ")},
    {MS("ぢ"), MS("ヂ")}, {MS("っ"), MS("ッ")}, {MS("つ"), MS("ツ")},
    {MS("づ"), MS("ヅ")}, {MS("て"), MS("テ")}, {MS("で"), MS("デ")},
    {MS("と"), MS("ト")}, {MS("ど"), MS("ド")},
    {MS("な"), MS("ナ")}, {MS("に"), MS("ニ")}, {MS("ぬ"), MS("ヌ")},
    {MS("ね"), MS("ネ")}, {MS("の"), MS("ノ")},
    {MS("は"), MS("ハ")}, {MS("ば"), MS("バ")}, {MS("ぱ"), MS("パ")},
    {MS("ひ"), MS("ヒ")}, {MS("び"), MS("ビ")}, {MS("ぴ"), MS("ピ")},
    {MS("ふ"), MS("フ")}, {MS("ぶ"), MS("ブ")}, {MS("ぷ"), MS("プ")},
    {MS("へ"), MS("ヘ")}, {MS("べ"), MS("ベ")}, {MS("ぺ"), MS("ペ")},
    {MS("ほ"), MS("ホ")}, {MS("ぼ"), MS("ボ")}, {MS("ぽ"), MS("ポ")},
    {MS("ま"), MS("マ")}, {MS("み"), MS("ミ")}, {MS("む"), MS("ム")},
    {MS("め"), MS("メ")}, {MS("も"), MS("モ")},
    {MS("ゃ"), MS("ャ")}, {MS("や"), MS("ヤ")}, {MS("ゅ"), MS("ュ")},
    {MS("ゆ"), MS("ユ")}, {MS("ょ"), MS("ョ")}, {MS("よ"), MS("ヨ")},
    {MS("ら"), MS("ラ")}, {MS("り"), MS("リ")}, {MS("る"), MS("ル")},
    {MS("れ"), MS("レ")}, {MS("ろ"), MS("ロ")},
    {MS("ゎ"), MS("ヮ")}, {MS("わ"), MS("ワ")}, {MS("ゐ"), MS("ヰ")},
    {MS("ゑ"), MS("ヱ")}, {MS("を"), MS("ヲ")},
    {MS("ん"), MS("ン")},
    {0, 0}
};

/* 半角/全角 の連想テーブル。
 * 0 で終端すること。
 */
static MOJI tblHanzen[][2] = {
    {MC(' '), MS("　")},         // 全角空白
    {MC('!'), MS("！")}, {MC('"'), MS("”")}, {MC('#'), MS("＃")},
    {MC('$'), MS("＄")}, {MC('%'), MS("％")}, {MC('&'), MS("＆")},
    {MC('\''), MS("’")}, {MC('('), MS("（")}, {MC(')'), MS("）")},
    {MC('*'), MS("＊")}, {MC('+'), MS("＋")}, {MC(','), MS("，")},
    {MC('-'), MS("－")}, {MC('.'), MS("．")}, {MC('/'), MS("／")},
    {MC('0'), MS("０")}, {MC('1'), MS("１")}, {MC('2'), MS("２")},
    {MC('3'), MS("３")}, {MC('4'), MS("４")}, {MC('5'), MS("５")},
    {MC('6'), MS("６")}, {MC('7'), MS("７")}, {MC('8'), MS("８")},
    {MC('9'), MS("９")},
    {MC(':'), MS("：")}, {MC(';'), MS("；")}, {MC('<'), MS("＜")},
    {MC('='), MS("＝")}, {MC('>'), MS("＞")}, {MC('?'), MS("？")},
    {MC('@'), MS("＠")},
    {MC('A'), MS("Ａ")}, {MC('B'), MS("Ｂ")}, {MC('C'), MS("Ｃ")},
    {MC('D'), MS("Ｄ")}, {MC('E'), MS("Ｅ")}, {MC('F'), MS("Ｆ")},
    {MC('G'), MS("Ｇ")}, {MC('H'), MS("Ｈ")}, {MC('I'), MS("Ｉ")},
    {MC('J'), MS("Ｊ")}, {MC('K'), MS("Ｋ")}, {MC('L'), MS("Ｌ")},
    {MC('M'), MS("Ｍ")}, {MC('N'), MS("Ｎ")}, {MC('O'), MS("Ｏ")},
    {MC('P'), MS("Ｐ")}, {MC('Q'), MS("Ｑ")}, {MC('R'), MS("Ｒ")},
    {MC('S'), MS("Ｓ")}, {MC('T'), MS("Ｔ")}, {MC('U'), MS("Ｕ")},
    {MC('V'), MS("Ｖ")}, {MC('W'), MS("Ｗ")}, {MC('X'), MS("Ｘ")},
    {MC('Y'), MS("Ｙ")}, {MC('Z'), MS("Ｚ")},
    {MC('['), MS("［")}, {MC('\\'), MS("￥")}, {MC(']'), MS("］")},
    {MC('^'), MS("＾")}, {MC('_'), MS("＿")},
    //<127e>
    //{MC('`'), MS("｀")},
    {MC('`'), MS("‘")},
    //</127e>
    {MC('a'), MS("ａ")}, {MC('b'), MS("ｂ")}, {MC('c'), MS("ｃ")},
    {MC('d'), MS("ｄ")}, {MC('e'), MS("ｅ")}, {MC('f'), MS("ｆ")},
    {MC('g'), MS("ｇ")}, {MC('h'), MS("ｈ")}, {MC('i'), MS("ｉ")},
    {MC('j'), MS("ｊ")}, {MC('k'), MS("ｋ")}, {MC('l'), MS("ｌ")},
    {MC('m'), MS("ｍ")}, {MC('n'), MS("ｎ")}, {MC('o'), MS("ｏ")},
    {MC('p'), MS("ｐ")}, {MC('q'), MS("ｑ")}, {MC('r'), MS("ｒ")},
    {MC('s'), MS("ｓ")}, {MC('t'), MS("ｔ")}, {MC('u'), MS("ｕ")},
    {MC('v'), MS("ｖ")}, {MC('w'), MS("ｗ")}, {MC('x'), MS("ｘ")},
    {MC('y'), MS("ｙ")}, {MC('z'), MS("ｚ")},
    {MC('{'), MS("｛")}, {MC('|'), MS("｜")}, {MC('}'), MS("｝")},
    {MC('~'), MS("￣")},
    {0, 0}
};

/* 清音/濁音 の連想テーブル。
 * 0 で終端すること。
 */
static MOJI tblDaku[][2] = {
    {MS("あ"), MS("ぁ")}, {MS("い"), MS("ぃ")}, {MS("う"), MS("ぅ")},
    {MS("え"), MS("ぇ")}, {MS("お"), MS("ぉ")},
    {MS("か"), MS("が")}, {MS("き"), MS("ぎ")}, {MS("く"), MS("ぐ")},
    {MS("け"), MS("げ")}, {MS("こ"), MS("ご")},
    {MS("さ"), MS("ざ")}, {MS("し"), MS("じ")}, {MS("す"), MS("ず")},
    {MS("せ"), MS("ぜ")}, {MS("そ"), MS("ぞ")},
    {MS("た"), MS("だ")}, {MS("ち"), MS("ぢ")}, {MS("つ"), MS("づ")},
    {MS("て"), MS("で")}, {MS("と"), MS("ど")},
    {MS("は"), MS("ば")}, {MS("ひ"), MS("び")}, {MS("ふ"), MS("ぶ")},
    {MS("へ"), MS("べ")}, {MS("ほ"), MS("ぼ")},
    {MS("や"), MS("ゃ")}, {MS("ゆ"), MS("ゅ")}, {MS("よ"), MS("ょ")},
    {MS("わ"), MS("ゎ")},

    {MS("ア"), MS("ァ")}, {MS("イ"), MS("ィ")}, {MS("ウ"), MS("ゥ")},
    {MS("エ"), MS("ェ")}, {MS("オ"), MS("ォ")},
    {MS("カ"), MS("ガ")}, {MS("キ"), MS("ギ")}, {MS("ク"), MS("グ")},
    {MS("ケ"), MS("ゲ")}, {MS("コ"), MS("ゴ")},
    {MS("サ"), MS("ザ")}, {MS("シ"), MS("ジ")}, {MS("ス"), MS("ズ")},
    {MS("セ"), MS("ゼ")}, {MS("ソ"), MS("ゾ")},
    {MS("タ"), MS("ダ")}, {MS("チ"), MS("ヂ")}, {MS("ツ"), MS("ヅ")},
    {MS("テ"), MS("デ")}, {MS("ト"), MS("ド")},
    {MS("ハ"), MS("バ")}, {MS("ヒ"), MS("ビ")}, {MS("フ"), MS("ブ")},
    {MS("ヘ"), MS("ベ")}, {MS("ホ"), MS("ボ")},
    {MS("ヤ"), MS("ャ")}, {MS("ユ"), MS("ュ")}, {MS("ヨ"), MS("ョ")},
    {MS("ワ"), MS("ヮ")},

    //<hankana>
    {MC('ｱ'), MC('ｧ')}, {MC('ｲ'), MC('ｨ')}, {MC('ｳ'), MC('ｩ')}, 
    {MC('ｴ'), MC('ｪ')}, {MC('ｵ'), MC('ｫ')}, 
    {MC('ﾔ'), MC('ｬ')}, {MC('ﾕ'), MC('ｭ')}, {MC('ﾖ'), MC('ｮ')}, 
    //</hankana>
    {0, 0}
};

/* 清音/半濁音の連想テーブル。
 * 0 で終端すること。
 */
static MOJI tblHandaku[][2] = {
    //<v127b>
    {MS("う"), MS("ヴ")},
    {MS("か"), MS("ヵ")}, {MS("け"), MS("ヶ")},
    //</v127b>
    {MS("つ"), MS("っ")},
    {MS("は"), MS("ぱ")}, {MS("ひ"), MS("ぴ")}, {MS("ふ"), MS("ぷ")},
    {MS("へ"), MS("ぺ")}, {MS("ほ"), MS("ぽ")},

    {MS("ウ"), MS("ヴ")},
    {MS("カ"), MS("ヵ")}, {MS("ケ"), MS("ヶ")},
    {MS("ツ"), MS("ッ")},
    {MS("ハ"), MS("パ")}, {MS("ヒ"), MS("ピ")}, {MS("フ"), MS("プ")},
    {MS("ヘ"), MS("ペ")}, {MS("ホ"), MS("ポ")},

    //<hankana>
    {MC('ﾂ'), MC('ｯ')}, 
    //</hankana>
    {0, 0}
};

/* 句読点の連想テーブル。
 * 0 で終端すること。
 */
static MOJI tblPunct[][2] = {
    {MS("、"), MS("，")}, {MS("。"), MS("．")},
    {0, 0}
};


//<hankana>
#define MK(s)    B2MOJI(0, *(s)), 0
#define MD(s)    B2MOJI(0, *(s)), B2MOJI(0, *((s) + 1))

/* 全角かな → 半角かな の連想テーブル。
 * 各要素は、
 *   { <全角の MOJI> , <半角の MOJI> , 0 }            (清音の場合)
 * または、
 *   { <全角の MOJI> , <半角の MOJI> , <゛または゜> } (濁音・半濁音の場合)
 * である。
 * 0 で終端すること。
 */
static MOJI tblHankana[][3] = {
    {MS("。"), MK("｡")}, {MS("「"), MK("｢")}, {MS("」"), MK("｣")}, 
    {MS("、"), MK("､")}, {MS("・"), MK("･")}, {MS("ヲ"), MK("ｦ")}, 
    {MS("ァ"), MK("ｧ")}, {MS("ィ"), MK("ｨ")}, {MS("ゥ"), MK("ｩ")}, 
    {MS("ェ"), MK("ｪ")}, {MS("ォ"), MK("ｫ")}, 
    {MS("ャ"), MK("ｬ")}, {MS("ュ"), MK("ｭ")}, {MS("ョ"), MK("ｮ")}, 
    {MS("ッ"), MK("ｯ")}, {MS("ー"), MK("ｰ")}, 
    {MS("ア"), MK("ｱ")}, {MS("イ"), MK("ｲ")}, {MS("ウ"), MK("ｳ")}, 
    {MS("エ"), MK("ｴ")}, {MS("オ"), MK("ｵ")}, 
    {MS("カ"), MK("ｶ")}, {MS("キ"), MK("ｷ")}, {MS("ク"), MK("ｸ")}, 
    {MS("ケ"), MK("ｹ")}, {MS("コ"), MK("ｺ")}, 
    {MS("サ"), MK("ｻ")}, {MS("シ"), MK("ｼ")}, {MS("ス"), MK("ｽ")}, 
    {MS("セ"), MK("ｾ")}, {MS("ソ"), MK("ｿ")}, 
    {MS("タ"), MK("ﾀ")}, {MS("チ"), MK("ﾁ")}, {MS("ツ"), MK("ﾂ")}, 
    {MS("テ"), MK("ﾃ")}, {MS("ト"), MK("ﾄ")}, 
    {MS("ナ"), MK("ﾅ")}, {MS("ニ"), MK("ﾆ")}, {MS("ヌ"), MK("ﾇ")}, 
    {MS("ネ"), MK("ﾈ")}, {MS("ノ"), MK("ﾉ")}, 
    {MS("ハ"), MK("ﾊ")}, {MS("ヒ"), MK("ﾋ")}, {MS("フ"), MK("ﾌ")}, 
    {MS("ヘ"), MK("ﾍ")}, {MS("ホ"), MK("ﾎ")}, 
    {MS("マ"), MK("ﾏ")}, {MS("ミ"), MK("ﾐ")}, {MS("ム"), MK("ﾑ")}, 
    {MS("メ"), MK("ﾒ")}, {MS("モ"), MK("ﾓ")}, 
    {MS("ヤ"), MK("ﾔ")}, {MS("ユ"), MK("ﾕ")}, {MS("ヨ"), MK("ﾖ")}, 
    {MS("ラ"), MK("ﾗ")}, {MS("リ"), MK("ﾘ")}, {MS("ル"), MK("ﾙ")}, 
    {MS("レ"), MK("ﾚ")}, {MS("ロ"), MK("ﾛ")}, 
    {MS("ワ"), MK("ﾜ")}, {MS("ン"), MK("ﾝ")}, 
    {MS("゛"), MK("ﾞ")}, {MS("゜"), MK("ﾟ")}, 

    {MS("ガ"), MD("ｶﾞ")},{MS("ギ"), MD("ｷﾞ")},{MS("グ"), MD("ｸﾞ")},
    {MS("ゲ"), MD("ｹﾞ")},{MS("ゴ"), MD("ｺﾞ")},
    {MS("ザ"), MD("ｻﾞ")},{MS("ジ"), MD("ｼﾞ")},{MS("ズ"), MD("ｽﾞ")},
    {MS("ゼ"), MD("ｾﾞ")},{MS("ゾ"), MD("ｿﾞ")},
    {MS("ダ"), MD("ﾀﾞ")},{MS("ヂ"), MD("ﾁﾞ")},{MS("ヅ"), MD("ﾂﾞ")},
    {MS("デ"), MD("ﾃﾞ")},{MS("ド"), MD("ﾄﾞ")},
    {MS("バ"), MD("ﾊﾞ")},{MS("ビ"), MD("ﾋﾞ")},{MS("ブ"), MD("ﾌﾞ")},
    {MS("ベ"), MD("ﾍﾞ")},{MS("ボ"), MD("ﾎﾞ")},
    {MS("パ"), MD("ﾊﾟ")},{MS("ピ"), MD("ﾋﾟ")},{MS("プ"), MD("ﾌﾟ")},
    {MS("ペ"), MD("ﾍﾟ")},{MS("ポ"), MD("ﾎﾟ")},

    //{MS("を"), MK("ｦ")}, 
    //{MS("ぁ"), MK("ｧ")}, {MS("ぃ"), MK("ｨ")}, {MS("ぅ"), MK("ｩ")}, 
    //{MS("ぇ"), MK("ｪ")}, {MS("ぉ"), MK("ｫ")}, 
    //{MS("ゃ"), MK("ｬ")}, {MS("ゅ"), MK("ｭ")}, {MS("ょ"), MK("ｮ")}, 
    //{MS("っ"), MK("ｯ")}, 
    //{MS("あ"), MK("ｱ")}, {MS("い"), MK("ｲ")}, {MS("う"), MK("ｳ")}, 
    //{MS("え"), MK("ｴ")}, {MS("お"), MK("ｵ")}, 
    //{MS("か"), MK("ｶ")}, {MS("き"), MK("ｷ")}, {MS("く"), MK("ｸ")}, 
    //{MS("け"), MK("ｹ")}, {MS("こ"), MK("ｺ")}, 
    //{MS("さ"), MK("ｻ")}, {MS("し"), MK("ｼ")}, {MS("す"), MK("ｽ")}, 
    //{MS("せ"), MK("ｾ")}, {MS("そ"), MK("ｿ")}, 
    //{MS("た"), MK("ﾀ")}, {MS("ち"), MK("ﾁ")}, {MS("つ"), MK("ﾂ")}, 
    //{MS("て"), MK("ﾃ")}, {MS("と"), MK("ﾄ")}, 
    //{MS("な"), MK("ﾅ")}, {MS("に"), MK("ﾆ")}, {MS("ぬ"), MK("ﾇ")}, 
    //{MS("ね"), MK("ﾈ")}, {MS("の"), MK("ﾉ")}, 
    //{MS("は"), MK("ﾊ")}, {MS("ひ"), MK("ﾋ")}, {MS("ふ"), MK("ﾌ")}, 
    //{MS("へ"), MK("ﾍ")}, {MS("ほ"), MK("ﾎ")}, 
    //{MS("ま"), MK("ﾏ")}, {MS("み"), MK("ﾐ")}, {MS("む"), MK("ﾑ")}, 
    //{MS("め"), MK("ﾒ")}, {MS("も"), MK("ﾓ")}, 
    //{MS("や"), MK("ﾔ")}, {MS("ゆ"), MK("ﾕ")}, {MS("よ"), MK("ﾖ")}, 
    //{MS("ら"), MK("ﾗ")}, {MS("り"), MK("ﾘ")}, {MS("る"), MK("ﾙ")}, 
    //{MS("れ"), MK("ﾚ")}, {MS("ろ"), MK("ﾛ")}, 
    //{MS("わ"), MK("ﾜ")}, {MS("ん"), MK("ﾝ")}, 
    //
    //{MS("が"), MD("ｶﾞ")},{MS("ぎ"), MD("ｷﾞ")},{MS("ぐ"), MD("ｸﾞ")},
    //{MS("げ"), MD("ｹﾞ")},{MS("ご"), MD("ｺﾞ")},
    //{MS("ざ"), MD("ｻﾞ")},{MS("じ"), MD("ｼﾞ")},{MS("ず"), MD("ｽﾞ")},
    //{MS("ぜ"), MD("ｾﾞ")},{MS("ぞ"), MD("ｿﾞ")},
    //{MS("だ"), MD("ﾀﾞ")},{MS("ぢ"), MD("ﾁﾞ")},{MS("づ"), MD("ﾂﾞ")},
    //{MS("で"), MD("ﾃﾞ")},{MS("ど"), MD("ﾄﾞ")},
    //{MS("ば"), MD("ﾊﾞ")},{MS("び"), MD("ﾋﾞ")},{MS("ぶ"), MD("ﾌﾞ")},
    //{MS("べ"), MD("ﾍﾞ")},{MS("ぼ"), MD("ﾎﾞ")},
    //{MS("ぱ"), MD("ﾊﾟ")},{MS("ぴ"), MD("ﾋﾟ")},{MS("ぷ"), MD("ﾌﾟ")},
    //{MS("ぺ"), MD("ﾍﾟ")},{MS("ぽ"), MD("ﾎﾟ")},
    {0, 0, 0}
};

#undef MK
#undef MD
//</hankana>

#undef MS
#undef MC

static MOJI mojiChange(MOJI moji, MOJI m[][2]) {
    for (; (*m)[0] != 0; m++) {
        if (moji == (*m)[0]) {
            return (*m)[1];
        } else if (moji == (*m)[1]) {
            return (*m)[0];
        }
    }
    return moji;
}

MOJI mojiHirakata(MOJI moji) {
    return mojiChange(moji, tblHirakata);
}

MOJI mojiHanzen(MOJI moji) {
    return mojiChange(moji, tblHanzen);
}

MOJI mojiDaku(MOJI moji) {
    return mojiChange(moji, tblDaku);
}

MOJI mojiHandaku(MOJI moji) {
    return mojiChange(moji, tblHandaku);
}

MOJI mojiPunct(MOJI moji) {
    return mojiChange(moji, tblPunct);
}

//<hankana>
/* 全角かな → 半角かな の変換。
 * zen == 『あ』 ⇒ *han == 『ｱ』; *daku == 0;      返戻値 1
 * zen == 『ガ』 ⇒ *han == 『ｶ』; *daku == 『゛』; 返戻値 1
 * zen == 『漢』 ⇒ *han == 『漢』; *daku == 0;     返戻値 0
 * 逆変換 (半角かな → 全角かな) はダメ。
 */
int mojiHankana(MOJI zen, MOJI *han, MOJI *daku) {
    MOJI (*m)[3] = tblHankana;
    for (; (*m)[0] != 0; m++) {
        if (zen == (*m)[0]) {
            *han = (*m)[1]; *daku = (*m)[2];
            return 1;
        }
    }
    *han = zen; *daku = 0;
    return 0;
}
//</hankana>

/* -------------------------------------------------------------------
 * MojiBufer クラス
 */

#define REM(x) ((x) % size)

// コンストラクタ (サイズを指定)
MojiBuffer::MojiBuffer(int n) {
    size = n;
    buf = new MOJI[size];
    str = new char[n * 2 + 1]; // 1 文字は高々 2 バイト
    clear();
}

// デストラクタ
MojiBuffer::~MojiBuffer() {
    delete(buf);
    delete(str);
}

// -------------------------------------------------------------------

// 内容を消去
void MojiBuffer::clear() {
    beg = 0; len = 0; //pt = 0;
}

// 内容の長さ
int MojiBuffer::length() {
    return len;
}

// 空かどうか
int MojiBuffer::isEmpty() {
    return (len == 0);
}

// 一杯かどうか
int MojiBuffer::isFull() {
    return (len == size);
}

// -------------------------------------------------------------------

// 末尾に文字を追加。
// ただし一杯の時は、追加しない (何もしない)。
void MojiBuffer::pushSoft(MOJI m) {
    if (isFull()) {
    // nop
    } else {
    buf[REM(beg + len)] = m;
    len++;
    }
}

void MojiBuffer::pushSoft(char *s) {
    MOJI m;
    for (char *p = s; *p; ) {
        m = str2moji(p, &p);
        pushSoft(m);
    }
}

void MojiBuffer::pushSoftN(MOJI m, int n) {
    for (int i = 0; i < n; i++) { pushSoft(m); }
}

void MojiBuffer::pushSoftN(char *s, int n) {
    for (int i = 0; i < n; i++) { pushSoft(s); }
}

// -------------------------------------------------------------------
// 末尾に文字を追加。
// ただし一杯の時は、先頭の文字を捨てて、末尾に追加する。

void MojiBuffer::pushHard(MOJI m) {
    if (isFull()) {
    beg = REM(beg + 1);
    len--;
    }
    pushSoft(m);
}

void MojiBuffer::pushHard(char *s) {
    MOJI m;
    for (char *p = s; *p; ) {
        m = str2moji(p, &p);
        pushHard(m);
    }
}

// -------------------------------------------------------------------
// 空でない時、末尾の文字を取り除き、それを返す。

MOJI MojiBuffer::pop() {
    if (isEmpty()) {
        return (MOJI)0;
    } else {
        MOJI m = buf[REM(beg + len - 1)];
        len--;
        return m;
    }
}

MOJI MojiBuffer::popN(int n) {
    MOJI m;
    for (int i = 0; i < n; i++) { m = pop(); }
    return m;
}

// -------------------------------------------------------------------
/* バッファの内容の位置 offset の文字を返す。
 * offset : -len .. (len - 1)
 * offset が負の場合、内容の末尾から数える。
 */
MOJI MojiBuffer::moji(int offset) {
    if (len == 0) { return (MOJI)0; } //XXX

    offset = (offset + len) % len;
    return buf[REM(beg + offset)];
}

/* バッファの内容の位置 offset から始まる長さ n の列を
 * 文字列にして返す。
 * offset : -len .. (len - 1)
 * n      : 0 .. (len - offset)
 * offset が負の場合、内容の末尾から数える。
 * n が大きすぎる場合は、内容の末尾まで。
 */
char *MojiBuffer::string(int offset, int n) {
    *str = 0;
    if (len == 0) { return str; }

    offset = (offset + len) % len;
    if (len < n) { n = len - offset; }

    int i = 0;
    for ( ; 0 < n; offset++, n--) {
        MOJI m = buf[REM(beg + offset)];
        char h = (char)MOJI2H(m);
        char l = (char)MOJI2L(m);
        switch (mojitype(m)) {
        case MOJI_ASCII:
        case MOJI_HANKANA:
            str[i] = l; str[++i] = 0;
            break;
        case MOJI_ZENKAKU:
            str[i] = h; str[++i] = l; str[++i] = 0;
            break;
        default:
            str[i] = '?'; str[++i] = 0;
            break;
        }
    }
    return str;
}

// 内容の offset 位置以降を文字列にして返す。
char *MojiBuffer::string(int offset) {
    offset = (offset + len) % len;
    return string(offset, len - offset);
}

// 内容全体を文字列にして返す。
char *MojiBuffer::string() {
    return string(0, len);
}

#undef REM
