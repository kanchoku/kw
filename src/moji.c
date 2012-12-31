#include <string.h>

#include "moji.h"
#include "debug.h"

/* -------------------------------------------------------------------
 * ���� m �̎�ނ�Ԃ�
 */
int mojitype(MOJI m) {
    int h = MOJI2H(m);
    int l = MOJI2L(m);

    if (IS_ZENKAKU(h)) { return MOJI_ZENKAKU; }
    if (h == '@' && l >= 0x80 || h > '@' && h <= 0x7F) { return MOJI_UNICODE; }  // test

    switch (h) {
    case 0:
    if (IS_HANKANA(l)) { return MOJI_HANKANA; }
    else { return MOJI_ASCII; }

    case MOJI_VKEY:
    return MOJI_VKEY;

    case MOJI_CTRLVKY:
    return MOJI_CTRLVKY;

    case MOJI_SPECIAL:
    return MOJI_SPECIAL;

    default:
    return MOJI_UNKNOWN;
    }
}

/* -------------------------------------------------------------------
 * char ������ src �̐擪�� 1 ������ MOJI �^�Ŏ��o���ĕԂ��B
 * pnext �� NULL �łȂ���΁A*pnext �Ɏ��̕����ʒu��ݒ肷��B
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
 * ���� moji ���A������ dst �̌��ɘA�����Adst ��Ԃ��B
 * strlen(dst) �̑����ʂ͍ő�� 2�B
 * �̈�̃`�F�b�N�͍s��Ȃ��B
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
 * MOJI �^�̊e��ϊ�
 */
#define MS(s) B2MOJI(*(s), *((s) + 1))
#define MC(c) B2MOJI(0, (c))

/* �Ђ炪��/�������� �̘A�z�e�[�u���B
 * 0 �ŏI�[���邱�ƁB
 */
static MOJI tblHirakata[][2] = {
    {MS("��"), MS("�@")}, {MS("��"), MS("�A")}, {MS("��"), MS("�B")},
    {MS("��"), MS("�C")}, {MS("��"), MS("�D")}, {MS("��"), MS("�E")},
    {MS("��"), MS("�F")}, {MS("��"), MS("�G")}, {MS("��"), MS("�H")},
    {MS("��"), MS("�I")},
    {MS("��"), MS("�J")}, {MS("��"), MS("�K")}, {MS("��"), MS("�L")},
    {MS("��"), MS("�M")}, {MS("��"), MS("�N")}, {MS("��"), MS("�O")},
    {MS("��"), MS("�P")}, {MS("��"), MS("�Q")}, {MS("��"), MS("�R")},
    {MS("��"), MS("�S")},
    {MS("��"), MS("�T")}, {MS("��"), MS("�U")}, {MS("��"), MS("�V")},
    {MS("��"), MS("�W")}, {MS("��"), MS("�X")}, {MS("��"), MS("�Y")},
    {MS("��"), MS("�Z")}, {MS("��"), MS("�[")}, {MS("��"), MS("�\")},
    {MS("��"), MS("�]")},
    {MS("��"), MS("�^")}, {MS("��"), MS("�_")}, {MS("��"), MS("�`")},
    {MS("��"), MS("�a")}, {MS("��"), MS("�b")}, {MS("��"), MS("�c")},
    {MS("��"), MS("�d")}, {MS("��"), MS("�e")}, {MS("��"), MS("�f")},
    {MS("��"), MS("�g")}, {MS("��"), MS("�h")},
    {MS("��"), MS("�i")}, {MS("��"), MS("�j")}, {MS("��"), MS("�k")},
    {MS("��"), MS("�l")}, {MS("��"), MS("�m")},
    {MS("��"), MS("�n")}, {MS("��"), MS("�o")}, {MS("��"), MS("�p")},
    {MS("��"), MS("�q")}, {MS("��"), MS("�r")}, {MS("��"), MS("�s")},
    {MS("��"), MS("�t")}, {MS("��"), MS("�u")}, {MS("��"), MS("�v")},
    {MS("��"), MS("�w")}, {MS("��"), MS("�x")}, {MS("��"), MS("�y")},
    {MS("��"), MS("�z")}, {MS("��"), MS("�{")}, {MS("��"), MS("�|")},
    {MS("��"), MS("�}")}, {MS("��"), MS("�~")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")},
    {0, 0}
};

/* ���p/�S�p �̘A�z�e�[�u���B
 * 0 �ŏI�[���邱�ƁB
 */
static MOJI tblHanzen[][2] = {
    {MC(' '), MS("�@")},         // �S�p��
    {MC('!'), MS("�I")}, {MC('"'), MS("�h")}, {MC('#'), MS("��")},
    {MC('$'), MS("��")}, {MC('%'), MS("��")}, {MC('&'), MS("��")},
    {MC('\''), MS("�f")}, {MC('('), MS("�i")}, {MC(')'), MS("�j")},
    {MC('*'), MS("��")}, {MC('+'), MS("�{")}, {MC(','), MS("�C")},
    {MC('-'), MS("�|")}, {MC('.'), MS("�D")}, {MC('/'), MS("�^")},
    {MC('0'), MS("�O")}, {MC('1'), MS("�P")}, {MC('2'), MS("�Q")},
    {MC('3'), MS("�R")}, {MC('4'), MS("�S")}, {MC('5'), MS("�T")},
    {MC('6'), MS("�U")}, {MC('7'), MS("�V")}, {MC('8'), MS("�W")},
    {MC('9'), MS("�X")},
    {MC(':'), MS("�F")}, {MC(';'), MS("�G")}, {MC('<'), MS("��")},
    {MC('='), MS("��")}, {MC('>'), MS("��")}, {MC('?'), MS("�H")},
    {MC('@'), MS("��")},
    {MC('A'), MS("�`")}, {MC('B'), MS("�a")}, {MC('C'), MS("�b")},
    {MC('D'), MS("�c")}, {MC('E'), MS("�d")}, {MC('F'), MS("�e")},
    {MC('G'), MS("�f")}, {MC('H'), MS("�g")}, {MC('I'), MS("�h")},
    {MC('J'), MS("�i")}, {MC('K'), MS("�j")}, {MC('L'), MS("�k")},
    {MC('M'), MS("�l")}, {MC('N'), MS("�m")}, {MC('O'), MS("�n")},
    {MC('P'), MS("�o")}, {MC('Q'), MS("�p")}, {MC('R'), MS("�q")},
    {MC('S'), MS("�r")}, {MC('T'), MS("�s")}, {MC('U'), MS("�t")},
    {MC('V'), MS("�u")}, {MC('W'), MS("�v")}, {MC('X'), MS("�w")},
    {MC('Y'), MS("�x")}, {MC('Z'), MS("�y")},
    {MC('['), MS("�m")}, {MC('\\'), MS("��")}, {MC(']'), MS("�n")},
    {MC('^'), MS("�O")}, {MC('_'), MS("�Q")},
    //<127e>
    //{MC('`'), MS("�M")},
    {MC('`'), MS("�e")},
    //</127e>
    {MC('a'), MS("��")}, {MC('b'), MS("��")}, {MC('c'), MS("��")},
    {MC('d'), MS("��")}, {MC('e'), MS("��")}, {MC('f'), MS("��")},
    {MC('g'), MS("��")}, {MC('h'), MS("��")}, {MC('i'), MS("��")},
    {MC('j'), MS("��")}, {MC('k'), MS("��")}, {MC('l'), MS("��")},
    {MC('m'), MS("��")}, {MC('n'), MS("��")}, {MC('o'), MS("��")},
    {MC('p'), MS("��")}, {MC('q'), MS("��")}, {MC('r'), MS("��")},
    {MC('s'), MS("��")}, {MC('t'), MS("��")}, {MC('u'), MS("��")},
    {MC('v'), MS("��")}, {MC('w'), MS("��")}, {MC('x'), MS("��")},
    {MC('y'), MS("��")}, {MC('z'), MS("��")},
    {MC('{'), MS("�o")}, {MC('|'), MS("�b")}, {MC('}'), MS("�p")},
    {MC('~'), MS("�P")},
    {0, 0}
};

/* ����/���� �̘A�z�e�[�u���B
 * 0 �ŏI�[���邱�ƁB
 */
static MOJI tblDaku[][2] = {
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")},

    {MS("�A"), MS("�@")}, {MS("�C"), MS("�B")}, {MS("�E"), MS("�D")},
    {MS("�G"), MS("�F")}, {MS("�I"), MS("�H")},
    {MS("�J"), MS("�K")}, {MS("�L"), MS("�M")}, {MS("�N"), MS("�O")},
    {MS("�P"), MS("�Q")}, {MS("�R"), MS("�S")},
    {MS("�T"), MS("�U")}, {MS("�V"), MS("�W")}, {MS("�X"), MS("�Y")},
    {MS("�Z"), MS("�[")}, {MS("�\"), MS("�]")},
    {MS("�^"), MS("�_")}, {MS("�`"), MS("�a")}, {MS("�c"), MS("�d")},
    {MS("�e"), MS("�f")}, {MS("�g"), MS("�h")},
    {MS("�n"), MS("�o")}, {MS("�q"), MS("�r")}, {MS("�t"), MS("�u")},
    {MS("�w"), MS("�x")}, {MS("�z"), MS("�{")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")},

    //<hankana>
    {MC('�'), MC('�')}, {MC('�'), MC('�')}, {MC('�'), MC('�')}, 
    {MC('�'), MC('�')}, {MC('�'), MC('�')}, 
    {MC('�'), MC('�')}, {MC('�'), MC('�')}, {MC('�'), MC('�')}, 
    //</hankana>
    {0, 0}
};

/* ����/�������̘A�z�e�[�u���B
 * 0 �ŏI�[���邱�ƁB
 */
static MOJI tblHandaku[][2] = {
    //<v127b>
    {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},
    //</v127b>
    {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")}, {MS("��"), MS("��")},
    {MS("��"), MS("��")}, {MS("��"), MS("��")},

    {MS("�E"), MS("��")},
    {MS("�J"), MS("��")}, {MS("�P"), MS("��")},
    {MS("�c"), MS("�b")},
    {MS("�n"), MS("�p")}, {MS("�q"), MS("�s")}, {MS("�t"), MS("�v")},
    {MS("�w"), MS("�y")}, {MS("�z"), MS("�|")},

    //<hankana>
    {MC('�'), MC('�')}, 
    //</hankana>
    {0, 0}
};

/* ��Ǔ_�̘A�z�e�[�u���B
 * 0 �ŏI�[���邱�ƁB
 */
static MOJI tblPunct[][2] = {
    {MS("�A"), MS("�C")}, {MS("�B"), MS("�D")},
    {0, 0}
};


//<hankana>
#define MK(s)    B2MOJI(0, *(s)), 0
#define MD(s)    B2MOJI(0, *(s)), B2MOJI(0, *((s) + 1))

/* �S�p���� �� ���p���� �̘A�z�e�[�u���B
 * �e�v�f�́A
 *   { <�S�p�� MOJI> , <���p�� MOJI> , 0 }            (�����̏ꍇ)
 * �܂��́A
 *   { <�S�p�� MOJI> , <���p�� MOJI> , <�J�܂��́K> } (�����E�������̏ꍇ)
 * �ł���B
 * 0 �ŏI�[���邱�ƁB
 */
static MOJI tblHankana[][3] = {
    {MS("�B"), MK("�")}, {MS("�u"), MK("�")}, {MS("�v"), MK("�")}, 
    {MS("�A"), MK("�")}, {MS("�E"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("�@"), MK("�")}, {MS("�B"), MK("�")}, {MS("�D"), MK("�")}, 
    {MS("�F"), MK("�")}, {MS("�H"), MK("�")}, 
    {MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("�b"), MK("�")}, {MS("�["), MK("�")}, 
    {MS("�A"), MK("�")}, {MS("�C"), MK("�")}, {MS("�E"), MK("�")}, 
    {MS("�G"), MK("�")}, {MS("�I"), MK("�")}, 
    {MS("�J"), MK("�")}, {MS("�L"), MK("�")}, {MS("�N"), MK("�")}, 
    {MS("�P"), MK("�")}, {MS("�R"), MK("�")}, 
    {MS("�T"), MK("�")}, {MS("�V"), MK("�")}, {MS("�X"), MK("�")}, 
    {MS("�Z"), MK("�")}, {MS("�\"), MK("�")}, 
    {MS("�^"), MK("�")}, {MS("�`"), MK("�")}, {MS("�c"), MK("�")}, 
    {MS("�e"), MK("�")}, {MS("�g"), MK("�")}, 
    {MS("�i"), MK("�")}, {MS("�j"), MK("�")}, {MS("�k"), MK("�")}, 
    {MS("�l"), MK("�")}, {MS("�m"), MK("�")}, 
    {MS("�n"), MK("�")}, {MS("�q"), MK("�")}, {MS("�t"), MK("�")}, 
    {MS("�w"), MK("�")}, {MS("�z"), MK("�")}, 
    {MS("�}"), MK("�")}, {MS("�~"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    {MS("�J"), MK("�")}, {MS("�K"), MK("�")}, 

    {MS("�K"), MD("��")},{MS("�M"), MD("��")},{MS("�O"), MD("��")},
    {MS("�Q"), MD("��")},{MS("�S"), MD("��")},
    {MS("�U"), MD("��")},{MS("�W"), MD("��")},{MS("�Y"), MD("��")},
    {MS("�["), MD("��")},{MS("�]"), MD("��")},
    {MS("�_"), MD("��")},{MS("�a"), MD("��")},{MS("�d"), MD("��")},
    {MS("�f"), MD("��")},{MS("�h"), MD("��")},
    {MS("�o"), MD("��")},{MS("�r"), MD("��")},{MS("�u"), MD("��")},
    {MS("�x"), MD("��")},{MS("�{"), MD("��")},
    {MS("�p"), MD("��")},{MS("�s"), MD("��")},{MS("�v"), MD("��")},
    {MS("�y"), MD("��")},{MS("�|"), MD("��")},

    //{MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //{MS("��"), MK("�")}, {MS("��"), MK("�")}, 
    //
    //{MS("��"), MD("��")},{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},{MS("��"), MD("��")},
    //{MS("��"), MD("��")},{MS("��"), MD("��")},
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

MOJI mojiKata(MOJI moji) {
    MOJI (*m)[2] = tblHirakata;
    for (; (*m)[0] != 0; m++) {
        if (moji == (*m)[0]) {
            return (*m)[1];
        }
    }
    return moji;
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
/* �S�p���� �� ���p���� �̕ϊ��B
 * zen == �w���x �� *han == �w��x; *daku == 0;      �Ԗߒl 1
 * zen == �w�K�x �� *han == �w��x; *daku == �w�J�x; �Ԗߒl 1
 * zen == �w���x �� *han == �w���x; *daku == 0;     �Ԗߒl 0
 * �t�ϊ� (���p���� �� �S�p����) �̓_���B
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
 * MojiBufer �N���X
 */

#define REM(x) ((x) % size)

// �R���X�g���N�^ (�T�C�Y���w��)
MojiBuffer::MojiBuffer(int n) {
    size = n;
    buf = new MOJI[size];
    str = new char[n * 2 + 1]; // 1 �����͍��X 2 �o�C�g
    clear();
}

// �f�X�g���N�^
MojiBuffer::~MojiBuffer() {
    delete(buf);
    delete(str);
}

// -------------------------------------------------------------------

// ���e������
void MojiBuffer::clear() {
    beg = 0; len = 0; //pt = 0;
}

// ���e�̒���
int MojiBuffer::length() {
    return len;
}

// �󂩂ǂ���
int MojiBuffer::isEmpty() {
    return (len == 0);
}

// ��t���ǂ���
int MojiBuffer::isFull() {
    return (len == size);
}

// -------------------------------------------------------------------

// �����ɕ�����ǉ��B
// ��������t�̎��́A�ǉ����Ȃ� (�������Ȃ�)�B
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
// �����ɕ�����ǉ��B
// ��������t�̎��́A�擪�̕������̂ĂāA�����ɒǉ�����B

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
// ��łȂ����A�����̕�������菜���A�����Ԃ��B

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
/* �o�b�t�@�̓��e�̈ʒu offset �̕�����Ԃ��B
 * offset : -len .. (len - 1)
 * offset �����̏ꍇ�A���e�̖������琔����B
 */
MOJI MojiBuffer::moji(int offset) {
    if (len == 0) { return (MOJI)0; } //XXX

    offset = (offset + len) % len;
    return buf[REM(beg + offset)];
}

/* �o�b�t�@�̓��e�̈ʒu offset ����n�܂钷�� n �̗��
 * ������ɂ��ĕԂ��B
 * offset : -len .. (len - 1)
 * n      : 0 .. (len - offset)
 * offset �����̏ꍇ�A���e�̖������琔����B
 * n ���傫������ꍇ�́A���e�̖����܂ŁB
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

// ���e�� offset �ʒu�ȍ~�𕶎���ɂ��ĕԂ��B
char *MojiBuffer::string(int offset) {
    offset = (offset + len) % len;
    return string(offset, len - offset);
}

// ���e�S�̂𕶎���ɂ��ĕԂ��B
char *MojiBuffer::string() {
    return string(0, len);
}

#undef REM
