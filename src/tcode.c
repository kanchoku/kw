//<v127a - shiftcheck>
#include <mbstring.h>
//</v127a - shiftcheck>
#include "tcode.h"
#include "table_window.h"
#include "debug.h"

#ifdef _MSC_VER //<OKA>
#define for if(0);else for
#endif          //</OKA>

/* -------------------------------------------------------------------
 * TCode クラス - T-Code 変換器
 */



/* -------------------------------------------------------------------
 * T-Code 変換器の動作
 * ===================
 *
 * T-Code 変換器は、キー入力があるたびに、次の各 phase を繰り返す
 * (NORMAL モード、すなわち通常のストローク入力モードの場合)。
 *
 * (1) [テーブル遷移] - keyinNormal()
 *     キー入力 (T-Code キー番号で入力される) をもとに現在のテーブルを引き、
 *     状態遷移を行う。
 *
 * (2) [文字の拾い出し] - keyinNormal()
 *     遷移の結果が文字列であれば、その文字列を preBuffer の末尾に追加する。
 *     特殊定義のうち、部首合成変換や交ぜ書き変換の開始を示す ◆ や ◇、
 *     また、変換の実行を示す SPC も、そのまま preBuffer に入れる。
 *
 * (3) [変換] - isReducibleBy*(), reduceBy*()
 *     preBuffer の内容を見て、必要ならば部首合成変換または交ぜ書き変換を
 *     実行する (後述)。
 *     変換した文字は helpBuffer に追加する。
 *
 * (4) [出力] - TableWindow::output()
 *     preBuffer の内容が、変換完了状態 (◆ または ◇ がない) であれば、
 *     それを対象アプリに送る。
 *     また、後置変換に備えて、postBuffer にも追加しておく。
 *
 * (5) [仮想鍵盤] - makeVKB(), TableWindow::draw*()
 *     現在の状態に応じて仮想鍵盤を作成し、表示する。
 *
 * CAND モード (交ぜ書き変換の候補選択モード) または HIST モード (ヒストリ入力
 * モード) では、上記の (1) において、通常のテーブルのかわりに、
 * 交ぜ書き変換の候補の配列またはヒストリ候補の配列を用いて、状態遷移を行う。
 *
 * 変換の実行
 * ----------
 *
 * 上記 (3) の変換の動作。
 * 次の (A), (B) の操作を、preBuffer の内容がもはや変化しなくなるまで繰り返す。
 *
 * (A) preBuffer の末尾が「◆ <文字> <文字>」という形をしていれば、
 *     この部分を、部首合成変換した文字で置き換える。
 * (B) preBuffer の末尾が「◇ <文字列> <SPC>」という形をしていれば、
 *     この部分を、交ぜ書き変換した文字列で置き換える。
 *     交ぜ書き変換は一般に候補の選択が必要なので、一時的に変換器を
 *     CAND モード (候補選択モード) に切り替えて処理を行う。
 *
 * 例として、preBuffer の内容が「◇ か ん ◆ ウ」である場合を考える。
 * 次の [入力] phase で、「子」が入力されると、preBuffer の内容は、
 *     「◇ か ん ◆ ウ 子」  →  「◇ か ん 字」
 * のように変化する。
 * さらに、SPC が入力されると、preBuffer の内容は、
 *     「◇ か ん 字 <SPC>」  →  「漢 字」
 * のように (候補選択モードを経て) 変化する。最後の「漢 字」の状態は
 * 変換完了状態であり、次の [出力] phase で出力されることになる。
 *
 * 「◇ か ん ◆ ウ」または「◇ か ん 字」の状態は、(次の入力が来ない限り)
 * 変換可能な状態ではないが、変換完了状態でもない。
 * このような場合は、preBuffer の内容は仮想鍵盤のミニバッファに表示する。
 *
 * -------------------------------------------------------------------
 */

/* 交ぜ書き変換に関する定数
 * ------------------------
 * candOrder
 * ---------
 * 交ぜ書き変換の候補選択での、キーの優先順位 (0 ～ 29)。
 * 値 -1 は候補選択に使用しないキーを表す (無効打鍵)。
 * 優先順位の上位 10 位 (0 ～ 9) は、キーボード中段の 10 キー、すなわち
 * キー番号 20 ～ 29 のキーに割り振る (これは、仮想鍵盤の表示の際に、
 * ヒストリ入力と同様の扱いにするためで、変更しないこと)。
 * 次候補群を指示するための SPC (通常 40 番のキーに割り当てられる) は、
 * -1 でなければならない。
 * なお、前候補群の指示は、通常 BackSpace に割り当てられる。
 * また、将来、文字数を指定しない交ぜ書き変換が実装される場合、
 * 44・45 番のキーが、読みの伸ばし/縮めに予約される可能性がある。
 *
 * candSkip
 * --------
 * 次候補群/前候補群に進む時にいくつの候補をスキップするか。
 * candOrder のうち、値が -1 でないものの個数 (か、それより小さい数)。
 */
const int TCode::candOrder[TC_NKEYS] = {         // (日本語キーボードの例)
    -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1,  // 1 2 3 4 5    6 7 8 9 0
    17, 15, 13, 11, 19,    18, 10, 12, 14, 16,  // q w e r t    y u i o p
     7,  5,  3,  1,  9,     8,  0,  2,  4,  6,  // a s d f g    h j k l ;
    27, 25, 23, 21, 29,    28, 20, 22, 24, 26,  // z x c v b    n m , . /
      -1, -1, -1, -1, -1, -1, -1, -1, -1        // SP - ^ \ @ [ : ] _
};
const unsigned int TCode::candSkip = 30;

/* コンストラクタとデストラクタ
 * ----------------------------
 * 仮想キーの列 v, ストローク漢字変換のテーブル t,
 * 交ぜ書き変換辞書 m, 部首合成辞書 b を取りこんで、変換器を生成。
 * このとき、与えられたテーブルから、ストローク逆引きテーブルを作っておく。
 * v, t, m, b は、複製されないので、他の場所で delete しないこと (デストラクタ
 * 内で delete される)。
 */
//<v127a - gg>
//TCode::TCode(int *v, ControlBlock *t, MgTable *m, BushuDic *b) {
TCode::TCode(int *v, ControlBlock *t, MgTable *m, BushuDic *b, GgDic *g) {
//</v127a - gg>
    vkey = v;
    table = t;
    mgTable = m;
    bushuDic = b;
    //<v127a - shiftcheck>
    isShiftKana = new bool[TC_NKEYS];
    for (int i = 0; i < TC_NKEYS; i++)
        isShiftKana[i] = false;
	isAnyShiftKana = false;
    //</v127a - shiftcheck>
    //<v127a - gg>
    ggDic = g;
    //</v127a - gg>

    preBuffer  = new MojiBuffer(TC_BUFFER_SIZE);
    postBuffer = new MojiBuffer(TC_BUFFER_SIZE);
    helpBuffer = new MojiBuffer(TC_BUFFER_SIZE);
    currentStroke = new std::vector<STROKE>;
    stTable = new StTable(table);
    assignedsBuffer = new MojiBuffer(TC_BUFFER_SIZE);
    explicitGG = 0;
    ggCand = 0;
    ggCInc = 0;
    ittaku = 0;

    OPT_keyboard = 0;
    OPT_tableFile = 0;
    OPT_bushu = 0;
    OPT_mazegaki = 0;
    //<v127a - gg>
    OPT_gg = 0;
    //<gg-defg>
    OPT_defg = 0;
    //</gg-defg>
    //</v127a - gg>

    //<OKA> 初期化しておかないと addToHistMaybe() でクラッシュする
    for (int i = 0; i < TC_NHIST; ++i) {
        hist[i] = NULL;
        histRef[i] = 0;
    }
    histPtr = 0;
    //</OKA>

    //<multishift2>
    // directive テーブルの初期化
    for (int i = 0; i < DIR_MAX_ENT; i++) { dirTable[i] = 0; }
    //</multishift2>

    //<record>
    // 記録用
    record.OPT_record = 0;
    record.nchar = 0;
    record.nstroke = 0;
    record.nbushu = 0;
    record.nmaze = 0;
    record.nspecial = 0;

    // 統計用
    stat.OPT_stat = 0;
    stat.map.clear();
    //</record>
}

TCode::~TCode() {
    delete vkey;
    delete table;
    delete mgTable;
    delete bushuDic;
    //<v127a - shiftcheck>
    delete isShiftKana;
    //</v127a - shiftcheck>

    delete preBuffer;
    delete postBuffer;
    delete helpBuffer;
    delete currentStroke;
    delete stTable;

    if (OPT_keyboard != 0)  { delete OPT_keyboard; }
    if (OPT_tableFile != 0) { delete OPT_tableFile; }
    if (OPT_bushu != 0)     { delete OPT_bushu; }
    if (OPT_mazegaki != 0)  { delete OPT_mazegaki; }
    //<v127a - gg>
    if (OPT_gg != 0)        { delete OPT_gg; }
    //</v127a - gg>
    //<gg-defg>
    if (OPT_defg != 0)      { delete OPT_defg; }
    //</gg-defg>

    //<multishift2>
    for (int i = 0; i < DIR_MAX_ENT; i++) {
        if (dirTable[i]) { delete [] dirTable[i]; }
    }
    //</multishift2>

    //<record>
    if (record.OPT_record) { delete record.OPT_record; }

    if (stat.OPT_stat) { delete stat.OPT_stat; }
    //</record>
}

/* -------------------------------------------------------------------
 * 変換器のリセット
 * ----------------
 */

void TCode::reset() {
    currentBlock = table;
    currentStroke->clear();
    currentShift = 0;
    helpModeSave = 0;
    clearGG(table);
    clearAssignStroke();
    if (!explicitGG) clearCandGG();  // 打ち間違い救済
}

/* -------------------------------------------------------------------
 * キー入力
 * --------
 *
 * - 3 つの主モード (NORMAL, CAND, HIST) に対する、
 * - 5 種の入力キー (RET, BS, ESC, TAB, T-Code キー)
 * の、すべての組み合わせに対処すること。
 *
 * さらに、NORMAL モードにおいては、次の 2 種類の場合分けが必要:
 *
 * (A0) 通常の直接入力              (preBuffer が空っぽ)
 * (A1) 補助変換の読み・部首の入力  (preBuffer に既に文字が入っている)
 *
 * (B0) 第一打鍵待ち        (currentBlock == table)
 * (B1) 第二打鍵以降待ち    (currentBlock != table)
 */

#define ISEMPTY (preBuffer->length() == 0)
#define ISRESET (currentBlock == table)
#define MS(s) B2MOJI(*(s), *((s) + 1))
#define MC(c) B2MOJI(0, (c))
#define MV(v) B2MOJI(MOJI_VKEY, v)

/* NORMAL モードでのキー入力
 * -------------------------
 * - RET    : 補助変換の読み・部首の入力中なら、無変換で確定。
 *            そうでなければ、RET そのもの。
 * - BS     : 第二打鍵以降なら、今までのストロークをキャンセル。
 *            第一打鍵なら、BS そのもの。
 *            補助変換の読み・部首の入力中なら、読み・部首の削除。
 * - ESC    : 第二打鍵以降なら、今までのストロークをキャンセル。
 *            第一打鍵なら、ESC そのもの。
 *            補助変換の読み・部首の入力中なら、読み・部首を破棄。
 * - TAB    : 補助変換の読み・部首の入力中なら、ひらがな/かたかな変換・確定。
 *            第二打鍵以降なら、第一打鍵のキーの入力。
 *            第一打鍵なら、TAB そのもの。
 *
 * - T-Code キー : テーブルを引いて、定義に従う。
 */
void TCode::keyinNormal(int key) {
    // 何かキー入力があったら、まずヘルプを非表示にすること
    helpMode = 0;

    // シフト打鍵
    if (OPT_shiftKana) currentShift |= TC_ISSHIFTED(key);
    /* currentShift &= TC_ISSHIFTED(key); 8 */
    int justCurShift = TC_ISSHIFTED(key);

    int vk;
    switch (key) {
    case RET_KEY:
    case CM_KEY:
    case CJ_KEY:
        switch (key) {
        case CM_KEY:  vk = 'M'; break;
        case CJ_KEY:  vk = 'J'; break;
        case RET_KEY:
        default: vk = VK_RETURN; break;
        }
        if (ISEMPTY) { preBuffer->pushSoft(MV(vk)); }
        else         { nfer(); addToHistMaybe(preBuffer->string()); }
        reset();
        return;

    case BS_KEY:
    case CH_KEY:
        switch (key) {
        case CH_KEY:  vk = 'H'; break;
        case BS_KEY:
        default: vk = VK_BACK; break;
        }
        if (ggCand && ggCInc == 0 && ISRESET) {
            clearCandGG();
            return;
        }
        if (ISEMPTY) {
            if (ISRESET || OPT_hardBS) {
                preBuffer->pushSoft(MV(vk));
                if (ggCand) ggCInc -= 1;
            }
        } else {
            if (ISRESET || OPT_hardBS) {
                preBuffer->pop();
                if (ggCand) ggCInc -= 1;
            }
        }
        if (ggCand && !explicitGG) explicitGG = new char[1];  // 打ち間違い救済
        reset();
        return;

    case ESC_KEY:
    case CG_KEY:
        switch (key) {
        case CG_KEY:  vk = 'G'; break;
        case ESC_KEY:
        default: vk = VK_ESCAPE; break;
        }
        if (ISEMPTY) {
            if (ISRESET) { preBuffer->pushSoft(MV(vk)); }
        } else {
            if (ISRESET) { preBuffer->clear(); } // XXX これはキツイ?
        }
        reset();
        return;

    case TAB_KEY:
    case CI_KEY:
        switch (key) {
        case CI_KEY:  vk = 'I'; break;
        case TAB_KEY:
        default: vk = VK_TAB; break;
        }
        if (ISRESET) {
            if (ISEMPTY) {
                preBuffer->pushSoft(MV(vk));
            } else {
                nferHirakata();
                addToHistMaybe(preBuffer->string());
            }
        } else {
            preBuffer->pushSoft(MV(vkey[TC_UNSHIFT(currentStroke->at(0))]));
        }
        reset();
        return;

    default:
        break;
    }

    /* 以下 T-Code キー
     * ----------------
     * T-Code キーの場合は、テーブルを引いて、その定義に従う:
     * - 空定義     : 状態をリセット
     * - 文字列     : 定義された文字を、必要に応じて、ひらがな/かたかな変換、
     *                または、半角/全角変換し、preBuffer に格納する
     * - 入れ子の表 : 状態遷移する
     * - 特殊定義   : 定義に従った動作
     */
    currentStroke->push_back(key);
    Block *nextBlock = (currentBlock->block)[key];
    if (OPT_shiftKana && !nextBlock) nextBlock = (currentBlock->block)[TC_UNSHIFT(key)];
    //<record>
    record.nstroke += 1;
    //</record>

    if (nextBlock == NULL) { reset(); return; }

    int n = 0;                  // 後置型の交ぜ書き変換の読みの長さ
    switch (nextBlock->kind()) {
    case STRING_BLOCK:
        {
            char *str = ((StringBlock *)nextBlock)->str;
            MojiBuffer mb(TC_BUFFER_SIZE);
            mb.clear();
            for (char *s = str; *s; ) {
                MOJI m = str2moji(s, &s);
                if (currentShift) { m = mojiHirakata(m); }
                if (punctMode)    { m = mojiPunct(m); }
                //<hankana>
                //if (hanzenMode)   { m = mojiHanzen(m); }
                //if (hirakataMode) { m = mojiHirakata(m); }
                // 「全角かな → 半角かな」の変換 (mijiHandaku(...))。
                // 半角かな変換は、濁音や半濁音等で文字数が変化するので、
                // 他の変換系の操作 (mojiHOGE(m) の類) より後に処理する。
                if (hirakataMode) { m = mojiHirakata(m); }
                if (hanzenMode) {
                    if (OPT_enableHankakuKana) {
                        MOJI han, daku;
                        if (mojiHankana(m, &han, &daku)) {
                            if (daku) { mb.pushSoft(han); m = daku; }
                            else      { m = han; }
                        } else {
                            m = mojiHanzen(m);
                        }
                    } else {
                        m = mojiHanzen(m);
                    }
                }
                //</hankana>
                mb.pushSoft(m);
            }
            str = mb.string();
            //<record>
            record.nchar += mb.length();
            {
                int len = mb.length();
                for (int i = 0; i < len; i++) {
                    statCount(mb.moji(i), STAT_DIRECT);
                }
            }
            //</record>
            preBuffer->pushSoft(str);
            if (ggCand) { ggCInc += mb.length(); }
        }
        reset(); return;

    case CONTROL_BLOCK:
        // ここは reset() しないこと
        currentBlock = (ControlBlock *)nextBlock;
        return;

    case SPECIAL_BLOCK:
        /* 以下、特殊定義
         * - @s     : 互換性のため、@K と同じ扱い
         * - @K @Z  : ひらがな/かたかなモード、半角/全角モードをトグル
         * - @m @b  : 変換開始マークを preBuffer に入れる
         * - @!     : ヒストリ入力モードに遷移
         * - @q     : preBuffer の内容をクリアし、通常入力モードに戻す
         * - @h @H  : helpMode をセットし、ポインタをセットする
         * - @v     : 第一打鍵を preBuffer に入れる
         * - @B @1..@9 : 変換の読み・部品を postBuffer から取ってきて、
         *               変換開始マークとともに preBuffer に入れる。
         *               また、遅延デリート量を設定する。
         * - @D @P  : 直前の入力文字を postBuffer から取ってきて、
         *            濁音または半濁音変換して preBuffer に入れる。
         *            また、遅延デリート量を設定する。
         */
        n = 0;                  // 後置型の交ぜ書き変換の読みの長さ
        switch (((SpecialBlock *)nextBlock)->function) {
        case F_HANZEN:
            hanzenMode ^= 1;
            //<record>
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_SWITCH_MODE:     // OBSOLETE
        case F_HIRAKATA:
            hirakataMode ^= 1;
            //<record>
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_PUNCT:
            punctMode ^= 1;
            //<record>
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_MAZE2GG:
            OPT_maze2gg ^= 1;
            clearCandGG();
            //<record>
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_BUSHU_PRE:
            //<v127c - postInPre>
            //postDelete = 0;
            //</v127c>
            if (bushuReady != 0) { preBuffer->pushSoft(MOJI_BUSHU); }
            else                 { preBuffer->pushSoft("◆"); }
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_MAZE_PRE:
            //<v127c - postInPre>
            //postDelete = 0;
            //</v127c>
            if (mazeReady != 0) { preBuffer->pushSoft(MOJI_MAZE); }
            else                { preBuffer->pushSoft("◇"); }
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_HIST:
            reset();
            mode = HIST;        // XXX needs check?
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            return;

        case F_QUIT:
            preBuffer->clear();
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_HELP_BACKW:
            {
                int len = helpBuffer->length();
                if (len == 0) { reset(); return; }
                if (helpModeSave == 0) {
                    helpOffset = len - 1;
                } else {
                    helpOffset = (helpOffset + len - 1) % len;
                }
            }
            reset(); helpMode = 1;
            helpModeSave = 1;
            //<record>
            record.nspecial += 1;
            //</record>
            return;

        case F_HELP_FORW:
            {
                int len = helpBuffer->length();
                if (len == 0) { reset(); return; }
                if (helpModeSave == 0) {
                    helpOffset = len - 1;
                } else {
                    helpOffset = (helpOffset + len + 1) % len;
                }
            }
            reset(); helpMode = 1;
            helpModeSave = 1;
            //<record>
            record.nspecial += 1;
            //</record>
            return;

        case F_VERB_FIRST:
            preBuffer->pushSoft(MV(vkey[TC_UNSHIFT(currentStroke->at(0))]));
            //<record>
            // XXX: ?
            //record.nspecial += 1;
            //</record>
            reset(); return;

        case F_BUSHU_POST:
            reset();
            if (bushuReady == 0) { return; }
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            if (!ISEMPTY) {
                //<v127c - postInPre>
                //// 前置変換との組み合わせは未実装
                // 前置変換との組み合わせ
                if (preBuffer->length() < 3) return;
                for (int i = -1; -2 <= i; i--) {
                    MOJI m = preBuffer->moji(i);
                    if (mojitype(m) == MOJI_SPECIAL) return;
                }
                postInPre = 2;
                strcpy(yomi, preBuffer->string(-2));
                preBuffer->popN(2);
                preBuffer->pushSoft(MOJI_BUSHU);
                preBuffer->pushSoft(yomi);
                //</v127c>
                return;
            }
            if (postBuffer->length() < 2) { return; }
            postDelete = 2;
            strcpy(yomi, postBuffer->string(-postDelete));
            preBuffer->pushSoft(MOJI_BUSHU);
            preBuffer->pushSoft(yomi);
            return;

        case F_MAZE_POST9: n += 1; /* go down */
        case F_MAZE_POST8: n += 1; /* go down */
        case F_MAZE_POST7: n += 1; /* go down */
        case F_MAZE_POST6: n += 1; /* go down */
        case F_MAZE_POST5: n += 1; /* go down */
        case F_MAZE_POST4: n += 1; /* go down */
        case F_MAZE_POST3: n += 1; /* go down */
        case F_MAZE_POST2: n += 1; /* go down */
        case F_MAZE_POST1: n += 1; // この時点で n == 1 ～ 9
            reset();
            if (mazeReady == 0) { return; }
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            if (!ISEMPTY) {
                //<v127c - postInPre>
                //// 前置変換との組み合わせは未実装
                // 前置変換との組み合わせ
                if (preBuffer->length() < n + 1) return;
                for (int i = -1; -n <= i; i--) {
                    MOJI m = preBuffer->moji(i);
                    if (mojitype(m) == MOJI_SPECIAL) return;
                }
                postInPre = n;
                strcpy(yomi, preBuffer->string(-n));
                preBuffer->popN(n);
                preBuffer->pushSoft(MOJI_MAZE);
                preBuffer->pushSoft(yomi);
                preBuffer->pushSoft((MOJI)' '); // 変換終了マーク
                //</v127c>
                return;
            }
            if (postBuffer->length() < n) { return; }
            postDelete = n;
            strcpy(yomi, postBuffer->string(-postDelete));
            preBuffer->pushSoft(MOJI_MAZE);
            preBuffer->pushSoft(yomi);
            preBuffer->pushSoft((MOJI)' '); // 変換終了マーク
            return;

#define DAKUTEN_HANDAKUTEN(MOJI_CHANGE_FUNC             \
        /**<hankana>**/ , DAKU2, DAKU1 /**</hankaha>**/ \
        )                                               \
    do {                                                \
        MOJI m0, m1;                                    \
        if (ISEMPTY) {                                  \
            if (postBuffer->length() < 1) { return; }   \
            m0 = postBuffer->moji(-1);                  \
            m1 = MOJI_CHANGE_FUNC(m0);                  \
            /**<hankana>**/                             \
            /*                                          \
            if (m0 == m1) { return; }                   \
            */                                          \
            if (m0 == m1) {                             \
                if (hanzenMode) { preBuffer->pushSoft(DAKU1); } \
                else {            preBuffer->pushSoft(DAKU2); } \
                return;                                 \
            }                                           \
            /**</hankana>**/                            \
            preBuffer->pushSoft(MV(VK_BACK));           \
            preBuffer->pushSoft(m1);                    \
            return;                                     \
        } else {                                        \
            m0 = preBuffer->moji(-1);                   \
            m1 = MOJI_CHANGE_FUNC(m0);                  \
            /**<hankana>**/                             \
            /*                                          \
            if (m0 == m1) { return; }                   \
            */                                          \
            if (m0 == m1) {                             \
                if (hanzenMode) { preBuffer->pushSoft(DAKU1); } \
                else {            preBuffer->pushSoft(DAKU2); } \
                return;                                 \
            }                                           \
            /**</hankana>**/                            \
            preBuffer->pop();                           \
            preBuffer->pushSoft(m1);                    \
            return;                                     \
        }                                               \
    } while (0)

        case F_DAKUTEN:
            //<record>
            // XXX: ?
            ////record.nspecial += 1;
            //</record>
            reset();
            if (OPT_enableHankakuKana) {
                DAKUTEN_HANDAKUTEN(mojiDaku, "゛", "ﾞ");
            } else {
                DAKUTEN_HANDAKUTEN(mojiDaku, "゛", "゛");
            }

        case F_HANDAKUTEN:
            //<record>
            // XXX: ?
            ////record.nspecial += 1;
            //</record>
            reset();
            if (OPT_enableHankakuKana) {
                DAKUTEN_HANDAKUTEN(mojiHandaku, "゜", "ﾟ");
            } else {
                DAKUTEN_HANDAKUTEN(mojiHandaku, "゜", "゜");
            }

#undef DAKUTEN_HANDAKUTEN

        default:                // 未実装の SPECIAL
            reset();
            //<record>
            // XXX: ?
            ////record.nspecial += 1;
            //</record>
            return;
        } // switch block function

    default:                    // ここにも来ないはず
        ;
    } // switch block kind
                                // ここにも来ないはず
}

/* 交ぜ書き変換の候補選択モードでのキー入力
 * ----------------------------------------
 * - RET    : 無変換で確定
 * - BS     : 前候補群。ただし、最初の候補群まで戻ったら、
 *            NORMAL モード (読み入力モード) に戻す
 * - ESC    : NORMAL モードに戻す
 * - TAB    : ひらがな/かたかな変換して、確定
 * - <      : 読みを伸ばす
 * - >      : 読みを縮める
 *
 * - SPC    : 次候補換
 * - その他のキー : たぶん候補選択キーなので、対応する候補を preBuffer に挿入
 */
void TCode::keyinCand(int key) {
    // 何かキー入力があったら、まずヘルプを非表示にすること
    helpMode = 0;

    switch (key) {
    case RET_KEY:
    case CM_KEY:
    case CJ_KEY:
        mode = NORMAL;
        nfer();
        return;

    case BS_KEY:
    case CH_KEY:
        if (candSkip <= candOffset) { candOffset -= candSkip; }
        //<v127c - postInPre>
        //else                        { mode = NORMAL; }
        else {
            mode = NORMAL;
            if (postInPre) cancelPostInPre(yomiLen);
        }
        //</v127c>
        return;

    case ESC_KEY:
    case CG_KEY:
        //<v127c - postInPre>
        //mode = NORMAL; return;
        mode = NORMAL;
        if (postInPre) cancelPostInPre(yomiLen);
        return;
        //</v127c>

    case TAB_KEY:
    case CI_KEY:
        mode = NORMAL;
        nferHirakata();
        return;

    case LT_KEY:
        if (OPT_conjugationalMaze != 2) return;
        makeMazeYomiLonger();
        return;

    case GT_KEY:
        if (OPT_conjugationalMaze != 2) return;
        makeMazeYomiShorter();
        return;

    default:
        break;
    }

    /* 以下 T-Code キー
     *
     * ここにたどりつくということは、候補が 2 つ以上あるときだけ。
     * また、reduceByMaze() を経由しているはずで、yomi と yomiLen に
     * 正しく値が設定されていると仮定。
     */

    // シフト打鍵
    //currentShift |= TC_ISSHIFTED(key);
    /* currentShift &= TC_ISSHIFTED(key); 8 */
    key = TC_UNSHIFT(key);

    // SPC なら次候補群
    if (vkey[key] == VK_SPACE) {
        candOffset += candSkip;
        if (currentCand->size() <= candOffset) { candOffset = 0; }
        return;
    }

    // 非選択キーは無視
    if (candOrder[key] < 0) { return; }

    // 無効な選択キーも無視
    unsigned int oc = candOffset + candOrder[key];
    if (currentCand->size() <= oc) { return; }

    cand = (*currentCand)[oc];

    finishCand(cand);

    mode = NORMAL;
    return;
}

/* 交ぜ書き変換の候補が唯一つの場合でのキー入力
 * ----------------------------------------
 * - RET    : 確定
 * - BS     : NORMAL モードに戻す
 * - ESC    : NORMAL モードに戻す
 * - TAB    : ひらがな/かたかな変換して、確定
 * - <      : 読みを伸ばす
 * - >      : 読みを縮める
 *
 * - その他のキー : 確定後 NORMAL モードに引き継ぐ
 */
void TCode::keyinCand1(int key) {
    // 何かキー入力があったら、まずヘルプを非表示にすること
    helpMode = 0;

    switch (key) {
    case RET_KEY:
    case CM_KEY:
    case CJ_KEY:
        cand = (*currentCand)[0];
        finishCand(cand);
        mode = NORMAL;
        return;

    case BS_KEY:
    case CH_KEY:
    case ESC_KEY:
    case CG_KEY:
        //<v127c - postInPre>
        //mode = NORMAL; return;
        mode = NORMAL;
        if (postInPre) cancelPostInPre(yomiLen);
        return;
        //</v127c>

    case TAB_KEY:
    case CI_KEY:
        mode = NORMAL;
        nferHirakata();
        return;

    case LT_KEY:
        if (OPT_conjugationalMaze != 2) return;
        makeMazeYomiLonger();
        if (OPT_maze2gg) { ggCand = currentCand; setCandGGHeader(); }
        return;

    case GT_KEY:
        if (OPT_conjugationalMaze != 2) return;
        makeMazeYomiShorter();
        if (OPT_maze2gg) { ggCand = currentCand; setCandGGHeader(); }
        return;

    default:
        break;
    }

    /* 以下 T-Code キー
     */
    cand = (*currentCand)[0];
    finishCand(cand);
    mode = NORMAL;
    keyinNormal(key);
}

/* ヒストリ入力モードでのキー入力
 * ------------------------------
 * - 無効な選択キー : 通常入力モードに戻す
 * - 有効な選択キー : 対応する候補の参照ビットをセットし、
 *                    候補を preBuffer に入れる
 */
void TCode::keyinHist(int key) {
    // 何かキー入力があったら、まずヘルプを非表示にすること
    helpMode = 0;

    // シフト打鍵
    //currentShift |= TC_ISSHIFTED(key);
    /* currentShift &= TC_ISSHIFTED(key); 8 */
    key = TC_UNSHIFT(key);

    // 無効な選択キーは無視
    if (40 <= key) { mode = NORMAL; return; }

    int k = key % 10;
    char *cand = hist[k];
    if (cand != NULL) {
        histRef[k] = 1; /* histRef[k] += 1; */
        preBuffer->pushSoft(cand);
        //<record>
        {
            char *s = cand;
            for (char *s = cand; *s; ) {
                MOJI m = str2moji(s, &s);
                ////record.nhist += 1;
                statCount(m, STAT_AUX);
            }
        }
        //</record>
    }
    mode = NORMAL; return;
}

/* -------------------------------------------------------------------
 * 補助入力
 * --------
 */

/* isReducibleByBushu()
 * --------------------
 * preBuffer が部首合成変換可能かどうか。
 * すなわち、preBuffer の末尾が、「◆ <文字> <文字>」の形をしているか。
 */
int TCode::isReducibleByBushu() {
    if (mode != NORMAL) { return 0; }
    if (preBuffer->length() < 3) { return 0; }
    for (int i = -1; -2 <= i; i--) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) == MOJI_SPECIAL) { return 0; }  // XXX チェック甘
    }
    if (preBuffer->moji(-3) != MOJI_BUSHU) { return 0; }
    return 1;
}

/* reduceByBushu()
 * ---------------
 * isReducibleByBushu() == 1 である時に、
 * preBuffer の内容の末尾に対して、部首合成入力を実行する。
 */
void TCode::reduceByBushu() {
    MOJI m1 = preBuffer->moji(-2);
    MOJI m2 = preBuffer->moji(-1);
    MOJI m = bushuDic->look(m1, m2, OPT_bushuAlgo);
    if (m == 0) {               // 変換失敗 XXX 0?
        //<v127c - postInPre>
        //if (postDelete != 0) {  // 後置
        if (postInPre) {        // 前置中の後置
            cancelPostInPre(2);
            return;
        } else if (postDelete > 0 && preBuffer->length() == 3) {
                                // 後置
        //</v127c>
            preBuffer->popN(3);
            postDelete = 0;
            //<v127c - postInPre>
            // 次の一行は余計だったようだ
            //preBuffer->pop();
            //</v127c>
            return;
        } else {                // 前置
            preBuffer->pop();   // 失敗の元を取り除く
            return;
        }
    }
    // 変換成功
    //<v127c - postInPre>
    //if (postDelete != 0) {      // 後置
    //    preBuffer->popN(3);
    //    preBuffer->pushSoftN(MV(VK_BACK), postDelete); postDelete = 0;
    //    preBuffer->pushSoft(m);
    //    if (isComplete()) { addToHelpBufferMaybe(m); }
    //    if (isComplete()) { addToHistMaybe(m); }
    //    return;
    //} else {
    //    preBuffer->popN(3);
    //    preBuffer->pushSoft(m);
    //    if (isComplete()) { addToHelpBufferMaybe(m); }
    //    if (isComplete()) { addToHistMaybe(m); }
    //    return;
    //}
    //// ここには来ないはず
    preBuffer->popN(3);
    if (postInPre) {            // 前置中の後置
        postInPre = 0;
    } else if (postDelete > 0 && preBuffer->length() == 0) {
                                // 後置
        preBuffer->pushSoftN(MV(VK_BACK), postDelete); postDelete = 0;
    }
    preBuffer->pushSoft(m);
    //<record>
    record.nbushu += 1;
    statCount(m, STAT_AUX);
    //</record>
    if (isComplete()) { addToHelpBufferMaybe(m); }
    if (isComplete()) { addToHistMaybe(m); }
    return;
    //</v127c>
}

/* isReducibleByMaze()
 * -------------------
 * preBuffer の内容が交ぜ書き変換可能かどうか。
 * すなわち、preBuffer の末尾が「◇ <読み> <空白>」の形をしているか。
 */
int TCode::isReducibleByMaze() {
    if (mode != NORMAL) { return 0; }
    if (preBuffer->length() < 3) { return 0; }
    MOJI m = preBuffer->moji(-1);
    // 次の行は全角の空白を含んでいる
    if (m != MC(' ') && m != MS("　") && m != MV(VK_SPACE)) { return 0; }

    int len = preBuffer->length();
    int offset;
    for (offset = len - 2; 0 <= offset; offset--) {
        m = preBuffer->moji(offset);
        if (m == MOJI_MAZE) {
            yomiLen = len - offset - 2;
            return 1;
        }
        if (mojitype(m) == MOJI_SPECIAL) { return 0; }  // XXX チェック甘
    }
    return 0;
}

/* reduceByMaze()
 * --------------
 * isReducibleByMaze() == 1 である時に、
 * preBuffer の末尾に対して、交ぜ書き変換を実行する。
 */
void TCode::reduceByMaze() {
    preBuffer->pop();           // 変換終了マーク
    strcpy(yomi, preBuffer->string(-yomiLen));
    int check = 0;
    if (OPT_conjugationalMaze == 2) {
        for (okuriLen=0; okuriLen < yomiLen; okuriLen++) {
            int len = (okuriLen ? strlen(preBuffer->string(-okuriLen)) : 0);
            check = mgTable->setCand(yomi, OPT_conjugationalMaze, len);
            if (check > 0) break;
        }
    } else {
        check = mgTable->setCand(yomi, OPT_conjugationalMaze);
    }
    if (check == 0) {
        //<v127c - postInPre>
        //if (postDelete != 0) {
        //    preBuffer->popN(postDelete); postDelete = 0;
        if (postInPre) {        // 前置中の後置
            cancelPostInPre(yomiLen);
        } else if (postDelete > 0 && preBuffer->length() == yomiLen + 1) {
                                // 後置
            preBuffer->popN(yomiLen); postDelete = 0;
        //</v127c>
            preBuffer->pop();   // ◇
        }
        return;
    }
    currentCand = mgTable->cand; candOffset = 0;

    if (OPT_maze2gg) {
        mode = CAND1;
        ggCand = currentCand;
        setCandGGHeader();
        return;
    }
    if (currentCand->size() == 1) {
        if (OPT_conjugationalMaze == 2) {
            mode = CAND1;
        } else {
            cand = (*currentCand)[0];
            finishCand(cand);
        }
    } else {
        mode = CAND;
    }
}

void TCode::finishCand(char *cand) {
    if (OPT_maze2gg) {
        goCandGG();
        return;
    }
    MojiBuffer candBuffer(strlen(cand));
    int headerLen;
    candBuffer.pushSoft(cand);
    for (headerLen = 0; headerLen < yomiLen && headerLen < candBuffer.length(); headerLen++) {
        if (candBuffer.moji(headerLen) != preBuffer->moji(-yomiLen+headerLen)) break;
    }
    MojiBuffer okuri(okuriLen);
    if (OPT_conjugationalMaze == 2 && okuriLen > 0) {
        okuri.pushSoft(preBuffer->string(-okuriLen));
    }
    preBuffer->popN(yomiLen + 1);
    //<v127c - postInPre>
    //preBuffer->pushSoftN(MV(VK_BACK), postDelete); postDelete = 0;
    if (postInPre) {
        postInPre = 0;
    } else if (postDelete > 0 && preBuffer->length() == 0) {
        //preBuffer->pushSoftN(MV(VK_BACK), postDelete);
        preBuffer->pushSoftN(MV(VK_BACK), postDelete-headerLen);
        cand = candBuffer.string(headerLen);
        postDelete = 0;
    }
    //</v127c>
    preBuffer->pushSoft(cand);
    if (OPT_conjugationalMaze == 2 && okuriLen > 0) {
        preBuffer->pushSoft(okuri.string(-okuriLen));
    }
    //<record>
    {
        char *s = cand;
        for (char *s = cand; *s; ) {
            MOJI m = str2moji(s, &s);
            record.nmaze += 1;
            statCount(m, STAT_AUX);
        }
    }
    //</record>
    if (isComplete()) { addToHelpBufferMaybe(cand, yomi); }
    if (isComplete()) { addToHistMaybe(cand); }
}

void TCode::makeMazeYomiLonger() {
    //現状では全長が一定なので送り仮名が短くなる方向に
    int check = 0;
    int oL;
    for (oL=okuriLen-1; oL >= 0; oL--) {
        int len = (oL ? strlen(preBuffer->string(-oL)) : 0);
        check = mgTable->setCand(yomi, OPT_conjugationalMaze, len);
        if (check > 0) break;
    }
    if (check == 0) {
        int len = (okuriLen ? strlen(preBuffer->string(-okuriLen)) : 0);
        mgTable->setCand(yomi, OPT_conjugationalMaze, len);
        currentCand = mgTable->cand; candOffset = 0;
        return;
    } else {
        okuriLen = oL;
    }
    currentCand = mgTable->cand; candOffset = 0;
    if (OPT_maze2gg) { return; }
    if (currentCand->size() == 1) mode = CAND1;
    else mode = CAND;
}

void TCode::makeMazeYomiShorter() {
    //現状では全長が一定なので送り仮名が長くなる方向に
    int check = 0;
    int oL;
    for (oL=okuriLen+1; oL < yomiLen; oL++) {
        int len = (oL ? strlen(preBuffer->string(-oL)) : 0);
        check = mgTable->setCand(yomi, OPT_conjugationalMaze, len);
        if (check > 0) break;
    }
    if (check == 0) {
        int len = (okuriLen ? strlen(preBuffer->string(-okuriLen)) : 0);
        mgTable->setCand(yomi, OPT_conjugationalMaze, len);
        currentCand = mgTable->cand; candOffset = 0;
        return;
    } else {
        okuriLen = oL;
    }
    currentCand = mgTable->cand; candOffset = 0;
    if (OPT_maze2gg) { return; }
    if (currentCand->size() == 1) mode = CAND1;
    else mode = CAND;
}

/* nfer()
 * ------
 * preBuffer から、すべての変換開始マークを取り除く。
 * その結果、isReducibleBy*() == 0 && isComplete() == 1 となるので、
 * TableWindow::output() によって、preBuffer の内容がまとめて出力される。
 */
void TCode::nfer() {
    MojiBuffer tmpMB(TC_BUFFER_SIZE);
    tmpMB.clear();
    for (int i = 0; i < preBuffer->length(); i++) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) != MOJI_SPECIAL) {
            tmpMB.pushSoft(m);
        }
    }
    preBuffer->clear();
    preBuffer->pushSoftN(MV(VK_BACK), postDelete); postDelete = 0;
    for (int i = 0; i < tmpMB.length(); i++) {
        preBuffer->pushSoft(tmpMB.moji(i));
    }
    //<record>
    // NOP
    //</record>
}

/* nferHirakata()
 * --------------
 * ひらがな/かたかな変換を行うほかは、nfer() と同じ
 */
void TCode::nferHirakata() {
    MojiBuffer tmpMB(TC_BUFFER_SIZE);
    tmpMB.clear();
    for (int i = 0; i < preBuffer->length(); i++) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) != MOJI_SPECIAL) {
            tmpMB.pushSoft(mojiHirakata(m));
        }
    }
    preBuffer->clear();
    preBuffer->pushSoftN(MV(VK_BACK), postDelete); postDelete = 0;
    for (int i = 0; i < tmpMB.length(); i++) {
        preBuffer->pushSoft(tmpMB.moji(i));
    }
    //<record>
    // NOP
    //</record>
}

/* isComplete()
 * ------------
 * preBuffer の内容が、もうこれ以上変換が実行できない形かどうか。
 */
int TCode::isComplete() {
    for (int i = 0; i < preBuffer->length(); i++) {
        if (mojitype(preBuffer->moji(i)) == MOJI_SPECIAL) { return 0; }
    }
    return 1;
}

//<v127c - postInPre>
void TCode::cancelPostInPre(int n) {
    strcpy(yomi, preBuffer->string(-n));
    preBuffer->popN(n + 1);
    preBuffer->pushSoft(yomi);
    postInPre = 0;
}
//</v127c>

void TCode::setCandGGHeader()
{
    MojiBuffer ggBuffer(TC_BUFFER_SIZE);
    int i, j;
    ggBuffer.pushSoft((*ggCand)[0]);
    for (i = 1; i < ggCand->size(); i++) {
        MojiBuffer ccand(strlen((*ggCand)[i]));
        ccand.pushSoft((*ggCand)[i]);
        for (j = 0; j < ccand.length(); j++) {
            if (j == ggBuffer.length()) break;
            if (ccand.moji(j) != ggBuffer.moji(j)) {
                ggBuffer.popN(ggBuffer.length()-j);
                break;
            }
        }
    }
    for (i = 0; i < ggBuffer.length(); i++) {
        if (!stTable->look(ggBuffer.moji(i))) continue;
        for (j = 0; j < yomiLen; j++) {
            if (ggBuffer.moji(i) == preBuffer->moji(-yomiLen+j)) break;
        }
        if (j < yomiLen) continue;
        break;
    }
    ggCInc = i;
}

void TCode::goCandGG() {
    MojiBuffer candBuffer(strlen((*currentCand)[0])); // 頭からggCIncまでは全候補同じ
    int headerLen;
    candBuffer.pushSoft((*currentCand)[0]);
    for (headerLen = 0; headerLen < ggCInc; headerLen++) {
        if (candBuffer.moji(headerLen) != preBuffer->moji(-yomiLen+headerLen)) break;
    }
    preBuffer->popN(yomiLen + 1);
    if (postInPre) {        // 前置中の後置
        postInPre = 0;
        preBuffer->pushSoft(candBuffer.string(0, ggCInc));
    } else if (postDelete > 0 && preBuffer->length() == 0) {
                            // 後置
        preBuffer->pushSoftN(MV(VK_BACK), postDelete-headerLen);
        preBuffer->pushSoft(candBuffer.string(headerLen, ggCInc-headerLen));
        postDelete = 0;
    } else {                // 前置
        preBuffer->pushSoft(candBuffer.string(0, ggCInc));
    }
}

void TCode::makeCandGG() {
    MojiBuffer ggBuffer(TC_BUFFER_SIZE+ggCand->size()), ggBuffer2(ggCand->size());
    int i, j;
    if (explicitGG) delete [] explicitGG;
    explicitGG = 0;
    for (i = 0; i < ggCand->size(); i++) {
        MojiBuffer ccand(strlen((*ggCand)[i]));
        ccand.pushSoft((*ggCand)[i]);
        for (j = 0; j < ccand.length(); j++) {
            if (ggBuffer.isEmpty()) {
                if (j < ggCInc) {
                    if (preBuffer->length() > 0) {
                        if (preBuffer->moji(-(mode==CAND1?yomiLen:ggCInc)+j) != ccand.moji(j)) break;
                    } else {
                        if (postBuffer->moji(-(mode==CAND1?yomiLen:ggCInc)+j) != ccand.moji(j)) break;
                    }
                } else if (j == ggCInc) {
                    if (ggBuffer2.isEmpty()) ggBuffer.pushSoft(ccand.string());
                    else ggBuffer2.pushSoft(ccand.moji(j));
                }
            } else if (j < ggCInc) {
                if (ccand.moji(j) != ggBuffer.moji(j)) break;
            } else {
                if (j == ggBuffer.length()) {
                    ggBuffer2.pushSoft(ccand.moji(j));
                    break;
                }
                if (ccand.moji(j) != ggBuffer.moji(j)) {
                    ggBuffer2.clear();
                    ggBuffer2.pushSoft(ggBuffer.moji(j));
                    ggBuffer2.pushSoft(ccand.moji(j));
                    ggBuffer.popN(ggBuffer.length()-j);
                    break;
                }
            }
        }
    }
    ittaku = ggBuffer.length();
    ggBuffer.pushSoft(ggBuffer2.string());
    if (ggBuffer.isEmpty()) { return; }
    explicitGG = new char[ggBuffer.length()*2+1];
    strcpy(explicitGG, ggBuffer.string());
}

void TCode::clearCandGG() {
    ggCand = 0;
    ggCInc = 0;
    ittaku = 0;
    if (explicitGG) delete [] explicitGG;
    explicitGG = 0;
}

#undef MS
#undef MC
#undef MV

/* -------------------------------------------------------------------
 * 文字ヘルプ
 * ----------
 * 部首合成変換や交ぜ書き変換で入力した文字のうち、文字ヘルプの対象とする
 * ものだけを、helpBuffer に格納する。
 * 文字ヘルプの対象にする条件は、
 * - ストロークが存在すること (外字でないこと)、かつ
 * - 交ぜ書き変換の読みに含まれていないこと
 * である。また、次の条件も考慮する:
 * - 同一文字が連続して helpBuffer に入らないようにする
 */

void TCode::addToHelpBuffer(MOJI m) {
    helpBuffer->pushHard(m);
    helpOffset = helpBuffer->length() - 1;
    helpMode = 1; helpModeSave = 1;
}

void TCode::addToHelpBufferMaybe(MOJI m) {
    int check = stTable->look(m);
    if (check == 0) { return; }
    if (helpBuffer->length() != 0 && helpBuffer->moji(-1) == m) {
        helpOffset = helpBuffer->length() - 1;
        helpMode = 1; helpModeSave = 1;
        return;
    }
    addToHelpBuffer(m);
}

void TCode::addToHelpBufferMaybe(char *kanji, char *kana) {
    MojiBuffer candMB(TC_BUFFER_SIZE), yomiMB(TC_BUFFER_SIZE);
    int candLen;
    candMB.clear(); candMB.pushHard(kanji); candLen = candMB.length();
    yomiMB.clear(); yomiMB.pushHard(kana);

    for (int ic = candLen - 1; 0 <= ic; ic--) {
        int iy;
        for (iy = 0; iy < yomiLen; iy++) {
            if (candMB.moji(ic) == yomiMB.moji(iy)) { break; }
        }
        if (iy == yomiLen) { addToHelpBufferMaybe(candMB.moji(ic)); }
    }
}

/* -------------------------------------------------------------------
 * ヒストリ入力
 * ------------
 * 部首合成変換や交ぜ書き変換で入力した文字 (列) のうち、ヒストリに残す
 * べきものだけを、hist に格納する。
 * ヒストリに残す条件は、
 * - 外字を (少くとも 1 文字) 含むこと、または
 * - 長さが 3 文字以上であること
 * である。また、次の条件も考慮する。
 * - すでにヒストリに存在する候補は、新たに追加しない
 * - ヒストリの内容が 10 個を超えるときは、参照されていないものから上書きする
 * ヒストリ候補の置き換えには、クロックアルゴリズムを利用している。
 * なお、ヒストリに追加しただけでは、参照ビットはセットしない。
 */

void TCode::addToHistMaybe(char *s) {
    int check = 0;
    {
        MojiBuffer mb(TC_BUFFER_SIZE);
        mb.clear(); mb.pushSoft(s);
        if (3 <= mb.length()) { check = 1; goto END_CHECK; }
        for (int i = 0; i < mb.length(); i++) {
            if (stTable->look(mb.moji(i)) == 0) { check = 1; goto END_CHECK; }
        }
        END_CHECK: ;
    }
    if (check == 0) { return; }

    for (int i = 0; i < TC_NHIST; i++) {
        if (hist[i] != NULL && strcmp(hist[i], s) == 0) { return; }
    }

    // clock algorithm
    while (histRef[histPtr] != 0) {
        histRef[histPtr] = 0; /* histRef[histPtr] /= 2; */
        histPtr = (histPtr + 1) % TC_NHIST;
    }
#define p hist[histPtr]
    if (p != NULL) { delete [] p; }
    p = new char[strlen(s) + 1]; strcpy(p, s);
    histPtr = (histPtr + 1) % TC_NHIST;
#undef p
}

void TCode::addToHistMaybe(MOJI m) {
    char s[3]; s[0] = 0;
    moji2strcat(s, m);
    addToHistMaybe(s);
}

/* -------------------------------------------------------------------
 * 仮想鍵盤
 * --------
 * 仮想鍵盤を作る。
 * キー番号 i のキーに対して、
 * - vkbFace[i] : キーのフェイス (キートップに表示する文字)
 * - vkbFG[i]   : 前景色 (文字の色)
 * - vkbBG[i]   : 背景色 (キーの色)
 * を設定する。
 *
 * 5 通りの場合分け:
 * - OFF モード             : 何も表示しない
 * - 文字ヘルプモード       : 文字のストロークで背景色とフェイスを設定
 * - 通常入力モード         : 現在のストロークで背景色を、
 *                            現在の遷移状態でフェイスを設定
 * - 交ぜ書き候補選択モード : 現在の候補群の内容でフェイスを設定
 * - ヒストリ候補選択モード : 現在のヒストリ内容でフェイスを設定、さらに、
 *                            ヒストリポインタの位置を背景色で、
 *                            参照ビットが立っている候補を前景色で明示しておく
 *
 * なお、ヒストリ候補は、交ぜ書き変換の候補選択モードの候補が 10 個以下の場合
 * と同じ扱いにするため、ホーム段の 10 キー、すなわちキー番号 20 ～ 29 のキー
 * に設定している。
 */

//<v127a - gg>
// この2関数はblock.cで実装するのが正しい?
//<gg-defg>
//void TCode::makeGG(MOJI *strGG) {
void TCode::makeGG(char *strGG, int start, int protectOnConflict) {
//</gg-defg>
    char *nums = "２\0３\0４\0５\0６\0７\0８\0９";
    //<gg-defg>
    //for (; *strGG != 0; strGG++) {
    //    int check = stTable->look(*strGG);
    MojiBuffer *s = new MojiBuffer(strlen(strGG));
    s->pushSoft(strGG);
    MOJI m;
    for (; !s->isEmpty();) {
        m = s->pop();
        if (s->length() < start) break;
        int i;
        for (i = 0; i < s->length(); i++) if (s->moji(i) == m) break;
        if (i < s->length()) continue;
        int check = stTable->look(m);
    //</gg-defg>
        if (check == 0) { continue; }
        int stlen = strokelen(stTable->stroke);
        ControlBlock *p = table;
        for (i = 0; i < stlen-1; i++) {
            Block *np = (p->block)[stTable->stroke[i]];
            p = (ControlBlock *)np;
        }
        Block *np = (p->block)[stTable->stroke[stlen-1]];
        ((StringBlock *)np)->flagGG = 1;
        char *face1 = ((StringBlock *)np)->face;
        p = table;
        for (int i = 0; i < stlen-1; i++) {
            Block *np = (p->block)[stTable->stroke[i]];
            p = (ControlBlock *)np;
            if (!p->faceGG || s->length() < protectOnConflict) {  // 候補1個目
                p->faceGG = face1;
            } else if (p->faceGG >= nums && p->faceGG < nums+21) {
                p->faceGG += 3;
            } else {  // 候補2個目
                p->faceGG = nums;
            }
        }
    }
    //<gg-defg>
    delete s;
    //</gg-defg>
}
//</v127a - gg>

//<v127a - gg>
void TCode::clearGG(ControlBlock *p) {
    for (int i = 0; i < TC_NKEYS*2; i++) {
        Block *np = (p->block)[i];
        if (np == NULL) { continue; }
        switch (np->kind()) {
        case STRING_BLOCK:
            ((StringBlock *)np)->flagGG = 0;
            break;
        case CONTROL_BLOCK:
            ((ControlBlock *)np)->faceGG = NULL;
            clearGG((ControlBlock *)np);
            break;
        default:
            break;
        }
    }
}

//</v127a - gg>

void TCode::assignStroke(char *strGG) {
    MojiBuffer *s = new MojiBuffer(strlen(strGG));
    s->pushSoft(strGG);
    MOJI m;
    for (; !s->isEmpty();) {
        m = s->pop();
        int check = stTable->look(m);
        if (check == 0) { assignStroke(m); }
    }
    delete s;
}

void TCode::assignStroke(MOJI m) {
    if (!OPT_prefixautoassign) return;
    int len = strokelen(OPT_prefixautoassign);
    STROKE *st = new STROKE[len + 2];
    memcpy(st, OPT_prefixautoassign, len + 1);
    st[len+1] = EOST;
    ControlBlock *p = table;
    for (int i = 0; i < len; i++) {
        if (!(p->block)[st[i]]) (p->block)[st[i]] = new ControlBlock();
        Block *np = (p->block)[st[i]];
        if (np->kind() != CONTROL_BLOCK) { delete st; return; }
        p = (ControlBlock *)np;
    }
    for (int oc = 0; oc < candSkip; oc++) {
        int i;
        for (i = 0; i < TC_NKEYS; i++) {
            if (candOrder[i] == oc) break;
        }
        if (i >= TC_NKEYS) continue;
        if ((p->block)[i]) continue;
        st[len] = i;
        (*stTable->stMap)[m] = st;
        char c[3];
        c[0] = 0; moji2strcat(c, m);
        (p->block)[i] = new StringBlock(c);
        assignedsBuffer->pushSoft(m);
        return;
    }
    delete st;
}

void TCode::clearAssignStroke() {
    while (!assignedsBuffer->isEmpty()) {
        MOJI m = assignedsBuffer->pop();
        int check = stTable->look(m);
        if (check == 0) continue;
        int stlen = strokelen(stTable->stroke);
        ControlBlock *p = table;
        for (int i = 0; i < stlen-1; i++) {
            Block *np = (p->block)[stTable->stroke[i]];
            p = (ControlBlock *)np;
        }
        Block *np = (p->block)[stTable->stroke[stlen-1]];
        if (np->kind() != STRING_BLOCK) continue;
        delete np;
        (p->block)[stTable->stroke[stlen-1]] = 0;
        delete (*stTable->stMap)[m];
        stTable->stMap->erase(m);
    }

}

void TCode::makeVKB() {
    // 初期化
    for (int i = 0; i < TC_NKEYS; i++) {
        vkbFace[i] = vkbFace[TC_SHIFT(i)] = NULL;
        vkbFG[i] = vkbFG[TC_SHIFT(i)] = TC_FG_NIL;
        vkbBG[i] = TC_BG_NIL;
        vkbCorner[i] = 0;
    }

    // OFF mode
    if (mode == OFF) { return; }

    // helpMode
    if (helpMode) {
        MOJI moji = helpBuffer->moji(helpOffset);
        int check = stTable->look(moji);
        if (check == 0) { return; }
        //<multishift>
        //makeVKBBG(stTable->stroke);
        makeVKBBG(stTable->baseStroke);
        //</multishift>
        for (int i = 0; i < TC_NKEYS; i++) {
            switch (vkbBG[i]) {
            //<multishift>
            //case TC_BG_ST1: vkbFace[i] = "●"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST2: vkbFace[i] = "○"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST3: vkbFace[i] = "△"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STF: vkbFace[i] = "◇"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STW: vkbFace[i] = "◎"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STX: vkbFace[i] = "☆"; vkbFG[i] = TC_FG_STROKE; break;
            // 三枚表 T-Code スタイルの打鍵図のマーク
            //// 右表
            //case TC_BG_ST1R: vkbFace[i] = "▲"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST2R: vkbFace[i] = "○"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STWR: vkbFace[i] = "▲"; vkbFG[i] = TC_FG_STROKE; break;
            // 左表
            //case TC_BG_ST1L: vkbFace[i] = "▽"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST2L: vkbFace[i] = "○"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STWL: vkbFace[i] = "▽"; vkbFG[i] = TC_FG_STROKE; break;
            case TC_BG_ST1:
                vkbFace[i] = stTable->mkSt[0]; vkbFG[i] = TC_FG_STROKE; break;
            case TC_BG_ST2:
                vkbFace[i] = stTable->mkSt[1]; vkbFG[i] = TC_FG_STROKE; break;
            case TC_BG_ST3:
                vkbFace[i] = stTable->mkSt[2]; vkbFG[i] = TC_FG_STROKE; break;
            case TC_BG_STF:
                vkbFace[i] = stTable->mkSt[3]; vkbFG[i] = TC_FG_STROKE; break;
            case TC_BG_STW:
                vkbFace[i] = stTable->mkSt[4]; vkbFG[i] = TC_FG_STROKE; break;
            case TC_BG_STX:
                vkbFace[i] = stTable->mkSt[5]; vkbFG[i] = TC_FG_STROKE; break;
            //</multishift>
            // ホームポジションとその上下段に表示する「・」
            default:
                if (10 <= i && i < 40 && (i % 10 <= 3 || 6 <= i % 10)) {
                    vkbFace[i] = "・";
                    vkbFG[i] = TC_FG_STROKE;
                }
            }
        }
        return;
    } // if helpMode

    // NORMAL mode
    if (mode == NORMAL || mode == CAND1) {
        //<v127a - gg>
        //<gg-defg>
        char *strGG = NULL;
        //</gg-defg>
        //<gg-defg2>
        //if (ggReady) {
        if (currentBlock == table && ggReady) {
        //</gg-defg2>
            // make GGuide
            //<gg-defg>
            //MOJI *strGG = NULL;
            //</gg-defg>
            if (preBuffer->length() > 0) {
                ////strGG = ggDic->look(preBuffer);
                int i, n = preBuffer->length();
                for (i = 0; i < n; i++) {
                    if (preBuffer->moji(i) == MOJI_BUSHU) break;
                }
                if (i < n)  // 部首合成変換には不要
                    ;
                else if (preBuffer->length() == 1)  // 交ぜ書き開始直後なら
                    strGG = ggDic->look(postBuffer);
                else 
                    strGG = ggDic->look(preBuffer);
            } else if (postBuffer->length() > 0) {
                strGG = ggDic->look(postBuffer);
            }
            if (strGG) assignStroke(strGG);
        //<gg-defg>
            //clearGG(table);
            //if (strGG) {
            //    makeGG(strGG);
            //}
        }
        //<gg-defg2>
        //if (OPT_defg && !strGG) strGG = OPT_defg;
        //clearGG(table);
        //if (strGG) {
        //    makeGG(strGG);
        if (currentBlock == table) {
            if (ggCand) makeCandGG();
            if (OPT_defg && !strGG) strGG = OPT_defg;
            if (OPT_defg || ggReady || OPT_maze2gg) {
                clearGG(table);
                if (explicitGG) makeGG(explicitGG, ggCInc, ittaku);
                else if (strGG) makeGG(strGG);
            }
        //</gg-defg2>
        //</gg-defg>
        }
        //</v127a - gg>
        int check = 0;
        if (explicitGG && ggCInc < ittaku) {
            MojiBuffer hoge(strlen(explicitGG));
            hoge.pushSoft(explicitGG);
            MOJI moji = hoge.moji(ggCInc);
            check = stTable->look(moji);
        }
        if (check) makeVKBBG(stTable->stroke);
        else makeVKBBG(currentStroke);
        for (int i = 0; i < TC_NKEYS*2; i++) {
            Block *block = currentBlock->block[i];
            if (block == 0) { continue; }
            switch (block->kind()) {
            case STRING_BLOCK:
                // XXX hirakataMode や hanzenMode での変換は未実装
                vkbFace[i] = block->getFace();
                vkbFG[i] = TC_FG_NORMAL;
                //<v127a - gg>
                if (((StringBlock *)block)->flagGG)
                    vkbFG[i] = TC_FG_GG;
                //</v127a - gg>
                break;
            case CONTROL_BLOCK:
                if (currentBlock != table) {
                    vkbFace[i] = block->getFace();
                    vkbFG[i] = TC_FG_SPECIAL;
                }
                //<v127a - gg>
                if (((ControlBlock *)block)->faceGG) {
                    vkbFace[i] = ((ControlBlock *)block)->faceGG;
                    vkbFG[i] = TC_FG_GG;
                }
                //</v127a - gg>
                break;
            case SPECIAL_BLOCK:
                vkbFace[i] = block->getFace();
                vkbFG[i] = TC_FG_SPECIAL;
                break;
            default:
                break;
            } // switch block->kind()
        } // for i
        return;
    } // if mode NORMAL

    // HIST mode
    if (mode == HIST) {
        for (int i = 0; i < 10; i++) {
            if (hist[i] != NULL) {
                vkbFace[i + 20] = hist[i];
                vkbFG[i + 20] = TC_FG_NORMAL;
                if (histRef[i] != 0) { vkbFG[i + 20] = TC_FG_HISTREF; }
            }
        }
        vkbBG[(histPtr + 9) % 10 + 20] = TC_BG_HISTPTR;
        return;
    } // if mode HIST

    // CAND mode
    if (mode == CAND) {
        for (int i = 0; i < TC_NKEYS; i++) {
            if (candOrder[i] < 0) { continue; }
            unsigned int oc = candOffset + candOrder[i];
            if (oc < currentCand->size()) {
                vkbFace[i] = (*currentCand)[oc];
                vkbFG[i] = TC_FG_NORMAL;
            }
        }
        return;
    } // if mode CAND

}

void TCode::makeVKBBG(vector<STROKE> *vst) {
    int stlen = vst->size();
    STROKE *st = new char[stlen + 1];
    for (int i = 0; i < stlen; i++) {
        st[i] = vst->at(i);
    }
    st[stlen] = EOST;

    makeVKBBG(st);

    delete [] st;
}

void TCode::makeVKBBG(STROKE *st) {
    int stlen = strokelen(st);

    //<multishift>
    //// 三枚表 T-Code スタイルの打鍵図
    //if (OPT_useTTCode && stlen == 4) {
    //    if (st[0] == 26 && st[1] == 23) {
    //        if (st[2] == st[3]) {
    //            vkbBG[st[2]] = TC_BG_STWR;
    //        } else {
    //            vkbBG[st[2]] = TC_BG_ST1R;
    //            vkbBG[st[3]] = TC_BG_ST2R;
    //        }
    //        return;
    //    } else if (st[0] == 23 && st[1] == 26) {
    //        if (st[2] == st[3]) {
    //            vkbBG[st[2]] = TC_BG_STWL;
    //        } else {
    //            vkbBG[st[2]] = TC_BG_ST1L;
    //            vkbBG[st[3]] = TC_BG_ST2L;
    //        }
    //        return;
    //    }
    //} // if OPT_useTT && stlen == 4
    //</multishift>

    int needX = 0;
    for (int th = 0; th < stlen; th++) {
        int k = st[th];
        int isShift = TC_ISSHIFTED(k);
        k = TC_UNSHIFT(k);
        switch (th) {
        case 0:                 // 1st stroke
            if (isShift) vkbCorner[k] |= TC_MK_SH1;
            vkbBG[k] = TC_BG_ST1;
            break;
        case 1:                 // 2nd stroke
            if (vkbBG[k] != TC_BG_NIL) {
                if (isShift) vkbCorner[k] |= TC_MK_SH2;
                vkbBG[k] = TC_BG_STW;
                needX = 1;
            } else {
                if (isShift) vkbCorner[k] |= TC_MK_SH1;
                vkbBG[k] = TC_BG_ST2;
            }
            break;
        case 2:                 // 3rd stroke
            if (vkbBG[k] != TC_BG_NIL) {
                if (needX) {
                    if (isShift) vkbCorner[k] |= TC_MK_SH3;
                    vkbBG[k] = TC_BG_STX;
                } else {
                    if (isShift) vkbCorner[k] |= TC_MK_SH2;
                    vkbBG[k] = TC_BG_STW;
                    needX = 1;
                }
            } else {
                if (isShift) vkbCorner[k] |= TC_MK_SH1;
                vkbBG[k] = TC_BG_ST3;
            }
            break;
        default:                // forth stroke(s)
            if (vkbBG[k] != TC_BG_NIL) {
                if (needX) {
                    if (vkbBG[k] == TC_BG_STW || vkbBG[k] == TC_BG_STX) {
                        if (isShift) vkbCorner[k] |= TC_MK_SH3;
                    } else {
                        if (isShift) vkbCorner[k] |= TC_MK_SH2;
                    }
                    vkbBG[k] = TC_BG_STX;
                } else {
                    if (isShift) vkbCorner[k] |= TC_MK_SH2;
                    vkbBG[k] = TC_BG_STW;
                    needX = 1;
                }
            } else {
                if (isShift) vkbCorner[k] |= TC_MK_SH1;
                vkbBG[k] = TC_BG_STF;
            }
            break;
        } // switch th
    } // for th
}

//<v127a - shiftcheck>
// かな文字を入力するためのストロークの一部となるキー番号iに対する
// isShiftKana[i]の値をtrueにする。
bool TCode::checkShiftKana(ControlBlock *block) {
    bool ret = false;
    for (int i = 0; i < TC_NKEYS; i++) {
        Block *nextBlock = (block->block)[i];
        if (nextBlock == 0)
            continue;
        switch (nextBlock->kind()) {
        case CONTROL_BLOCK:
            if (checkShiftKana((ControlBlock *)nextBlock)) {
                isShiftKana[i] = true;
                ret = true;
            }
            break;
        case STRING_BLOCK:
            if (_ismbchira(str2moji(((StringBlock *)nextBlock)->str, NULL))) {
                isShiftKana[i] = true;
                ret = true;
            }
            break;
        case SPECIAL_BLOCK:
        default:
            break;
        }
    }
    return ret;
}
//</v127a - shiftcheck>
// 任意の文字でシフトと組み合わせられたストロークとなるキー番号iに対する
// isShiftKana[i]の値をtrueにする。
void TCode::checkShiftSeq(ControlBlock *block) {
    for (int i = 0; i < TC_NKEYS; i++) {
        if ((block->block)[TC_SHIFT(i)]) isShiftKana[i] = true;
        Block *nextBlock = (block->block)[i];
        if (nextBlock && nextBlock->kind() == CONTROL_BLOCK)
            checkShiftSeq((ControlBlock *)nextBlock);
        nextBlock = (block->block)[TC_SHIFT(i)];
        if (nextBlock && nextBlock->kind() == CONTROL_BLOCK)
            checkShiftSeq((ControlBlock *)nextBlock);
    }
    return ;
}

//<multishift2>
#define STRCPY(q, p)                                            \
        do {                                                    \
            if (*p != '"') {    /* p : raw str */               \
                strcpy(q, p);                                   \
            } else {            /* p : quoted ("\"hoge\"") */   \
                for (p++; *p != '"' && *p != '\0'; p++, q++) {  \
                    if (IS_ZENKAKU(*p)) { *q++ = *p++; }        \
                    else if (*p == '\\') { p++; }               \
                    *q = *p;                                    \
                }                                               \
                *q = '\0';                                      \
            }                                                   \
        } while (0)

void TCode::readDir(DIR_TABLE *pdt, ifstream *is) {
    char buf[1024], s[1024];
    int n;

    while (!(is->eof())) {
        is->getline(buf, sizeof(buf));
        if (*buf != '#') { continue; }
        if (sscanf(buf, "#define %s %n", &s, &n) != 1) { continue; }

        if (strcmp(s, "table-name") == 0) {
            if ((*pdt)[DIR_table_name]) {
                delete [] (*pdt)[DIR_table_name]; }
            char *p = buf + n;
            char *q = (*pdt)[DIR_table_name] = new char[strlen(p) + 1];
            STRCPY(q, p);

        } else if (strcmp(s, "prefix") == 0) {
            if ((*pdt)[DIR_prefix]) { delete [] (*pdt)[DIR_prefix]; }
            char *p = buf + n;
            char *q = (*pdt)[DIR_prefix] = new char[strlen(p) + 1];
            strcpy(q, p);

        } else if (strcmp(s, "defguide") == 0) {
            if ((*pdt)[DIR_defguide]) { delete [] (*pdt)[DIR_defguide]; }
            char *p = buf + n;
            char *q = (*pdt)[DIR_defguide] = new char[strlen(p) + 1];
            STRCPY(q, p);
        }
    }
}

#undef STRCPY
//</multishift2>

//<record>
void TCode::recordSetup(const char *filename) {
    if (record.OPT_record != 0) { delete record.OPT_record; }
    record.OPT_record = new char[strlen(filename) + 1];
    strcpy(record.OPT_record, filename);

    record.nchar = 0;
    record.nstroke = 0;
    record.nbushu = 0;
    record.nmaze = 0;
    record.nspecial = 0;
}

void TCode::recordOutput() {
    // 必要ないなら何もしなくていい
    if (record.OPT_record == 0) { return; }
    // 入力してないなら何もしなくていい
    if (record.nstroke <= 0) { return; }

    // 現在時刻を取得
    SYSTEMTIME stTime;
    GetLocalTime(&stTime);

    // 出力する行を作成
    char s[128];
    int rbushu   = (100 * record.nbushu  ) / record.nstroke;
    int rmaze    = (100 * record.nmaze   ) / record.nstroke;
    int rspecial = (100 * record.nspecial) / record.nstroke;
    char *month;
    switch (stTime.wMonth) {
    case  1: month = "Jan"; break;
    case  2: month = "Feb"; break;
    case  3: month = "Mar"; break;
    case  4: month = "Apr"; break;
    case  5: month = "May"; break;
    case  6: month = "Jun"; break;
    case  7: month = "Jul"; break;
    case  8: month = "Aug"; break;
    case  9: month = "Sep"; break;
    case 10: month = "Oct"; break;
    case 11: month = "Nov"; break;
    case 12: month = "Dec"; break;
    default: month = "???"; break;
    }
    sprintf(s,
            "%3s %2d %02d:%02d"
            "  文字: %4d"
            "  部首: %3d(%d%%)  交ぜ書き: %3d(%d%%)  機能: %3d(%d%%)\n",
            month, stTime.wDay, stTime.wHour, stTime.wMinute,
            record.nchar,
            record.nbushu,   rbushu,
            record.nmaze,    rmaze,
            record.nspecial, rspecial);

    // ファイルに書き出す
    ofstream *os = new ofstream();
    os->open(record.OPT_record, ios::app);
    if (os->fail()) { return; }
    os->write(s, strlen(s));
    os->close();

    // 後しまつ
    record.nchar = 0;
    record.nstroke = 0;
    record.nbushu = 0;
    record.nmaze = 0;
    record.nspecial = 0;
}
//</record>

//<record>
void TCode::statSetup(const char *filename) {
    if (stat.OPT_stat != 0) { delete stat.OPT_stat; }
    stat.OPT_stat = new char[strlen(filename) + 1];
    strcpy(stat.OPT_stat, filename);

    stat.map.clear();
}

void TCode::statOutput() {
    // 必要ないなら何もしなくていい
    if (stat.OPT_stat == 0) { return; }
    if (stat.map.size() == 0) { return; }

    // ファイルから読み出して加算
    ifstream *is = new ifstream();
    is->open(stat.OPT_stat);
    if (!is->fail()) {
        char line[256], str[256];
        struct STATENT se;
        MOJI m;
        while (!is->eof()) {
            is->getline(line, sizeof(line));
            if (sscanf(line, "%s %d %d", str, &se.direct, &se.aux) == 3) {
                m = str2moji(str, NULL);
                statCount(m, STAT_DIRECT, se.direct);
                statCount(m, STAT_AUX,    se.aux);
            }
        }
        is->close();
    }

    // ファイルに書き出す
    ofstream *os = new ofstream();
    os->open(stat.OPT_stat, ios::trunc);
    if (os->fail()) { return; }
    for (StatMap::iterator it = stat.map.begin();
         it != stat.map.end();
         it++) {
        char line[256];
        char str[3]; str[0] = '\0';
        moji2strcat(str, it->first);
        sprintf(line, "%s\t%d\t%d\n", str,
                it->second.direct, it->second.aux);
        os->write(line, strlen(line));
    }
    os->close();

    // 後しまつ
    stat.map.clear();
}

void TCode::statCount(MOJI m, int how) {
    statCount(m, how, 1);
}

void TCode::statCount(MOJI m, int how, int n) {
    // 必要ないなら何もしなくていい
    if (stat.OPT_stat == 0) { return; }

    // マップ内にあるかチェック
    // XXX
    if (stat.map.find(m) == stat.map.end()) {
        struct STATENT se;
        se.direct = 0;
        se.aux = 0;
        stat.map[m] = se;
    }

    // カウント
    switch (how) {
    case STAT_DIRECT: stat.map[m].direct += n; break;
    case STAT_AUX:    stat.map[m].aux    += n; break;
    default: ;
    }
}
//</record>

/* -------------------------------------------------------------------
 * EOF
 */
