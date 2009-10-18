#ifndef TC_H
#define TC_H
/* -------------------------------------------------------------------
 * tc.h - T-Code 関連の定義のうち比較的共通に使うもの
 */

/* TC_NKEYS
 * --------
 * T-Code 入力で用いるキーの数。
 *
 * TC_BUFFER_SIZE
 * --------------
 * ミニバッファなどの、内部バッファのサイズ (文字数)
 *
 * TC_NHIST
 * --------
 * ヒストリバッファのサイズ (候補数)。
 * ヒストリ入力は、10 列のキーで選択する仕様なので、変更しないこと。
 */
#define TC_NKEYS 49
#define TC_BUFFER_SIZE 64
#define TC_NHIST 10

/* シフト打鍵
 * ----------
 * - TC_SHIFT(k)    : キー番号 k のキーのシフト打鍵時のキー番号
 * - TC_UNSHIFT(k)  : キー k のアンシフト時のキー番号
 * - TC_ISSHIFTED(k): キー k がシフト打鍵かどうか
 */
#define TC_SHIFT(k) ((k) + ((k)<TC_NKEYS?TC_NKEYS:0))
#define TC_UNSHIFT(k) ((k) - ((k)>=TC_NKEYS&&(k)<TC_NKEYS*2?TC_NKEYS:0))
#define TC_ISSHIFTED(k) ((k)>=TC_NKEYS&&(k)<TC_NKEYS*2)

/* 仮想鍵盤 (の描画) に関する定数類
 * --------------------------------
 * 仮想鍵盤に表示する文字の種類に応じた、前景色 (TC_FG_*) と背景色 (TC_BG_*)。
 * 具体的な配色については、table_window.[ch] を参照。
 */
/* 前景色 (TCode::vkbFG[] のとる値)
 */
#define TC_FG_NIL       0       // なし
#define TC_FG_NORMAL    1       // 通常の文字
#define TC_FG_SPECIAL   2       // 機能キー (部首合成、交ぜ書き変換など)
#define TC_FG_STROKE    3       // 文字ヘルプに使う文字 (●○◎・など)
#define TC_FG_HISTREF   4       // ヒストリ入力で、参照マークの付いた候補
//<v127a - gg>
#define TC_FG_GG        5       // 連漢字ガイド
//</v127a - gg>
/* 背景色 (TCode::vkbBG[] のとる値)
 */
#define TC_BG_NIL       0       // なし
#define TC_BG_ST1       1       // 第 1 打鍵
#define TC_BG_ST2       2       // 第 2 打鍵
#define TC_BG_ST3       3       // 第 3 打鍵
#define TC_BG_STF       4       // 第 4 打鍵、およびそれ以降
#define TC_BG_STW       5       // 二重打鍵
#define TC_BG_STX       6       // 二重打鍵 (その 2)
//<multishift>
//#define TC_BG_ST1R      11      // 三枚表 T-Code の右表の第 1 打鍵
//#define TC_BG_ST2R      12      // 三枚表 T-Code の右表の第 2 打鍵
//#define TC_BG_STWR      13      // 三枚表 T-Code の右表の二重打鍵
//#define TC_BG_ST1L      21      // 三枚表 T-Code の左表の第 1 打鍵
//#define TC_BG_ST2L      22      // 三枚表 T-Code の左表の第 2 打鍵
//#define TC_BG_STWL      23      // 三枚表 T-Code の左表の二重打鍵
//</multishift>
#define TC_BG_HISTPTR   40      // ヒストリ入力で、直前に追加された候補

//<multishift>
#define TC_MK_ST1       "●"    // 第 1 打鍵
#define TC_MK_ST2       "○"    // 第 2 打鍵
#define TC_MK_ST3       "△"    // 第 3 打鍵
#define TC_MK_STF       "◇"    // 第 4 打鍵
#define TC_MK_STW       "◎"    // 二重打鍵
#define TC_MK_STX       "☆"    // 二重打鍵 (その 2)
//</multishift>

#define TC_MK_SH1       1
#define TC_MK_SH2       2
#define TC_MK_SH3       4


//<multishift2>
// -------------------------------------------------------------------
// directive
//
// テーブルファイル (*.tbl) 中に、入力方式固有の設定を記述。
// 書式は、次のような感じ。
//
// #define table-name "T"
// #define prefix /▲/26,23/▲○/▲/:/▽/23,26/▽○/▽/
// #define def-guide /亜域液牡凱梶丸偽漁吟芸元鯉剛坐雑事蛇什傷…/
//
// 「#」で始まる行は、テーブル定義を読みこむ時点ではコメントとして
// 扱われることに注意。
//</multishift2>

//<multishift2>
#define DIR_table_name      0
#define DIR_prefix          1
#define DIR_defguide        2

#define DIR_MAX_ENT         3

typedef char *DIR_TABLE[DIR_MAX_ENT];
//</multishift2>

// bushu conv algorithm
// -------------------------------------------------------------------
#define TC_BUSHU_ALGO_OKA      1 // 岡アルゴリズム (tc.el)
#define TC_BUSHU_ALGO_YAMANOBE 2 // 山辺アルゴリズム [tcode-ml:2652]

// -------------------------------------------------------------------
#endif // TC_H
