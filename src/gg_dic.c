//<v127a - gg>
#include "gg_dic.h"
#include "debug.h"
#ifdef _MSC_VER //<OKA>
#define for if(0);else for
#endif          //</OKA>

/* --------------------------------------------------------------------
 * 熟語ガイド辞書の書式
 * ====================
 *
 * 熟語ガイド辞書の書式は、新形式と旧形式の 2 種がある。
 * いずれの形式でも、(現在の「漢直Win」の仕様では) ソートされている
 * 必要はないが、重複エントリがあってはならない (たぶん)。
 *
 * 新形式 (『連習スレ 2』の 208-212 番のレス)
 * ------------------------------------------
 *
 * 新形式の辞書は、キーとして任意の長さの文字列に対応している。
 * 各行が次のような形式:
 *     <文字列> <半角空白> <半角スラッシュ> <文字の並び>
 * <文字列> に続く可能性のある文字を、<文字の並び> の部分に並べて指定する。
 * 例えば、
 *     不可 /解限欠思能避分
 * というエントリが辞書にあるとき、「不可」と入力した後に「解限欠思能避分」の
 * 各文字のガイドが表示されることになる。
 *
 * 旧形式 (『連習スレ 2』の 102-106 番のレス)
 * ------------------------------------------
 *
 * 旧形式の辞書は、2 文字の連鎖にのみ対応している。
 * 各行が次のような形式:
 *     <文字> <文字の並び>
 * <文字> に続く可能性のある文字を、<文字の並び> の部分に並べて指定する。
 * 例えば、
 *     本当物日式体命家元
 * というエントリが辞書にあるとき、「本」と入力した後に「当物日式体命家元」の
 * 各文字のガイドが表示されることになる。
 */

GgDic::GgDic() {
    nent = 0;
}

GgDic::~GgDic() {
    //<gg-defg>
    //for (int i = 0; i < nent; i++) { delete(ent[i]); }
    for (int i = 0; i < nent; i++) {
        delete(ent[i][0]);
        //<gg-defg2>
        //if (i == 0 || ent[i-1][1] != ent[i][1]) delete(ent[i][1]);
        if (ent4del[i]) delete(ent4del[i]);
        //</gg-defg2>
    }
    //</gg-defg>
}

//<gg-defg2>
int firstbyteofkey(const void *a, const void *b)
{
    return ((unsigned char **)a)[0][0] - ((unsigned char **)b)[0][0];
}
//</gg-defg2>

void GgDic::readFile(ifstream *is) {
    // 作業領域
    //<gg-defg>
    //char bufferS[2048], bufferD1[2048], bufferD2[2048];
    char bufferS[2048], bufferD1[2048];
    //</gg-defg>
    //int line = 1;

    for (nent = 0; !(is->eof()) && nent < GGDIC_MAXENT; // XXX
        //<gg-defg>
         //nent++) {
         ) {
        char c;
        //</gg-defg>
        // get one line
        is->getline(bufferS, sizeof(bufferS));
        if (*bufferS == 0) { break; } // XXX; ?
        //<gg-defg>
        //if (sscanf(bufferS, "%s /%s\n", bufferD1, bufferD2) == 2) {
        //    char *s1 = bufferD1, *s2 = bufferD2;
        //    int n1 = strlen(s1)/2, n2 = strlen(s2)/2;
        //    ent[nent] = new MOJI[n1+n2+2];
        //    ent[nent][0] = n1;
        //    for (int i = 0; i < n1; i++) {
        //        ent[nent][i+1] = str2moji(s1, &s1);
        //    }
        //    for (int i = 0; i < n2; i++) {
        //        ent[nent][i+1+n1] = str2moji(s2, &s2);
        //    }
        //    ent[nent][n1+n2+1] = 0;
        //} else {  // 後方互換
        if (sscanf(bufferS, "%s %1c", bufferD1, &c) == 1) {
            // 後方互換
            char *s = bufferD1;
            ent[nent][0] = new char[3];
            strncpy(ent[nent][0], s, 2);
            ent[nent][0][2] = 0;
            ent[nent][1] = new char[strlen(s) - 1];
            strcpy(ent[nent][1], s + 2);
            //<gg-defg2>
            ent4del[nent] = ent[nent][1];
            //</gg-defg2>
            nent++;
        } else {
            int i, n;
        //</gg-defg>
            char *s = bufferS;
            //<gg-defg>
            //int n = strlen(s)/2;
            //ent[nent] = new MOJI[n+2];
            //ent[nent][0] = 1;
            //for (int i = 0; i < n; i++) {
            //    ent[nent][i+1] = str2moji(s, &s);
            for (i = 0; sscanf(s, "%s %1c%n", bufferD1, &c, &n) >= 2; i++) {
                s += n - 1;
                ent[nent+i][0] = new char[strlen(bufferD1) + 1];
                strcpy(ent[nent+i][0], bufferD1);
            }
            n = i;
            s = bufferD1;
            if (s[0] == '/') s++;
            ent[nent][1] = new char[strlen(s) + 1];
            strcpy(ent[nent][1], s);
            //<gg-defg2>
            ent4del[nent] = ent[nent][1];
            //</gg-defg2>
            for (i = 1; i < n; i++) {
                ent[nent+i][1] = ent[nent][1];
                //<gg-defg2>
                ent4del[nent+i] = NULL;
                //</gg-defg2>
            //</gg-defg>
            }
            //<gg-defg>
            //ent[nent][n+1] = 0;
            nent += n;
            //</gg-defg>
        }
    }
    //<gg-defg2>
    int i, len;
    char c;
    for (i = 0; i < nent; i++) {
        len = strlen(ent[i][0]);
        c = ent[i][0][0];
        ent[i][0][0] = ent[i][0][len-1];
        ent[i][0][len-1] = c;
    }
    qsort((void *)ent, (size_t)nent, sizeof(ent[0]), firstbyteofkey);
    for (i = 0; i < 256; i++) index[i] = -1;
    index[ent[0][0][0]] = 0;
    for (i = 1; i < nent; i++) {
        if ((unsigned char)ent[i][0][0] > (unsigned char)ent[i-1][0][0]) {
            index[ent[i][0][0]] = i;
        }
    }
    for (i = 0; i < nent; i++) {
        len = strlen(ent[i][0]);
        c = ent[i][0][0];
        ent[i][0][0] = ent[i][0][len-1];
        ent[i][0][len-1] = c;
    }
    //</gg-defg2>
}

// aBuffer の末尾から続く可能性のある漢字の列を返す。
// 見つからなかった場合は NULL を返す。XXX
// 末尾で最長一致したエントリだけ。
//
//<gg-defg>
//MOJI * GgDic::look(MojiBuffer *aBuffer) {
char * GgDic::look(MojiBuffer *aBuffer) {
//</gg-defg>
    int m = 0, n = -1;
    char *p = aBuffer->string();
    int len = strlen(p);
    //<gg-defg2>
    //for (int i = 0; i < nent; i++) {
    if (index[p[len-1]] < 0) return NULL;
    for (int i = index[p[len-1]]; i < nent; i++) {
    //</gg-defg2>
        int j;
        //<gg-defg>
        //if (ent[i][0] > len) continue;
        //if (ent[i][0] <= m) continue;
        //char *s = p+len-ent[i][0]*2;
        //for (j = 0; j < ent[i][0]; j++) {
        //    if (ent[i][j+1] != str2moji(s, &s)) break;
        j = strlen(ent[i][0]);
        //<gg-defg2>
        //if (j > len) continue;
        if (ent[i][0][j-1] != p[len-1]) break;
        //</gg-defg2>
        if (j <= m) continue;
        //<gg-defg2>
        //char *s = p;
        //while (s < p + len - j) {
        //    (void)str2moji(s, &s);
        if (j > len) continue;
        if (strcmp(p + len - j, ent[i][0]) == 0) {
            char *s = p;
            while (s < p + len - j) {
                (void)str2moji(s, &s);
            }
            if (s > p + len - j) continue;
            m = j, n = i;
        //</gg-defg2>
        //</gg-defg>
        }
        //<gg-defg>
        //if (j == ent[i][0]) m = j, n = i;
        //if (s > p + len - j) continue;
        //if (strcmp(s, ent[i][0]) == 0) m = j, n = i;
        //</gg-defg>
    }
    //<gg-defg>
    //if (m > 0 && n >= 0) return ent[n]+1+ent[n][0];
    if (m > 0 && n >= 0) return ent[n][1];
    //</gg-defg>
    return NULL;
}
//</v127a - gg>
