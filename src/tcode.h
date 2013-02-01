#ifndef TCODE_H
#define TCODE_H
/* -------------------------------------------------------------------
 * TCode �N���X - T-Code �ϊ���
 */

#include <vector>
#include <map>
#include <algorithm>
#ifdef _MSC_VER //<OKA>
using std::vector;
#endif          //</OKA>

#include "block.h"
#include "bushu_dic.h"
#include "mg_table.h"
//<v127a - gg>
#include "gg_dic.h"
//</v127a - gg>
#include "moji.h"
#include "st_table.h"
#include "tc.h"

//<record>
struct STATENT {
    int direct;                 // ���ړ��͂�����
    int aux;                    // �⏕���͂œ��͂�����
};

#define STAT_DIRECT 0
#define STAT_AUX    1

typedef std::map<MOJI, struct STATENT> StatMap;
//</record>

// �ΏۃE�B���h�E�ւ̕����o�͕��@
#define OUT_DISABLE -1
#define OUT_WMCHAR 0
#define OUT_WMIMECHAR 1
#define OUT_WMKANCHOKUCHAR 2
#define OUT_WMUNICHAR 3
#define OUT_KEYEVENTFUNICODE 4

/* TCode �N���X
 *
 * T-Code �ϊ���Ƃ��Ă͂��炭
 */
class TCode {
public:
    /* �ϊ��e�[�u����
     * --------------
     * �ϊ��퐶�����ɏ��������A���̌�͒萔�̂悤�ɗp������́B
     */
    int *vkey;                  // TC_NKEYS �̉��z�L�[�̔z��
    ControlBlock *table;        // �X�g���[�N�����ϊ��e�[�u��
    StTable *stTable;           // �X�g���[�N�t�����e�[�u��
    MgTable *mgTable;           // ���������ϊ�����
    BushuDic *bushuDic;         // ���񍇐��ώ�
    //<v127a - gg>
    GgDic *ggDic;               // �n��K�C�h����
    //</v127a - gg>
    //int isReady;              // ���������������Ă��邩
    int mazeReady;              // ���������ϊ������p�\��
    int bushuReady;             // ���񍇐������p�\��
    //<v127a - gg>
    int ggReady;                // �n��K�C�h�����p�\��
    //</v127a - gg>
    //<v127a - shiftcheck>
    bool *isShiftKana;          // Shift��Ԃ�RegisterHotKey���K�v�ȃL�[��
                                // �ǂ����������t���O���i�[����z��
    //</v127a - shiftcheck>
    bool isAnyShiftSeq;         // �e�[�u���t�@�C���ŉ��炩�̃V�t�g�Ō�����`����Ă��邩
    //<multishift2>
    DIR_TABLE dirTable;
    static void readDir(DIR_TABLE *, ifstream *);
    //</multishift2>

    /* �I�v�V������
     * ------------
     * kanchoku.ini �Őݒ肷��I�v�V����
     * �I�v�V�����ꗗ�ƃf�t�H���g�l�́Atable_window.c �� initTC() ���Q�ƁB
     */
    int OPT_hotKey;             // ON/OFF �̃L�[
//<OKA> support unmodified hot key
    int OPT_unmodifiedHotKey;   // CTRL��SHIFT�Ȃǂ�modifier�Ȃ��ł�HotKey�Ƃ��ē���
//</OKA>
    int OPT_offHotKey;          // OFF �L�[
    int OPT_unmodifiedOffHotKey;// OFF �L�[(modifier�Ȃ�)
    char *OPT_keyboard;         // �L�[�{�[�h��`�t�@�C��
    char *OPT_tableFile;        // �e�[�u����`�t�@�C��

    char *OPT_bushu;            // ���񍇐������t�@�C��
    char *OPT_mazegaki;         // ���������ϊ������t�@�C��
    //<v127a - gg>
    char *OPT_gg;               // �n��K�C�h�����t�@�C��
    //<gg-defg>
    char *OPT_defg;             // �z���ē��������X�g
    //</gg-defg>
    //</v127a - gg>
    int OPT_bushuAlgo;          // ���񍇐��ϊ��̃A���S���Y��
    int OPT_conjugationalMaze;  // ���p��̌��������ϊ�
    int OPT_maze2gg;            // �������K���[�h�������
    STROKE *OPT_prefixautoassign; // �n��K�C�h���̊O���ɃX�g���[�N����U��

    int OPT_shiftKana;          // �V�t�g�Ō��ł�������
    int OPT_shiftFallback;      // �ꕔ�ł����V�t�g�Ō����g��Ȃ��z��̏ꍇ�A
                                // �V�t�g�Ō��̒�`���Ȃ���ʂŃV�t�g�Ȃ��Ō��̒�`�ő�p
    int OPT_shiftLockStroke;    // Shift�ɂ��X�g���[�N���b�N�@�\
    int OPT_enableHankakuKana;  // ���p���ȕϊ�

    int OPT_useWMIMECHAR;       // �o�̓��b�Z�[�W
    std::map<char *, int, ltstr> OPT_outputMethodMap; // �o�͐�E�B���h�E���̏o�͕��@
    //<v127a - outputsleep>
    long OPT_outputSleep;       // ���z�L�[�� output ���� Sleep �� (ms)
    //</v127a - outputsleep>
    int OPT_outputVKeyMethod;
    int OPT_outputAlphabetAsVKey;
    int OPT_outputUnicode;

    int OPT_syncWithIME;        // IME��ON/OFF�Ɗ���Win��ON/OFF��A��������
    int OPT_onoffLocal;         // �E�B���h�E��؂�ւ���Ƃ��Ɋe�A�v����IME��Ԃ�D��
    int OPT_whatisimeon;        // �A������IME��ON/OFF�Ƃ�ON/OFF�ł���ANATIVE/ALNUM�ł���
    int OPT_syncmaster;         // ����Win��hotkey��IME���ǂ�����
    int OPT_syncslave;          // IME�̏�ԕω��Ŋ���Win���ǂ�����
    int OPT_considerIMEAction;  // WM_KANCHOKU_NOTIFYVKPROCESSKEY�p

    int OPT_xLoc;               // �E�B���h�E�����ʒu
    int OPT_yLoc;               // �V
    int OPT_offHide;            // OFF ���ɃE�B���h�E��\��
    int OPT_followCaret;        // �J�[�\���Ǐ]
    int OPT_displayHelpDelay;   // ���z���Օ\���҂�����(ms)

    int OPT_hardBS;             // BS �ŏ�ɕ���������
    int OPT_weakBS;             // BS ��1�X�g���[�N�����߂�
    int OPT_useCtrlKey;         // C-h �Ȃǂ� BS �ȂǂƂ��Ĉ���
    int OPT_clearBufOnMove;     // �J�[�\���ړ��Ō�u�^�ϊ��p�o�b�t�@����ɂ���
    int OPT_useTTCode;          // �O���\ T-Code �X�^�C���̕����w���v
    int OPT_win95;              // win95 �ł̃t�H���g�̂���̕␳
    char *OPT_offResetModes;    // �e�탂�[�h�����Z�b�g
    int OPT_strokeTimeOut;      // �Ō������������܂ł̑҂�����(ms)

    //<record>
    // �L�^�p
    struct KWRECORD {
        char *OPT_record;       // kwrecord.txt �t�@�C����
        int nchar;              // �����ړ��͕�����
        int nstroke;            // ���X�g���[�N��
        int nbushu;             // ���񍇐��œ��͂���������
        int nmaze;              // ���������œ��͂���������
        int nspecial;           // ����@�\�̎g�p��
    } record;
    void recordSetup(const char *);
    void recordOutput();

    // ���v�p
    struct KWSTAT {
        char *OPT_stat;         // kwstat.txt �t�@�C����
        StatMap map;
    } stat;
    void statSetup(const char *);
    void statOutput();
    void statCount(MOJI, int);
    void statCount(MOJI, int, int);
    //</record>

    /* ���[�h��
     * --------
     * mode
     * ----
     * �僂�[�h
     * - OFF    : OFF
     * - NORMAL : �ʏ�̃X�g���[�N���́B�~�j�o�b�t�@���͂��܂�
     * - CAND   : ���������̌��I�����[�h
     * - HIST   : �q�X�g������ (�q�X�g���̌��I�����[�h)
     *
     * helpMode, helpModeSave
     * ----------------------
     * �����w���v��\�����邩�ǂ����B
     * �����w���v�́A�ʏ� mode == NORMAL ���ɂ�����������B
     *
     * hanzenMode, hirakataMode, punctMode
     * -----------------------------------
     * mode == NORMAL ���ɂ����āA�X�g���[�N�����ϊ������������A���ꂼ��A
     * �Ђ炪��/�������ȕϊ��A���p/�S�p�ϊ��A��Ǔ_�ϊ����邩�ǂ����B
     */
    enum MODE { OFF, NORMAL, CAND, CAND1, HIST };
    MODE mode;
    int helpMode;
    int helpModeSave;
    int hanzenMode;
    int hirakataMode;
    int punctMode;
    //int hankanaMode;
    int maze2ggMode;

    /* �X�g���[�N���͂̏��
     * --------------------
     * - currentBlock   : �ϊ��e�[�u���̌��݈ʒu�������|�C���^�B
     * - currentStroke  : ���݂܂ł̃L�[�X�g���[�N��ێ�����x�N�^�B
     * - currentShift   : ���݂܂ł̃X�g���[�N��ɃV�t�g�Ō������������B
     * - lockedBlock      : �X�g���[�N���b�N�@�\�ňꎞ�I��TCode::table�̑���ɂȂ�|�C���^�B
     * - lockedStroke     : �X�g���[�N���b�N�@�\�Ń��b�N�����X�g���[�N�B
     * - inputtedStroke   : displayHelpDelay �Ńf�B���C���̕\���Ɏg���X�g���[�N�B
     */
    ControlBlock *currentBlock;
    std::vector<STROKE> *currentStroke;
    int currentShift;
    ControlBlock *lockedBlock;
    std::vector<STROKE> *lockedStroke;
    std::vector<STROKE> *inputtedStroke;

    /* ���́E�ϊ��p�o�b�t�@
     * --------------------
     * - preBuffer  : �X�g���[�N�����ϊ������������ꎞ�I�ɕێ�����o�b�t�@�B
     *                �K�v�ɉ����āA���񍇐��E���������ϊ����s��ꂽ��A
     *                �A�v���P�[�V�����ɏo�͂����B
     * - postBuffer : �A�v���P�[�V�����ɏo�͂��ꂽ (�ŋ߂�) ������ێ��B
     *                ��u�^�̕ϊ��Ŏg�p����BTableWindow::output() �����ōX�V�B
     * - postDelete : ��u�^�̕ϊ��ŁA�x���f���[�g���镶�����B
     * - helpBuffer : �����w���v�̑ΏۂƂȂ镶�� (�⏕�ϊ��œ��͂�������) ��
     *                �ێ�����o�b�t�@�B
     * - helpOffset : helpBuffer ���ŁA���ݕ����w���v��\�����镶�����w���B
     */
    MojiBuffer *preBuffer;
    MojiBuffer *postBuffer;
    int postDelete;
    //<v127c - postInPre>
    int postInPre;
    int postKataPrevLen; // ���O�̌�u�^�̂������ȕϊ��ŕϊ�����������
    //</v127c>
    MojiBuffer *helpBuffer;
    int helpOffset;

    /* ���������ϊ�
     * ------------
     *
     * candOrder[], candSkip
     * ---------------------
     * ���������ϊ��̌��I���̃L�[�̗D�揇�ʁA����сA
     * �O���Q/�����Q�̃X�L�b�v�ʁBtcode.c �ł̒�`���Q�ƁB
     *
     * currentCand, candOffset
     * -----------------------
     * ���������ϊ��́A���݂̓ǂ݂ɑ΂���S���̃x�N�^�A����сA
     * ���݉��z���Ղɕ\�����Ă�����Q�̃I�t�Z�b�g�B
     *
     * - yomi       : ���݂̌��������ϊ��̓ǂ�
     * - yomiLen    : ���݂̌��������ϊ��̓ǂ݂̒���
     * - cand       : �I�΂ꂽ���
     */
    static const int candOrder[TC_NKEYS];
    static const unsigned int candSkip;
    std::vector<char *> *currentCand;
    unsigned int candOffset;
    char yomi[TC_BUFFER_SIZE * 2 + 1];
    int yomiLen;
    int okuriLen;
    char *cand;

    /* �q�X�g������
     * ------------
     * - hist[]     : ���݂̃q�X�g�����̑S���
     * - histRef[]  : �e�q�X�g�����̎Q�ƃr�b�g
     * - histPtr    : ���O�ɒǉ����ꂽ�q�X�g�����̈ʒu
     */
    char *hist[TC_NHIST];
    int histRef[TC_NHIST];
    int histPtr;

    /* �n��K�C�h�A�������K���[�h�A�O����������U��
	 */
    std::vector<char *> *ggCand;
    int ggCHeaderLen;  // ���͍ς݂Ƃ��镶����
    int ggCStart;  // preBuffer or postBuffer�ł̕ϊ����ʊJ�n�ʒu
                   // ex. preBuffer == "����", [���] �� ggCStart == 1
                   //     preBuffer == "", postBuffer == "...��", [���] �� ggCStart == -1
    int ittaku;  // ���
    char *explicitGG;
    MojiBuffer *assignedsBuffer;

    /* ���z����
     * --------
     * - vkbFace[]  : ���z���Ղ̊e�L�[�̃t�F�C�X
     * - vkbFG[]    : ���z���Ղ̊e�L�[�̑O�i�F�B��L TC_FG_* ���Q�ƁB
     * - vkbBG[]    : ���z���Ղ̊e�L�[�̔w�i�F�B��L TC_BG_* ���Q�ƁB
     * - waitKeytop : displayHelpDelay �o�ߑO���ǂ���
     */
    char *vkbFace[TC_NKEYS*2];
    int vkbFG[TC_NKEYS*2];
    int vkbBG[TC_NKEYS];
    int vkbCorner[TC_NKEYS];
    int waitKeytop;

    /* �R���X�g���N�^�ƃf�X�g���N�^
     */
    //<v127a - gg>
    //TCode(int *, ControlBlock *, MgTable *, BushuDic *);
    TCode(int *, ControlBlock *, MgTable *, BushuDic *, GgDic *);
    //</v127a - gg>
    ~TCode();

    /* ���Z�b�g
     */
    void reset();
    void resetBuffer();
    void lockStroke();
    void unlockStroke();

    /* �L�[����
     * --------
     * NORMAL, CAND, HIST �̊e���[�h�ɑΉ�����A�L�[���͏������[�`��
     */
    void keyinNormal(int);
    void keyinNormalKataPost(int);
    void keyinNormalKataPostShrink(int);
    void keyinCand(int);
    void keyinCand1(int);
    void keyinHist(int);

    /* �o�b�t�@�Ǘ�
     * ------------
     * - postBufferDeleted()    : postBuffer�̖�������pop()�����i�A�v����Backspace�𑗂����j�Ƃ��B
     * - postBufferCount()      : preBuffer�̓��e��postBuffer�֓]�������i�A�v���ɕ�����𑗂����j�Ƃ��B
     */
    void postBufferDeleted(int);
    void postBufferCount(int);

    /* �⏕�ϊ�
     * --------
     * ��������ApreBuffer �̓��e�Ɋւ�����́B
     * - isReducibleByBushu()   : ���񍇐��ϊ������s���ׂ����e���ǂ����B
     * - isReducibleByMaze()    : ���������ϊ������s���ׂ����e���ǂ����B
     * - isReducibleByKata()    : �������ȕϊ������s���ׂ����e���ǂ����B
     * - reduceByBushu()        : ���񍇐��ϊ������s�B
     * - reduceByMaze()         : ���������ϊ������s�B
     * - reduceByKata()         : �������ȕϊ������s�B
     * - nfer()                 : ���ϊ��Ŋm��B
     * - nferHirkata()          : �Ђ炪��/�������ȕϊ����s���A�m��B
     * - isComplete()           : ���񍇐�����������ϊ��������������ǂ����B
     */
    int isReducibleByBushu();
    int isReducibleByMaze();
    int isReducibleByKata();
    void reduceByBushu();
    void reduceByMaze();
    void reduceByKata();
    void finishCand(char *);
    void makeMazeYomiLonger();
    void makeMazeYomiShorter();
    void nfer();
    void nferHirakata();
    int isComplete();
    //<v127c - postInPre>
    void cancelPostInPre(int);
    //</v127c>

    void setCandGGHeader();
    void goCandGG();  // finishCand()����
    void makeCandGG();  // output()��ɌĂ�
    void clearCandGG();
    int ggCInputted();

    /* �����w���v
     * ----------
     * �⏕�ϊ��œ��͂��������̂����A�����w���v�̑Ώۂɂ��ׂ����̂������A
     * helpBuffer �ɒǉ�����֐��B
     * �ƁA���̉������B
     */
    void addToHelpBuffer(MOJI);
    void addToHelpBufferMaybe(MOJI);
    void addToHelpBufferMaybe(char *, char *);

    /* �q�X�g������
     * ------------
     * �⏕�ϊ��œ��͂��������̂����A�q�X�g���Ɏc���ׂ����̂������A
     * hist �ɒǉ�����֐��B
     * �q�X�g���Ɏc���ׂ������̏����́A�֐��̒�`���Q�ƁB
     */
    void addToHistMaybe(char *);
    void addToHistMaybe(MOJI);

    /* �����ˑ��������� (�n��K�C�h��)
     */
    void updateContext();
    //<v127a - gg>
    //<gg-defg>
    //void makeGG(MOJI *);
    void makeGG(char *strGG, int start = 0, int protectOnConflict = 0);
    //</gg-defg>
    void clearGG(ControlBlock *);
    //</v127a - gg>/
    void assignStroke(char *);
    void assignStroke(MOJI);
    void clearAssignStroke();

    /* ���z����
     * --------
     * - makeVKB()      : ���z���Ղ��쐬�B
     *                    �e�L�[�̃t�F�C�X�E�O�i�F�E�w�i�F��ݒ�B
     * - makeVKBBG()    : makeVKB �̉������B
     *                    �X�g���[�N�ɏ]���Ċe�L�[�̔w�i�F��ݒ�B
     */
    void makeVKB(bool unlock = false);
    void makeVKBBG(vector<STROKE> *);
    void makeVKBBG(STROKE *);

    //<v127a - shiftcheck>
    /* isShiftKana[]���Z�b�g���� */
    bool checkShiftKana(ControlBlock *block);
    //</v127a - shiftcheck>
    void checkShiftSeq(ControlBlock *block);
};

// -------------------------------------------------------------------
#endif // TCODE_H
