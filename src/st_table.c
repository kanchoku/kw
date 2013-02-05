#ifdef _MSC_VER //<OKA>
#pragma warning(disable:4786)
#if _MSC_VER < 1700
#define for if(0);else for
#endif
#endif          //</OKA>
#include "st_table.h"
#include "tc.h"
#include "debug.h"
// -------------------------------------------------------------------

// �X�g���[�N�� strk �̒�����Ԃ� (EOST �͊܂܂Ȃ�)
int strokelen(const STROKE *st) {
    int i;
    for (i = 0; *(st + i) != EOST; i++) {}
    return i;
}

// �X�g���[�N��̃R�s�[ (EOST ���܂�)
void strokecpy(STROKE *dst, const STROKE *src) {
    while (1) {
        *dst = *src;
        if (*src == EOST) { break; }
        dst++; src++;
    }
}

// -------------------------------------------------------------------
// �R���X�g���N�^�ƃf�X�g���N�^

StTable::StTable(Block *rootBlock) {
    stMap = new StMap();
    stroke = 0;

    init(rootBlock);

    //<multishift>
    // �����w���v�p�̋L���̏�����
    strcpy(defmkSt[0], TC_MK_ST1);
    strcpy(defmkSt[1], TC_MK_ST2);
    strcpy(defmkSt[2], TC_MK_ST3);
    strcpy(defmkSt[3], TC_MK_STF);
    strcpy(defmkSt[4], TC_MK_STW);
    strcpy(defmkSt[5], TC_MK_STX);
    strcpy(defmkTbl, "");
    nprefs = 0;
    baseStroke = 0;
    //</multishift>
}

StTable::~StTable() {
    delete(stMap);
}

// -------------------------------------------------------------------
// ������ : �t�����e�[�u�������

void StTable::init(Block *rootBlock) {
    // ��̃X�g���[�N����ꎞ�I�ɐ���
    STROKE *nullSt = new STROKE(EOST);
    initSub(rootBlock, nullSt);
    delete nullSt;
}

void StTable::initSub(Block *currentBlock, STROKE *currentSt) {
    char *str, *check;
    MOJI moji;
    int len;
    STROKE *st;

    // �u�����N�̏ꍇ�͉��������Ԃ�
    if (currentBlock == 0) { return; }

    switch (currentBlock->kind()) {
    case STRING_BLOCK:      // ������u���b�N�̏ꍇ
        // 1 �����̒�`�ł���Γo�^����
        str = ((StringBlock *)currentBlock)->str;
        moji = str2moji(str, &check);
        if (*str == 0 || *check != 0) { return; }
        // �X�g���[�N��𕡐�
        len = strokelen(currentSt);
        st = new STROKE[len + 1];
        memcpy(st, currentSt, len + 1);
        (*stMap)[moji] = st;
        return;

    case CONTROL_BLOCK:     // �R���g���[���u���b�N�̏ꍇ
        // �l�X�g�ɓ���
        for (int key = 0; key < TC_NKEYS*2; key++) {
            Block *nextBlock = (((ControlBlock *)currentBlock)->block)[key];
            // �V�����X�g���[�N����ꎞ�I�ɐ���
            len = strokelen(currentSt);
            st = new STROKE[len + 2];
            strokecpy(st, currentSt);
            st[len] = key; st[len + 1] = EOST;
            // �ċA
            initSub(nextBlock, st);
            delete [] st;
        }
        return;

    case SPECIAL_BLOCK:     // ����u���b�N�̏ꍇ : �������Ȃ�
    default:
        return;
    }
}

// -------------------------------------------------------------------
// ����

int StTable::look(MOJI moji) {
    // �}�b�v���ɂ��邩�`�F�b�N
    if (stMap->find(moji) == stMap->end()) {
        return 0;
    }

    // �X�g���[�N����o��
    stroke = (*stMap)[moji];

    //<multishift>
    matchPref(stroke, &baseStroke, &mkTbl, &mkSt);
    //</multishift>

    return 1;
}

//<multishift>
// -------------------------------------------------------------------
// ���i�V�t�g

void StTable::setupPref(const char *src) {
    char *spec;
    char *ent;
    char smkTbl[256], sst[256], smkSt1[256], smkSt2[256]; // XXX
    int k, n, i;
    char *p;
    MOJI m;

    nprefs = 0;
    if (src == 0) { return; }

    spec = new char[strlen(src) + 1];
    strcpy(spec, src);

    for (ent = strtok(spec, ":");
         // (Shift-JIS �� 2 �o�C�g�ڂ��u/�v�u:�v�ɂȂ邱�Ƃ͂Ȃ��炵��)
         ent && nprefs < MAXPREFS;
         ent = strtok(NULL, ":")) {
        // �t�H�[�}�b�g�`�F�b�N
        // (Shift-JIS �� 2 �o�C�g�ڂ��u/�v�u:�v�ɂȂ邱�Ƃ͂Ȃ��炵��)
        if (sscanf(ent, "/%[^/]/%[^/]/%[^/]/%[^/]/",
                   smkTbl, sst, smkSt1, smkSt2) != 4) { continue; }
        // �`�F�b�N OK�B��`�J�n�B
        struct PREF *q = &(pref[nprefs++]);

        // mkTbl
        q->mkTbl[0] = '\0'; moji2strcat(q->mkTbl, STR2MOJI(smkTbl));

        // stlen, st
        // ��͂��₷���悤�Ɂu,�v��t��
        sst[strlen(sst) + 1] = '\0'; sst[strlen(sst)] = ',';
        for (q->stlen = 0, p = sst;
             q->stlen < MAXPREFLEN;
             p += n, q->stlen++) {
            if (1 <= sscanf(p, "%d,%n", &k, &n)) ;
            else if (1 <= sscanf(p, "S%d,%n", &k, &n)) k = TC_SHIFT(k);
            else break;
            q->st[q->stlen] = k;
        }

        // mkSt
        // �L���̏�����
        for (i = 0; i < 6; i++) { strcpy(q->mkSt[i], defmkSt[i]); }

        // �L�����̈� (�P�ƑŌ�)
        for (i = 0, p = smkSt1;
             *p && (m = str2moji(p, &p)) != 0 && i < 4;
             i++) {
            q->mkSt[i][0] = '\0';
            moji2strcat(q->mkSt[i], m);
        }

        // �L�����̓� (�A���Ō�)
        for (i = 0, p = smkSt2;
             *p && (m = str2moji(p, &p)) != 0 && i < 2;
             i++) {
            q->mkSt[i + 4][0] = '\0';
            moji2strcat(q->mkSt[i + 4], m);
        }

    }

    delete [] spec;
}
//</multishift>

//<multishift>
void StTable::matchPref(STROKE *st, STROKE **pst, 
                        char **pmkTbl, char (**pmkSt)[3]) {
    for (int i = 0; i < nprefs; i++) {
        struct PREF *q = &(pref[i]);

        for (int j = 0; j < q->stlen; j++) {
            if (q->st[j] != st[j]) { goto MATCHPREF_UNMATCH; }
        }
        // �}�b�`����
        if (pst) { *pst = st + q->stlen; }
        if (pmkSt) { *pmkSt = q->mkSt; }
        if (pmkTbl) { *pmkTbl = q->mkTbl; }
        return;

    MATCHPREF_UNMATCH:
        continue;
    }
    // �}�b�`�Ȃ�
    if (pst) { *pst = st; }
    if (pmkSt) { *pmkSt = defmkSt; }
    if (pmkTbl) { *pmkTbl = defmkTbl; }
}
//</multishift>

// -------------------------------------------------------------------
