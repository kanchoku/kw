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
 * TCode �N���X - T-Code �ϊ���
 */



/* -------------------------------------------------------------------
 * T-Code �ϊ���̓���
 * ===================
 *
 * T-Code �ϊ���́A�L�[���͂����邽�тɁA���̊e phase ���J��Ԃ�
 * (NORMAL ���[�h�A���Ȃ킿�ʏ�̃X�g���[�N���̓��[�h�̏ꍇ)�B
 *
 * (1) [�e�[�u���J��] - keyinNormal()
 *     �L�[���� (T-Code �L�[�ԍ��œ��͂����) �����ƂɌ��݂̃e�[�u���������A
 *     ��ԑJ�ڂ��s���B
 *
 * (2) [�����̏E���o��] - keyinNormal()
 *     �J�ڂ̌��ʂ�������ł���΁A���̕������ preBuffer �̖����ɒǉ�����B
 *     �����`�̂����A���񍇐��ϊ�����������ϊ��̊J�n������ �� �� ���A
 *     �܂��A�ϊ��̎��s������ SPC ���A���̂܂� preBuffer �ɓ����B
 *
 * (3) [�ϊ�] - isReducibleBy*(), reduceBy*()
 *     preBuffer �̓��e�����āA�K�v�Ȃ�Ε��񍇐��ϊ��܂��͌��������ϊ���
 *     ���s���� (��q)�B
 *     �ϊ����������� helpBuffer �ɒǉ�����B
 *
 * (4) [�o��] - TableWindow::output()
 *     preBuffer �̓��e���A�ϊ�������� (�� �܂��� �� ���Ȃ�) �ł���΁A
 *     �����ΏۃA�v���ɑ���B
 *     �܂��A��u�ϊ��ɔ����āApostBuffer �ɂ��ǉ����Ă����B
 *
 * (5) [���z����] - makeVKB(), TableWindow::draw*()
 *     ���݂̏�Ԃɉ����ĉ��z���Ղ��쐬���A�\������B
 *
 * CAND ���[�h (���������ϊ��̌��I�����[�h) �܂��� HIST ���[�h (�q�X�g������
 * ���[�h) �ł́A��L�� (1) �ɂ����āA�ʏ�̃e�[�u���̂����ɁA
 * ���������ϊ��̌��̔z��܂��̓q�X�g�����̔z���p���āA��ԑJ�ڂ��s���B
 *
 * �ϊ��̎��s
 * ----------
 *
 * ��L (3) �̕ϊ��̓���B
 * ���� (A), (B) �̑�����ApreBuffer �̓��e�����͂�ω����Ȃ��Ȃ�܂ŌJ��Ԃ��B
 *
 * (A) preBuffer �̖������u�� <����> <����>�v�Ƃ����`�����Ă���΁A
 *     ���̕������A���񍇐��ϊ����������Œu��������B
 * (B) preBuffer �̖������u�� <������> <SPC>�v�Ƃ����`�����Ă���΁A
 *     ���̕������A���������ϊ�����������Œu��������B
 *     ���������ϊ��͈�ʂɌ��̑I�����K�v�Ȃ̂ŁA�ꎞ�I�ɕϊ����
 *     CAND ���[�h (���I�����[�h) �ɐ؂�ւ��ď������s���B
 *
 * ��Ƃ��āApreBuffer �̓��e���u�� �� �� �� �E�v�ł���ꍇ���l����B
 * ���� [����] phase �ŁA�u�q�v�����͂����ƁApreBuffer �̓��e�́A
 *     �u�� �� �� �� �E �q�v  ��  �u�� �� �� ���v
 * �̂悤�ɕω�����B
 * ����ɁASPC �����͂����ƁApreBuffer �̓��e�́A
 *     �u�� �� �� �� <SPC>�v  ��  �u�� ���v
 * �̂悤�� (���I�����[�h���o��) �ω�����B�Ō�́u�� ���v�̏�Ԃ�
 * �ϊ�������Ԃł���A���� [�o��] phase �ŏo�͂���邱�ƂɂȂ�B
 *
 * �u�� �� �� �� �E�v�܂��́u�� �� �� ���v�̏�Ԃ́A(���̓��͂����Ȃ�����)
 * �ϊ��\�ȏ�Ԃł͂Ȃ����A�ϊ�������Ԃł��Ȃ��B
 * ���̂悤�ȏꍇ�́ApreBuffer �̓��e�͉��z���Ղ̃~�j�o�b�t�@�ɕ\������B
 *
 * -------------------------------------------------------------------
 */

/* ���������ϊ��Ɋւ���萔
 * ------------------------
 * candOrder
 * ---------
 * ���������ϊ��̌��I���ł́A�L�[�̗D�揇�� (0 �` 29)�B
 * �l -1 �͌��I���Ɏg�p���Ȃ��L�[��\�� (�����Ō�)�B
 * �D�揇�ʂ̏�� 10 �� (0 �` 9) �́A�L�[�{�[�h���i�� 10 �L�[�A���Ȃ킿
 * �L�[�ԍ� 20 �` 29 �̃L�[�Ɋ���U�� (����́A���z���Ղ̕\���̍ۂɁA
 * �q�X�g�����͂Ɠ��l�̈����ɂ��邽�߂ŁA�ύX���Ȃ�����)�B
 * �����Q���w�����邽�߂� SPC (�ʏ� 40 �Ԃ̃L�[�Ɋ��蓖�Ă���) �́A
 * -1 �łȂ���΂Ȃ�Ȃ��B
 * �Ȃ��A�O���Q�̎w���́A�ʏ� BackSpace �Ɋ��蓖�Ă���B
 * �܂��A�����A���������w�肵�Ȃ����������ϊ������������ꍇ�A
 * 44�E45 �Ԃ̃L�[���A�ǂ݂̐L�΂�/�k�߂ɗ\�񂳂��\��������B
 *
 * candSkip
 * --------
 * �����Q/�O���Q�ɐi�ގ��ɂ����̌����X�L�b�v���邩�B
 * candOrder �̂����A�l�� -1 �łȂ����̂̌� (���A�����菬������)�B
 */
const int TCode::candOrder[TC_NKEYS] = {         // (���{��L�[�{�[�h�̗�)
    -1, -1, -1, -1, -1,    -1, -1, -1, -1, -1,  // 1 2 3 4 5    6 7 8 9 0
    17, 15, 13, 11, 19,    18, 10, 12, 14, 16,  // q w e r t    y u i o p
     7,  5,  3,  1,  9,     8,  0,  2,  4,  6,  // a s d f g    h j k l ;
    27, 25, 23, 21, 29,    28, 20, 22, 24, 26,  // z x c v b    n m , . /
      -1, -1, -1, -1, -1, -1, -1, -1, -1        // SP - ^ \ @ [ : ] _
};
const unsigned int TCode::candSkip = 30;

/* �R���X�g���N�^�ƃf�X�g���N�^
 * ----------------------------
 * ���z�L�[�̗� v, �X�g���[�N�����ϊ��̃e�[�u�� t,
 * ���������ϊ����� m, ���񍇐����� b ����肱��ŁA�ϊ���𐶐��B
 * ���̂Ƃ��A�^����ꂽ�e�[�u������A�X�g���[�N�t�����e�[�u��������Ă����B
 * v, t, m, b �́A��������Ȃ��̂ŁA���̏ꏊ�� delete ���Ȃ����� (�f�X�g���N�^
 * ���� delete �����)�B
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
	isAnyShiftSeq = false;
    //</v127a - shiftcheck>
    //<v127a - gg>
    ggDic = g;
    //</v127a - gg>

    preBuffer  = new MojiBuffer(TC_BUFFER_SIZE);
    postBuffer = new MojiBuffer(TC_BUFFER_SIZE);
    helpBuffer = new MojiBuffer(TC_BUFFER_SIZE);
    currentStroke = new std::vector<STROKE>;
    inputtedStroke = new std::vector<STROKE>;
    stTable = new StTable(table);
    assignedsBuffer = new MojiBuffer(TC_BUFFER_SIZE);
    explicitGG = 0;
    ggCand = 0;
    ggCStart = 0;
    ittaku = 0;
    lockedBlock = table;
    lockedStroke = new std::vector<STROKE>;
    postKataPrevLen = 0;

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

    //<OKA> ���������Ă����Ȃ��� addToHistMaybe() �ŃN���b�V������
    for (int i = 0; i < TC_NHIST; ++i) {
        hist[i] = NULL;
        histRef[i] = 0;
    }
    histPtr = 0;
    //</OKA>

    //<multishift2>
    // directive �e�[�u���̏�����
    for (int i = 0; i < DIR_MAX_ENT; i++) { dirTable[i] = 0; }
    //</multishift2>

    //<record>
    // �L�^�p
    record.OPT_record = 0;
    record.nchar = 0;
    record.nstroke = 0;
    record.nbushu = 0;
    record.nmaze = 0;
    record.nspecial = 0;

    // ���v�p
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
 * �ϊ���̃��Z�b�g
 * ----------------
 */

void TCode::reset() {
    currentBlock = lockedBlock;
    *currentStroke = *lockedStroke;
    *inputtedStroke = *lockedStroke;
    currentShift = 0;
    helpModeSave = 0;
    clearGG(table);
    clearAssignStroke();
    waitKeytop = (!!OPT_displayHelpDelay || OPT_offHide == 2);
    // maze2gg �ł��ԈႢ�~��
    // ���̓~�X���o�͌�makeVKB��explicitGG�������āA�ȍ~�̓��͂�reset���Ăяo���ꂽ�犮�S�I��
    if (!explicitGG) clearCandGG();
}

void TCode::resetBuffer() {
    preBuffer->clear();
    postBuffer->clear();
    postInPre = 0;
    postDelete = 0;
    postKataPrevLen = 0;
    clearCandGG();
}

void TCode::lockStroke() {
    lockedBlock = currentBlock;
    *lockedStroke = *currentStroke;
}

void TCode::unlockStroke() {
    lockedBlock = table;
    lockedStroke->clear();
}

/* -------------------------------------------------------------------
 * �L�[����
 * --------
 *
 * - 3 �̎僂�[�h (NORMAL, CAND, HIST) �ɑ΂���A
 * - 5 ��̓��̓L�[ (RET, BS, ESC, TAB, T-Code �L�[)
 * �́A���ׂĂ̑g�ݍ��킹�ɑΏ����邱�ƁB
 *
 * ����ɁANORMAL ���[�h�ɂ����ẮA���� 2 ��ނ̏ꍇ�������K�v:
 *
 * (A0) �ʏ�̒��ړ���              (preBuffer �������)
 * (A1) �⏕�ϊ��̓ǂ݁E����̓���  (preBuffer �Ɋ��ɕ����������Ă���)
 *
 * (B0) ���Ō��҂�        (currentBlock == table)
 * (B1) ���Ō��ȍ~�҂�    (currentBlock != table)
 */

#define ISEMPTY (preBuffer->length() == 0)
#define ISRESET (currentBlock == lockedBlock/*table*/)
#define ISLOCKED (lockedBlock != table)
#define MS(s) B2MOJI(*(s), *((s) + 1))
#define MC(c) B2MOJI(0, (c))
#define MV(v) B2MOJI(MOJI_VKEY, v)
#define M_V(v) B2MOJI(currentCtrl?MOJI_CTRLVKY:MOJI_VKEY, v)

/* NORMAL ���[�h�ł̃L�[����
 * -------------------------
 * - RET    : �⏕�ϊ��̓ǂ݁E����̓��͒��Ȃ�A���ϊ��Ŋm��B
 *            �����łȂ���΁ARET ���̂��́B
 * - BS     : ���Ō��ȍ~�Ȃ�A���܂ł̃X�g���[�N���L�����Z���B
 *            ���Ō��Ȃ�ABS ���̂��́B
 *            �⏕�ϊ��̓ǂ݁E����̓��͒��Ȃ�A�ǂ݁E����̍폜�B
 * - ESC    : ���Ō��ȍ~�Ȃ�A���܂ł̃X�g���[�N���L�����Z���B
 *            ���Ō��Ȃ�AESC ���̂��́B
 *            �⏕�ϊ��̓ǂ݁E����̓��͒��Ȃ�A�ǂ݁E�����j���B
 * - TAB    : �⏕�ϊ��̓ǂ݁E����̓��͒��Ȃ�A�Ђ炪��/�������ȕϊ��E�m��B
 *            ���Ō��ȍ~�Ȃ�A���Ō��̃L�[�̓��́B
 *            ���Ō��Ȃ�ATAB ���̂��́B
 *
 * - T-Code �L�[ : �e�[�u���������āA��`�ɏ]���B
 */
void TCode::keyinNormal(int key) {
    // �����L�[���͂���������A�܂��w���v���\���ɂ��邱��
    helpMode = 0;

    int currentCtrl = 0;
    switch (key) {
    case CM_KEY:
    case CJ_KEY:
    case CH_KEY:
    case CG_KEY:
    case CI_KEY:
        currentCtrl = 1;
    }
    if (OPT_useCtrlKey == 2 && currentCtrl) {
        currentCtrl = 0;
        switch (key) {
        case CM_KEY:
        case CJ_KEY:
            key = RET_KEY; break;
        case CH_KEY:
            key = BS_KEY; break;
        case CG_KEY:
            key = ESC_KEY; break;
        case CI_KEY:
            key = TAB_KEY; break;
        }
    }

    // �V�t�g�Ō�
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
        if (ISEMPTY) { preBuffer->pushSoft(M_V(vk)); }
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
        if (ggCand && !ggCInputted() && ISRESET) {
            clearCandGG();
            return;
        }
        if (ISEMPTY) {
            if (ISRESET || OPT_hardBS) {
                preBuffer->pushSoft(M_V(vk));
            }
        } else {
            if (ISRESET || OPT_hardBS) {
                preBuffer->pop();
                if (ISEMPTY) postDelete = 0;
            }
        }
        if (OPT_weakBS && !ISRESET) {
            currentStroke->pop_back();
            int stlen = currentStroke->size();
            ControlBlock *p = table;
            for (int i = 0; i < stlen; i++) {
                Block *np = (p->block)[(*currentStroke)[i]];
                p = (ControlBlock *)np;
            }
            currentBlock = p;
            *inputtedStroke = *currentStroke;
        } else {
            if (ggCand && !explicitGG) explicitGG = new char[1];  // �ł��ԈႢ�~��
            int w = waitKeytop;
            if (ISRESET) {
                reset();
            } else {
                reset();
                waitKeytop = w;
            }
        }
        return;

    case ESC_KEY:
    case CG_KEY:
        switch (key) {
        case CG_KEY:  vk = 'G'; break;
        case ESC_KEY:
        default: vk = VK_ESCAPE; break;
        }
        if (ISEMPTY) {
            if (ISRESET) { preBuffer->pushSoft(M_V(vk)); }
        } else {
            if (ISRESET) { preBuffer->clear(); postDelete = 0; } // XXX ����̓L�c�C?
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
                if (maze2ggMode && explicitGG && ggCInputted() < ittaku) {
                    MojiBuffer work(strlen(explicitGG));
                    work.pushSoft(explicitGG);
                    preBuffer->pushSoft(work.string(ggCInputted(), ittaku-ggCInputted()));
                } else {
                    preBuffer->pushSoft(M_V(vk));
                }
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

    /* �ȉ� T-Code �L�[
     * ----------------
     * T-Code �L�[�̏ꍇ�́A�e�[�u���������āA���̒�`�ɏ]��:
     * - ���`     : ��Ԃ����Z�b�g
     * - ������     : ��`���ꂽ�������A�K�v�ɉ����āA�Ђ炪��/�������ȕϊ��A
     *                �܂��́A���p/�S�p�ϊ����ApreBuffer �Ɋi�[����
     * - ����q�̕\ : ��ԑJ�ڂ���
     * - �����`   : ��`�ɏ]��������
     */
    if (OPT_shiftLockStroke == 1) {
        if (ISLOCKED) {
            if (!justCurShift) {
                unlockStroke();
                reset();
            }
        } else {
            if (justCurShift) lockStroke();
        }
    }
    currentStroke->push_back(key);
    Block *nextBlock = (currentBlock->block)[key];
    if (OPT_shiftFallback && !nextBlock) {
        currentStroke->pop_back();
        currentStroke->push_back(TC_UNSHIFT(key));
        nextBlock = (currentBlock->block)[TC_UNSHIFT(key)];
    }
    std::vector<STROKE> bkupStroke = *currentStroke;
    *inputtedStroke = bkupStroke;
    //<record>
    record.nstroke += 1;
    //</record>

    if (nextBlock == NULL) { reset(); return; }

    int n = 0;                  // ��u�^�̌��������ϊ��̓ǂ݂̒���
    switch (nextBlock->kind()) {
    case STRING_BLOCK:
        {
            char *str = ((StringBlock *)nextBlock)->str;
            MojiBuffer mb(TC_BUFFER_SIZE);
            mb.clear();
            for (char *s = str; *s; ) {
                MOJI m;
                char *tr=s;
                if (OPT_outputUnicode && strlen(s) >= 6 && s[0] == 'U' && s[1] == '+') m = (MOJI)strtoul(s+2, &tr, 16);
                if (OPT_outputUnicode && strlen(s) >= 6 && s[0] == 'U' && s[1] == '+'
                    && m < 0x3FFFU && tr == s+6) { s = tr; if (m >= 0x0080U) m += 0x4000; }
                else m = str2moji(s, &s);
                if (currentShift) { m = mojiHirakata(m); }
                if (punctMode)    { m = mojiPunct(m); }
                //<hankana>
                //if (hanzenMode)   { m = mojiHanzen(m); }
                //if (hirakataMode) { m = mojiHirakata(m); }
                // �u�S�p���� �� ���p���ȁv�̕ϊ� (mijiHandaku(...))�B
                // ���p���ȕϊ��́A�����┼�������ŕ��������ω�����̂ŁA
                // ���̕ϊ��n�̑��� (mojiHOGE(m) �̗�) ����ɏ�������B
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
                if (OPT_outputAlphabetAsVKey && m >= 'a' && m <= 'z' ) {
                    m = MV(m-'a'+'A');
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
            for (int i=0; i < mb.length(); i++) preBuffer->pushSoft(mb.moji(i));
        }
        reset(); *inputtedStroke = bkupStroke; return;

    case CONTROL_BLOCK:
        // ������ reset() ���Ȃ�����
        currentBlock = (ControlBlock *)nextBlock;
        return;

    case SPECIAL_BLOCK:
        /* �ȉ��A�����`
         * - @s     : �݊����̂��߁A@K �Ɠ�������
         * - @K @Z  : �Ђ炪��/�������ȃ��[�h�A���p/�S�p���[�h���g�O��
         * - @m @b  : �ϊ��J�n�}�[�N�� preBuffer �ɓ����
         * - @!     : �q�X�g�����̓��[�h�ɑJ��
         * - @q     : preBuffer �̓��e���N���A���A�ʏ���̓��[�h�ɖ߂�
         * - @h @H  : helpMode ���Z�b�g���A�|�C���^���Z�b�g����
         * - @v     : ���Ō��� preBuffer �ɓ����
         * - @B @1..@9 @Q..@Y @- @$..@)
         *             : �ϊ��̓ǂ݁E���i�� postBuffer �������Ă��āA
         *               �ϊ��J�n�}�[�N�ƂƂ��� preBuffer �ɓ����B
         *               �܂��A�x���f���[�g�ʂ�ݒ肷��B
         * - @D @P  : ���O�̓��͕����� postBuffer �������Ă��āA
         *            �����܂��͔������ϊ����� preBuffer �ɓ����B
         *            �܂��A�x���f���[�g�ʂ�ݒ肷��B
         */
        n = 0;                  // ��u�^�̌��������ϊ��̓ǂ݂̒���
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
            maze2ggMode ^= 1;
            clearCandGG();
            //<record>
            record.nspecial += 1;
            //</record>
            reset(); return;

        case F_SHOWWIN:
            if (OPT_offHide != 0) {
            OPT_offHide = 3 - OPT_offHide;  // ���̃p�b�`���ȗ��������Ă��������܂���
            //<record>
            record.nspecial += 1;
            //</record>
            }
            reset(); return;

        case F_BUSHU_PRE:
            //<v127c - postInPre>
            //postDelete = 0;
            //</v127c>
            if (bushuReady != 0) { preBuffer->pushSoft(MOJI_BUSHU); }
            else                 { preBuffer->pushSoft("��"); }
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
            else                { preBuffer->pushSoft("��"); }
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

        case F_VERB_THIS:
            preBuffer->pushSoft(MV(vkey[TC_UNSHIFT(currentStroke->back())]));
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
                //// �O�u�ϊ��Ƃ̑g�ݍ��킹�͖�����
                // �O�u�ϊ��Ƃ̑g�ݍ��킹
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
        case F_MAZE_POST1: n += 1; // ���̎��_�� n == 1 �` 9
            reset();
            if (mazeReady == 0) { return; }
            //<record>
            // XXX: ?
            record.nspecial += 1;
            //</record>
            if (!ISEMPTY) {
                //<v127c - postInPre>
                //// �O�u�ϊ��Ƃ̑g�ݍ��킹�͖�����
                // �O�u�ϊ��Ƃ̑g�ݍ��킹
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
                preBuffer->pushSoft((MOJI)' '); // �ϊ��I���}�[�N
                //</v127c>
                return;
            }
            if (postBuffer->length() < n) { return; }
            postDelete = n;
            strcpy(yomi, postBuffer->string(-postDelete));
            preBuffer->pushSoft(MOJI_MAZE);
            preBuffer->pushSoft(yomi);
            preBuffer->pushSoft((MOJI)' '); // �ϊ��I���}�[�N
            return;

        case F_KATA_POST9: n += 1; /* go down */
        case F_KATA_POST8: n += 1; /* go down */
        case F_KATA_POST7: n += 1; /* go down */
        case F_KATA_POST6: n += 1; /* go down */
        case F_KATA_POST5: n += 1; /* go down */
        case F_KATA_POST4: n += 1; /* go down */
        case F_KATA_POST3: n += 1; /* go down */
        case F_KATA_POST2: n += 1; /* go down */
        case F_KATA_POST1: n += 1; // ���̎��_�� n == 1 �` 9
        case F_KATA_POST0:
            keyinNormalKataPost(n);
            return;
        case F_KATA_POSTH6: n -= 1;
        case F_KATA_POSTH5: n -= 1;
        case F_KATA_POSTH4: n -= 1;
        case F_KATA_POSTH3: n -= 1;
        case F_KATA_POSTH2: n -= 1;
        case F_KATA_POSTH1: n -= 1; // F_KATA_POSTH[1-6]�̏ꍇ�An == -1 .. -6
            keyinNormalKataPost(n);
            return;
        case F_KATA_POSTS5: n += 1;
        case F_KATA_POSTS4: n += 1;
        case F_KATA_POSTS3: n += 1;
        case F_KATA_POSTS2: n += 1;
        case F_KATA_POSTS1: n += 1; // F_KATA_POSTS[1-5]�̏ꍇ�An == 1 .. 5
            keyinNormalKataPostShrink(n);
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
                DAKUTEN_HANDAKUTEN(mojiDaku, "�J", "�");
            } else {
                DAKUTEN_HANDAKUTEN(mojiDaku, "�J", "�J");
            }

        case F_HANDAKUTEN:
            //<record>
            // XXX: ?
            ////record.nspecial += 1;
            //</record>
            reset();
            if (OPT_enableHankakuKana) {
                DAKUTEN_HANDAKUTEN(mojiHandaku, "�K", "�");
            } else {
                DAKUTEN_HANDAKUTEN(mojiHandaku, "�K", "�K");
            }

#undef DAKUTEN_HANDAKUTEN

        default:                // �������� SPECIAL
            reset();
            //<record>
            // XXX: ?
            ////record.nspecial += 1;
            //</record>
            return;
        } // switch block function

    default:                    // �����ɂ����Ȃ��͂�
        ;
    } // switch block kind
                                // �����ɂ����Ȃ��͂�
}

// ��u�^�̂������ȕϊ�
void TCode::keyinNormalKataPost(int n) {
    reset();
    //<record>
    // XXX: ?
    record.nspecial += 1;
    //</record>
    if (!ISEMPTY) {
        // �O�u�ϊ��Ƃ̑g�ݍ��킹
        int isYomiToKata = 0;
        if (n <= 0) {
            int len = preBuffer->length();
            int offset;
            for (offset = len - 1; 0 <= offset; offset--) {
                MOJI m = preBuffer->moji(offset);
                if (m == MOJI_MAZE) {
                    // ���������ϊ��̓ǂ݂��������Ȃɕϊ�
                    isYomiToKata = 1;
                    break;
                }
                if (mojitype(m) == MOJI_SPECIAL) break;
            }
            if (!isYomiToKata && n < 0)
                offset += -n; // �w�蕶�����������Ă������Ȃɕϊ�
            n = len - offset - 1;
            if (n <= 0) return;
        } else {
            if (preBuffer->length() < n + 1) return;
            for (int i = -1; -n <= i; i--) {
                MOJI m = preBuffer->moji(i);
                if (mojitype(m) == MOJI_SPECIAL) return;
            }
        }
        postInPre = n;
        strcpy(yomi, preBuffer->string(-n));
        preBuffer->popN(n);
        if (isYomiToKata) preBuffer->pop(); // MOJI_MAZE
        preBuffer->pushSoft(MOJI_MAZE);
        preBuffer->pushSoft(yomi);
        preBuffer->pushSoft((MOJI)'\t'); // �ϊ��I���}�[�N
        return;
    }
    if (n <= 0) {
        // XXX: ���`�̕����̏ꍇpostBuffer�ɓ���Ȃ��̂Ŗ��������B
        // ��: �u��+���v���u���A�C�v(�u+�v�����������)
        // F_VERB_THIS�Ƃ��Ē�`����Ή���͉\�B
        int len = postBuffer->length();
        int offset;
        for (offset = len - 1; 0 <= offset; offset--) {
// TRUE�̕����������ԁA��u�^�������ȕϊ��ΏۂƂ���(�Ђ炪�ȁA�u�[�v)
#define IN_KATARANGE(m) ((MOJI2H(m) == 0x82 && \
                          0x9f <= MOJI2L(m) && MOJI2L(m) <= 0xf1) || \
                         (MOJI2H(m) == 0x81 && MOJI2L(m) == 0x5b))
            MOJI m = postBuffer->moji(offset);
            if (!IN_KATARANGE(m)) break;
        }
        if (n < 0)
            offset += -n; // �w�蕶�����������Ă������Ȃɕϊ�
        n = len - offset - 1;
        if (n <= 0) return;
    }
    if (postBuffer->length() < n) { return; }
    postDelete = n;
    strcpy(yomi, postBuffer->string(-postDelete));
    preBuffer->pushSoft(MOJI_MAZE);
    preBuffer->pushSoft(yomi);
    preBuffer->pushSoft((MOJI)'\t'); // �ϊ��I���}�[�N
}

// ���O�̌�u�^�̂������ȕϊ����k�߂�
// ��: �u�Ⴆ�΂��Ղ肯�[�����v�Ђ炪�Ȃ������Ԃ������Ȃɕϊ�
//   ���u��G�o�A�v���P�[�V�����v2�����k�߂�
//   ���u�Ⴆ�΃A�v���P�[�V�����v
void TCode::keyinNormalKataPostShrink(int n) {
    reset();
    record.nspecial += 1;
    if (postKataPrevLen == 0) return;
    if (!ISEMPTY) return; // �O�u�^�ϊ����̏ꍇ�͂��Ԃ�s�v
    int len = postBuffer->length();
    if (len < postKataPrevLen) { return; }
    int kataLen = postKataPrevLen - n;
    if (kataLen > 0)
        strcpy(yomi, postBuffer->string(-kataLen));
    else
        kataLen = 0;
    postDelete = postKataPrevLen;
    // �k�߂邱�ƂłЂ炪�ȂɂȂ镶����
    for (int i = len - postKataPrevLen; i < len - kataLen && i < len; i++) {
        MOJI m = postBuffer->moji(i);
        preBuffer->pushSoft(mojiHirakata(m));
    }
    if (kataLen > 0) {
        preBuffer->pushSoft(yomi); // �������Ȃ̂܂܂ɂ��镶����
    }
    postKataPrevLen = kataLen; // �J��Ԃ�Shrink�ł���悤��
}

/* ���������ϊ��̌��I�����[�h�ł̃L�[����
 * ----------------------------------------
 * - RET    : ���ϊ��Ŋm��
 * - BS     : �O���Q�B�������A�ŏ��̌��Q�܂Ŗ߂�����A
 *            NORMAL ���[�h (�ǂݓ��̓��[�h) �ɖ߂�
 * - ESC    : NORMAL ���[�h�ɖ߂�
 * - TAB    : �Ђ炪��/�������ȕϊ����āA�m��
 * - <      : �ǂ݂�L�΂�
 * - >      : �ǂ݂��k�߂�
 *
 * - SPC    : ����⊷
 * - ���̑��̃L�[ : ���Ԃ���I���L�[�Ȃ̂ŁA�Ή�������� preBuffer �ɑ}��
 */
void TCode::keyinCand(int key) {
    // �����L�[���͂���������A�܂��w���v���\���ɂ��邱��
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

    /* �ȉ� T-Code �L�[
     *
     * �����ɂ��ǂ���Ƃ������Ƃ́A��₪ 2 �ȏ゠��Ƃ������B
     * �܂��AreduceByMaze() ���o�R���Ă���͂��ŁAyomi �� yomiLen ��
     * �������l���ݒ肳��Ă���Ɖ���B
     */

    // �V�t�g�Ō�
    //currentShift |= TC_ISSHIFTED(key);
    /* currentShift &= TC_ISSHIFTED(key); 8 */
    key = TC_UNSHIFT(key);

    // SPC �Ȃ玟���Q
    if (vkey[key] == VK_SPACE) {
        candOffset += candSkip;
        if (currentCand->size() <= candOffset) { candOffset = 0; }
        return;
    }

    // ��I���L�[�͖���
    if (candOrder[key] < 0) { return; }

    // �����ȑI���L�[������
    unsigned int oc = candOffset + candOrder[key];
    if (currentCand->size() <= oc) { return; }

    cand = (*currentCand)[oc];

    finishCand(cand);

    mode = NORMAL;
    return;
}

/* ���������ϊ��̌�₪�B��̏ꍇ�ł̃L�[����
 * ----------------------------------------
 * - RET    : �m��
 * - BS     : NORMAL ���[�h�ɖ߂�
 * - ESC    : NORMAL ���[�h�ɖ߂�
 * - TAB    : �Ђ炪��/�������ȕϊ����āA�m��
 * - <      : �ǂ݂�L�΂�
 * - >      : �ǂ݂��k�߂�
 *
 * - ���̑��̃L�[ : �m��� NORMAL ���[�h�Ɉ����p��
 */
void TCode::keyinCand1(int key) {
    // �����L�[���͂���������A�܂��w���v���\���ɂ��邱��
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
        clearCandGG();
        nferHirakata();
        return;

    case LT_KEY:
        if (OPT_conjugationalMaze != 2) return;
        makeMazeYomiLonger();
        if (maze2ggMode) { ggCand = currentCand; setCandGGHeader(); }
        return;

    case GT_KEY:
        if (OPT_conjugationalMaze != 2) return;
        makeMazeYomiShorter();
        if (maze2ggMode) { ggCand = currentCand; setCandGGHeader(); }
        return;

    default:
        break;
    }

    /* �ȉ� T-Code �L�[
     */
    cand = (*currentCand)[0];
    finishCand(cand);
    mode = NORMAL;
    keyinNormal(key);
}

/* �q�X�g�����̓��[�h�ł̃L�[����
 * ------------------------------
 * - �����ȑI���L�[ : �ʏ���̓��[�h�ɖ߂�
 * - �L���ȑI���L�[ : �Ή�������̎Q�ƃr�b�g���Z�b�g���A
 *                    ���� preBuffer �ɓ����
 */
void TCode::keyinHist(int key) {
    // �����L�[���͂���������A�܂��w���v���\���ɂ��邱��
    helpMode = 0;

    // �V�t�g�Ō�
    //currentShift |= TC_ISSHIFTED(key);
    /* currentShift &= TC_ISSHIFTED(key); 8 */
    key = TC_UNSHIFT(key);

    // �����ȑI���L�[�͖���
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

void TCode::postBufferDeleted(int del) {
    if (ggCand && ggCStart < 0) ggCStart += del;
}

void TCode::postBufferCount(int inc) {
    if (ggCand) ggCStart -= inc;
}

/* -------------------------------------------------------------------
 * �⏕����
 * --------
 */

/* isReducibleByBushu()
 * --------------------
 * preBuffer �����񍇐��ϊ��\���ǂ����B
 * ���Ȃ킿�ApreBuffer �̖������A�u�� <����> <����>�v�̌`�����Ă��邩�B
 */
int TCode::isReducibleByBushu() {
    if (mode != NORMAL) { return 0; }
    if (preBuffer->length() < 3) { return 0; }
    for (int i = -1; -2 <= i; i--) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) == MOJI_SPECIAL) { return 0; }  // XXX �`�F�b�N��
    }
    if (preBuffer->moji(-3) != MOJI_BUSHU) { return 0; }
    return 1;
}

/* reduceByBushu()
 * ---------------
 * isReducibleByBushu() == 1 �ł��鎞�ɁA
 * preBuffer �̓��e�̖����ɑ΂��āA���񍇐����͂����s����B
 */
void TCode::reduceByBushu() {
    MOJI m1 = preBuffer->moji(-2);
    MOJI m2 = preBuffer->moji(-1);
    MOJI m = bushuDic->look(m1, m2, OPT_bushuAlgo);
    if (m == 0) {               // �ϊ����s XXX 0?
        //<v127c - postInPre>
        //if (postDelete != 0) {  // ��u
        if (postInPre) {        // �O�u���̌�u
            cancelPostInPre(2);
            return;
        } else if (postDelete > 0 && preBuffer->length() == 3) {
                                // ��u
        //</v127c>
            preBuffer->popN(3);
            postDelete = 0;
            //<v127c - postInPre>
            // ���̈�s�͗]�v�������悤��
            //preBuffer->pop();
            //</v127c>
            return;
        } else {                // �O�u
            preBuffer->pop();   // ���s�̌�����菜��
            return;
        }
    }
    // �ϊ�����
    //<v127c - postInPre>
    preBuffer->popN(3);
    if (postInPre) {            // �O�u���̌�u
        postInPre = 0;
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
 * preBuffer �̓��e�����������ϊ��\���ǂ����B
 * ���Ȃ킿�ApreBuffer �̖������u�� <�ǂ�> <��>�v�̌`�����Ă��邩�B
 */
int TCode::isReducibleByMaze() {
    if (mode != NORMAL) { return 0; }
    if (preBuffer->length() < 3) { return 0; }
    MOJI m = preBuffer->moji(-1);
    // ���̍s�͑S�p�̋󔒂��܂�ł���
    if (m != MC(' ') && m != MS("�@") && m != MV(VK_SPACE)) { return 0; }

    int len = preBuffer->length();
    int offset;
    for (offset = len - 2; 0 <= offset; offset--) {
        m = preBuffer->moji(offset);
        if (m == MOJI_MAZE) {
            yomiLen = len - offset - 2;
            return 1;
        }
        if (mojitype(m) == MOJI_SPECIAL) { return 0; }  // XXX �`�F�b�N��
    }
    return 0;
}

/* reduceByMaze()
 * --------------
 * isReducibleByMaze() == 1 �ł��鎞�ɁA
 * preBuffer �̖����ɑ΂��āA���������ϊ������s����B
 */
void TCode::reduceByMaze() {
    preBuffer->pop();           // �ϊ��I���}�[�N
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
        if (postInPre) {        // �O�u���̌�u
            cancelPostInPre(yomiLen);
        } else if (postDelete > 0 && preBuffer->length() == yomiLen + 1) {
                                // ��u
            preBuffer->popN(yomiLen); postDelete = 0;
        //</v127c>
            preBuffer->pop();   // ��
        }
        return;
    }
    currentCand = mgTable->cand; candOffset = 0;

    if (maze2ggMode) {
        ggCand = currentCand;
        if (OPT_conjugationalMaze == 2) {
            mode = CAND1;
            setCandGGHeader();
        } else {
            setCandGGHeader();
            cand = (*currentCand)[0];
            finishCand(cand);
            mode = NORMAL;
        }
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

/* isReducibleByKata()
 * -------------------
 * preBuffer �̓��e���������ȕϊ��\���ǂ����B
 * ���Ȃ킿�ApreBuffer �̖������u�� <�ǂ�> \t�v�̌`�����Ă��邩�B
 */
int TCode::isReducibleByKata() {
    if (mode != NORMAL) { return 0; }
    if (preBuffer->length() < 3) { return 0; }
    MOJI m = preBuffer->moji(-1);
    if (m != MC('\t') && m != MV(VK_TAB)) { return 0; }

    int len = preBuffer->length();
    int offset;
    for (offset = len - 2; 0 <= offset; offset--) {
        m = preBuffer->moji(offset);
        if (m == MOJI_MAZE) {
            yomiLen = len - offset - 2;
            return 1;
        }
        if (mojitype(m) == MOJI_SPECIAL) { return 0; }  // XXX �`�F�b�N��
    }
    return 0;
}

/* reduceByKata()
 * --------------
 * isReducibleByKata() == 1 �ł��鎞�ɁA
 * preBuffer �̖����ɑ΂��āA�������ȕϊ������s����B
 */
void TCode::reduceByKata() {
    preBuffer->pop();           // �ϊ��I���}�[�N

    MojiBuffer tmpMB(TC_BUFFER_SIZE);
    tmpMB.clear();
    for (int i = preBuffer->length() - yomiLen; i < preBuffer->length(); i++) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) != MOJI_SPECIAL) {
            // mojiHirakata()���ƁA��u�^�̂������ȕϊ��ōŏ��Ɏw�肵����������
            // �����������肸1�������₵�Ă���1��ϊ�����ꍇ�ɔ��]����č���
            tmpMB.pushSoft(mojiKata(m));
        }
    }

    preBuffer->popN(yomiLen + 1);
    if (postInPre) {
        postInPre = 0;
    }
    for (int i = 0; i < tmpMB.length(); i++) {
        preBuffer->pushSoft(tmpMB.moji(i));
    }
    postKataPrevLen = yomiLen;
}

void TCode::finishCand(char *cand) {
    if (maze2ggMode) {
        goCandGG();
        return;
    }
    MojiBuffer *okuri;
    if (OPT_conjugationalMaze == 2 && okuriLen > 0) {
        okuri = new MojiBuffer(okuriLen);
        okuri->pushSoft(preBuffer->string(-okuriLen));
    }
    preBuffer->popN(yomiLen + 1);
    //<v127c - postInPre>
    if (postInPre) {
        postInPre = 0;
    }
    //</v127c>
    preBuffer->pushSoft(cand);
    if (OPT_conjugationalMaze == 2 && okuriLen > 0) {
        preBuffer->pushSoft(okuri->string(-okuriLen));
        delete okuri;
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
    //����ł͑S�������Ȃ̂ő��艼�����Z���Ȃ������
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
    if (maze2ggMode) { return; }
    if (currentCand->size() == 1) mode = CAND1;
    else mode = CAND;
}

void TCode::makeMazeYomiShorter() {
    //����ł͑S�������Ȃ̂ő��艼���������Ȃ������
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
    if (maze2ggMode) { return; }
    if (currentCand->size() == 1) mode = CAND1;
    else mode = CAND;
}

/* nfer()
 * ------
 * preBuffer ����A���ׂĂ̕ϊ��J�n�}�[�N����菜���B
 * ���̌��ʁAisReducibleBy*() == 0 && isComplete() == 1 �ƂȂ�̂ŁA
 * TableWindow::output() �ɂ���āApreBuffer �̓��e���܂Ƃ߂ďo�͂����B
 */
void TCode::nfer() {
    MojiBuffer tmpMB(TC_BUFFER_SIZE);
    tmpMB.clear();
    int n = preBuffer->length();
    for (int i = 0; i < preBuffer->length(); i++) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) != MOJI_SPECIAL) {
            tmpMB.pushSoft(m);
        }
    }
    preBuffer->clear();
    for (int i = 0; i < tmpMB.length(); i++) {
        preBuffer->pushSoft(tmpMB.moji(i));
    }
    if (ggCand) ggCStart = preBuffer->length() - (n - ggCStart);
    //<record>
    // NOP
    //</record>
}

/* nferHirakata()
 * --------------
 * �Ђ炪��/�������ȕϊ����s���ق��́Anfer() �Ɠ���
 */
void TCode::nferHirakata() {
    MojiBuffer tmpMB(TC_BUFFER_SIZE);
    tmpMB.clear();
    int n = preBuffer->length();
    for (int i = 0; i < preBuffer->length(); i++) {
        MOJI m = preBuffer->moji(i);
        if (mojitype(m) != MOJI_SPECIAL) {
            tmpMB.pushSoft(mojiHirakata(m));
        }
    }
    preBuffer->clear();
    for (int i = 0; i < tmpMB.length(); i++) {
        preBuffer->pushSoft(tmpMB.moji(i));
    }
    if (ggCand) ggCStart = preBuffer->length() - (n - ggCStart);
    //<record>
    // NOP
    //</record>
}

/* isComplete()
 * ------------
 * preBuffer �̓��e���A��������ȏ�ϊ������s�ł��Ȃ��`���ǂ����B
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
        if (!stTable->look(ggBuffer.moji(i))) continue;  // �O�������͍ς݂Ƃ���
        for (j = 0; j < yomiLen; j++) {
            if (ggBuffer.moji(i) == preBuffer->moji(-yomiLen+j)) break;
        }
        if (j < yomiLen) continue;
        break;
    }
    ggCHeaderLen = i;
}

void TCode::goCandGG() {
    MojiBuffer candBuffer(strlen((*currentCand)[0])); // ������ggCHeaderLen�܂ł͑S��⓯��
    candBuffer.pushSoft((*currentCand)[0]);
    preBuffer->popN(yomiLen + 1);
    if (postInPre) {        // �O�u���̌�u
        postInPre = 0;
    }
    ggCStart = preBuffer->length();
    preBuffer->pushSoft(candBuffer.string(0, ggCHeaderLen));
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
                if (j < ggCInputted()) {
                    if (mode==CAND1) {  // goCandGG�ȑO
                    } else if (ggCStart + j >= 0) {
                        if (preBuffer->moji(ggCStart + j) != ccand.moji(j)) break;
                    } else {
                        if (postBuffer->moji(ggCStart + j) != ccand.moji(j)) break;
                    }
                } else if (j == ggCInputted()) {
                    if (ggBuffer2.isEmpty()) ggBuffer.pushSoft(ccand.string());
                    else ggBuffer2.pushSoft(ccand.moji(j));
                }
            } else if (j < ggCInputted()) {
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
    ggCStart = 0;
    ittaku = 0;
    if (explicitGG) delete [] explicitGG;
    explicitGG = 0;
}

int TCode::ggCInputted() {
    if (mode==CAND1) {  // goCandGG�ȑO
        return ggCHeaderLen;
    } else {
        return preBuffer->length() - ggCStart;
    }
}

#undef MS
#undef MC
#undef MV

/* -------------------------------------------------------------------
 * �����w���v
 * ----------
 * ���񍇐��ϊ�����������ϊ��œ��͂��������̂����A�����w���v�̑ΏۂƂ���
 * ���̂������AhelpBuffer �Ɋi�[����B
 * �����w���v�̑Ώۂɂ�������́A
 * - �X�g���[�N�����݂��邱�� (�O���łȂ�����)�A����
 * - ���������ϊ��̓ǂ݂Ɋ܂܂�Ă��Ȃ�����
 * �ł���B�܂��A���̏������l������:
 * - ���ꕶ�����A������ helpBuffer �ɓ���Ȃ��悤�ɂ���
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
 * �q�X�g������
 * ------------
 * ���񍇐��ϊ�����������ϊ��œ��͂������� (��) �̂����A�q�X�g���Ɏc��
 * �ׂ����̂������Ahist �Ɋi�[����B
 * �q�X�g���Ɏc�������́A
 * - �O���� (�����Ƃ� 1 ����) �܂ނ��ƁA�܂���
 * - ������ 3 �����ȏ�ł��邱��
 * �ł���B�܂��A���̏������l������B
 * - ���łɃq�X�g���ɑ��݂�����́A�V���ɒǉ����Ȃ�
 * - �q�X�g���̓��e�� 10 �𒴂���Ƃ��́A�Q�Ƃ���Ă��Ȃ����̂���㏑������
 * �q�X�g�����̒u�������ɂ́A�N���b�N�A���S���Y���𗘗p���Ă���B
 * �Ȃ��A�q�X�g���ɒǉ����������ł́A�Q�ƃr�b�g�̓Z�b�g���Ȃ��B
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
 * �����ˑ��������� (�n��K�C�h��)
 * --------
 */

void TCode::updateContext() {
    if (helpMode) {
        return;
    }
    if (mode == NORMAL || mode == CAND1) {
        //<v127a - gg>
        //<gg-defg>
        char *strGG = NULL;
        //</gg-defg>
        //<gg-defg2>
        //if (ggReady) {
        if ((currentBlock == table || currentBlock == lockedBlock) && ggReady) {
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
                if (i < n)  // ���񍇐��ϊ��ɂ͕s�v
                    ;
                else if (preBuffer->length() == 1)  // ���������J�n����Ȃ�
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
        if (currentBlock == table || currentBlock == lockedBlock) {
            if (ggCand) makeCandGG();
            if (OPT_defg && !strGG) strGG = OPT_defg;
            if (OPT_defg || ggReady || maze2ggMode) {
                clearGG(table);
                if (explicitGG) makeGG(explicitGG, ggCInputted(), ittaku);
                else if (strGG) makeGG(strGG);
            }
        //</gg-defg2>
        //</gg-defg>
        }
        //</v127a - gg>
    }
}

//<v127a - gg>
//<gg-defg>
//void TCode::makeGG(MOJI *strGG) {
void TCode::makeGG(char *strGG, int start, int protectOnConflict) {
//</gg-defg>
    char *nums = "�Q\0�R\0�S\0�T\0�U\0�V\0�W\0�X";
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
            if (!p->faceGG || s->length() < protectOnConflict) {  // ���1��
                p->faceGG = face1;
            } else if (p->faceGG >= nums && p->faceGG < nums+21) {
                p->faceGG += 3;
            } else {  // ���2��
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

/* -------------------------------------------------------------------
 * ���z����
 * --------
 * ���z���Ղ����B
 * �L�[�ԍ� i �̃L�[�ɑ΂��āA
 * - vkbFace[i] : �L�[�̃t�F�C�X (�L�[�g�b�v�ɕ\�����镶��)
 * - vkbFG[i]   : �O�i�F (�����̐F)
 * - vkbBG[i]   : �w�i�F (�L�[�̐F)
 * ��ݒ肷��B
 *
 * 5 �ʂ�̏ꍇ����:
 * - OFF ���[�h             : �����\�����Ȃ�
 * - �����w���v���[�h       : �����̃X�g���[�N�Ŕw�i�F�ƃt�F�C�X��ݒ�
 * - �ʏ���̓��[�h         : ���݂̃X�g���[�N�Ŕw�i�F���A
 *                            ���݂̑J�ڏ�ԂŃt�F�C�X��ݒ�
 * - �����������I�����[�h : ���݂̌��Q�̓��e�Ńt�F�C�X��ݒ�
 * - �q�X�g�����I�����[�h : ���݂̃q�X�g�����e�Ńt�F�C�X��ݒ�A����ɁA
 *                            �q�X�g���|�C���^�̈ʒu��w�i�F�ŁA
 *                            �Q�ƃr�b�g�������Ă������O�i�F�Ŗ������Ă���
 *
 * �Ȃ��A�q�X�g�����́A���������ϊ��̌��I�����[�h�̌�₪ 10 �ȉ��̏ꍇ
 * �Ɠ��������ɂ��邽�߁A�z�[���i�� 10 �L�[�A���Ȃ킿�L�[�ԍ� 20 �` 29 �̃L�[
 * �ɐݒ肵�Ă���B
 */

void TCode::makeVKB(bool unlock) {
    // ������
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
            //case TC_BG_ST1: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST2: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST3: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STF: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STW: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STX: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            // �O���\ T-Code �X�^�C���̑Ō��}�̃}�[�N
            //// �E�\
            //case TC_BG_ST1R: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST2R: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STWR: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            // ���\
            //case TC_BG_ST1L: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_ST2L: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
            //case TC_BG_STWL: vkbFace[i] = "��"; vkbFG[i] = TC_FG_STROKE; break;
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
            // �z�[���|�W�V�����Ƃ��̏㉺�i�ɕ\������u�E�v
            default:
                if (10 <= i && i < 40 && (i % 10 <= 3 || 6 <= i % 10)) {
                    vkbFace[i] = "�E";
                    vkbFG[i] = TC_FG_STROKE;
                }
            }
        }
        return;
    } // if helpMode

    // NORMAL mode
    if (mode == NORMAL || mode == CAND1) {
        int check = 0;
        if (explicitGG && ggCInputted() < ittaku) {
            MojiBuffer hoge(strlen(explicitGG));
            hoge.pushSoft(explicitGG);
            MOJI moji = hoge.moji(ggCInputted());
            check = stTable->look(moji);
        }
        if (waitKeytop) makeVKBBG(inputtedStroke);
        else if (check) makeVKBBG(stTable->stroke);
        else makeVKBBG(currentStroke);
        for (int i = 0; i < TC_NKEYS*2; i++) {
            Block *block = (unlock?table:(waitKeytop?lockedBlock:currentBlock))->block[i];
            if (block == 0) { continue; }
            switch (block->kind()) {
            case STRING_BLOCK:
                // XXX hirakataMode �� hanzenMode �ł̕ϊ��͖�����
                vkbFace[i] = block->getFace();
                vkbFG[i] = TC_FG_NORMAL;
                //<v127a - gg>
                if (((StringBlock *)block)->flagGG)
                    vkbFG[i] = TC_FG_GG;
                //</v127a - gg>
                break;
            case CONTROL_BLOCK:
                vkbFace[i] = "";
                if (!unlock && (waitKeytop?lockedBlock:currentBlock) != table) {
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
    //// �O���\ T-Code �X�^�C���̑Ō��}
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
// ���ȕ�������͂��邽�߂̃X�g���[�N�̈ꕔ�ƂȂ�L�[�ԍ�i�ɑ΂���
// isShiftKana[i]�̒l��true�ɂ���B
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
// �C�ӂ̕����ŃV�t�g�Ƒg�ݍ��킹��ꂽ�X�g���[�N�ƂȂ�L�[�ԍ�i�ɑ΂���
// isShiftKana[i]�̒l��true�ɂ���B
void TCode::checkShiftSeq(ControlBlock *block) {
    for (int i = 0; i < TC_NKEYS; i++) {
        if ((block->block)[TC_SHIFT(i)]) {
            isShiftKana[i] = true;
            isAnyShiftSeq = true;
        }
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
    // �K�v�Ȃ��Ȃ牽�����Ȃ��Ă���
    if (record.OPT_record == 0) { return; }
    // ���͂��ĂȂ��Ȃ牽�����Ȃ��Ă���
    if (record.nstroke <= 0) { return; }

    // ���ݎ������擾
    SYSTEMTIME stTime;
    GetLocalTime(&stTime);

    // �o�͂���s���쐬
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
            "  ����: %4d"
            "  ����: %3d(%d%%)  ��������: %3d(%d%%)  �@�\: %3d(%d%%)\n",
            month, stTime.wDay, stTime.wHour, stTime.wMinute,
            record.nchar,
            record.nbushu,   rbushu,
            record.nmaze,    rmaze,
            record.nspecial, rspecial);

    // �t�@�C���ɏ����o��
    ofstream *os = new ofstream();
    os->open(record.OPT_record, ios::app);
    if (os->fail()) { return; }
    os->write(s, strlen(s));
    os->close();

    // �サ�܂�
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
    // �K�v�Ȃ��Ȃ牽�����Ȃ��Ă���
    if (stat.OPT_stat == 0) { return; }
    if (stat.map.size() == 0) { return; }

    // �t�@�C������ǂݏo���ĉ��Z
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

    // �t�@�C���ɏ����o��
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

    // �サ�܂�
    stat.map.clear();
}

void TCode::statCount(MOJI m, int how) {
    statCount(m, how, 1);
}

void TCode::statCount(MOJI m, int how, int n) {
    // �K�v�Ȃ��Ȃ牽�����Ȃ��Ă���
    if (stat.OPT_stat == 0) { return; }

    // �}�b�v���ɂ��邩�`�F�b�N
    // XXX
    if (stat.map.find(m) == stat.map.end()) {
        struct STATENT se;
        se.direct = 0;
        se.aux = 0;
        stat.map[m] = se;
    }

    // �J�E���g
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
