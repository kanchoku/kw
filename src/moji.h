#ifndef MOJI_H
#define MOJI_H
/* -------------------------------------------------------------------
 * MOJI �^
 * -------
 *
 * ������ MOJI (== unsigned short) �^�ň����B
 * �ʏ�̕����̂ق��ɁA���z�L�[��A���ꕶ�� (���񍇐�����������ϊ��̊J�n�_)
 * ���AMOJI �^�ň������Ƃɂ��Ă���B
 * �����Ƃ���ɑΉ����� MOJI �^�̕\�� m �́A���̂Ƃ���B
 *
 * - ASCII ���� c   : m = (MOJI)c
 * - ���p���� k     : m = (MOJI)k        (0x80 <= k)
 * - �S�p (h l)     : m = B2MOJI(h, l)   (0x80 <= h)
 *
 * ���p���Ȃ������ƈ����̂Ȃ�A���_�┼���_���A���O�̕����ƂƂ���
 * �܂Ƃ߂� 1 �����Ƃ���̂����������m��Ȃ��B
 * �S�p����/���p���� �̕ϊ������鎞�Ƃ��B
 * <hankana>
 * �c�c�Ǝv�������A
 * ��͂�A���_�┼���_����ɓƗ����������Ƃ��Ĉ������Ƃɂ����B
 * </hankana>
 */

typedef unsigned short MOJI;

/* -------------------------------------------------------------------
 * char �ȂǂƂ̑��ݕϊ�
 */

// �S�p���� (�� 1 �o�C�g��) ���ǂ���
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

// ���p���ȕ������ǂ���
#define IS_HANKANA(c) (((c) & 0x80) && \
                       (0x20 <= MSBOFF(c) && MSBOFF(c) <= 0x5f))

// 2 ������ char ���� MOJI �^�𐶐�
#if 1  //<OKA> This may be better...
#define B2MOJI(h, l) MOJI((unsigned)((h) & 0xff) << 8 | (unsigned)((l) & 0xff))
#else  //<OKA> ...than below.
#define B2MOJI(h, l) ((unsigned)((h) & 0xff) << 8 | (unsigned)((l) & 0xff))
#endif //</OKA>

//<multishift>
// ������ s �̐擪�� 1 ������ MOJI �^�ɕϊ�
#define STR2MOJI(s) B2MOJI(*(s), *((s) + 1))
//</multishift>

// MOJI ���� h �� l �����o��
#define MOJI2H(m) (((m) >> 8) & 0xff)
#define MOJI2L(m) ((m) & 0xff)

// MOJI �̎�ނ�Ԃ�
int mojitype(MOJI);

// char ������̐擪�� 1 ������ MOJI �^�Ŏ��o��
MOJI str2moji(const char *, char **);

// MOJI ���A������o�b�t�@�ɒǉ����ď������� (strcat �̂悤��)
char *moji2strcat(char *, MOJI);

/* -------------------------------------------------------------------
 * MOJI �^�̊e��ϊ�
 * - mojiHirakata() : �Ђ炪��/�������ȕϊ�
 * - mojiKata()     : �������Ȃւ̕ϊ�(�������ȁ��Ђ炪�ȕϊ��͍s��Ȃ�)
 * - mojiHanzen()   : ���p/�S�p�ϊ� (ASCII ����)
 * - mojiDaku()     : ����/�����ϊ�
 * - mojiHandaku()  : ����/�������ϊ�
 * - mojiPunct()    : ��Ǔ_�ϊ� (�u�A�B�v/�u�C�D�v)
 * - mojiHankana()  : <hankana/> �S�p���� �� ���p���� �̕ϊ�
 */
MOJI mojiHirakata(MOJI);
MOJI mojiKata(MOJI);
MOJI mojiHanzen(MOJI);
MOJI mojiDaku(MOJI);
MOJI mojiHandaku(MOJI);
MOJI mojiPunct(MOJI);
//<hankana>
int mojiHankana(MOJI, MOJI *, MOJI *);
//</hankana>

/* -------------------------------------------------------------------
 * �����̎��
 */
#define MOJI_UNKNOWN -1
#define MOJI_ASCII    0         // ASCII ����
#define MOJI_HANKANA 'K'        // ���p����
#define MOJI_ZENKAKU 'Z'        // �S�p
#define MOJI_UNICODE 'U'        // ���j�R�[�h

#define MOJI_SPECIAL '@'        // ����
#define MOJI_VKEY    '!'        // ���z�L�[
#define MOJI_CTRLVKY '"'        // Ctrl+���z�L�[

/* -------------------------------------------------------------------
 * ���ꕶ��
 */
#define MOJI_BUSHU B2MOJI('@', 'b') // �O�u���񍇐��̕ϊ��J�n�_�̃}�[�J
#define MOJI_MAZE  B2MOJI('@', 'm') // �O�u���������̕ϊ��J�n�_�̃}�[�J

/* -------------------------------------------------------------------
 * MojiBuffer
 * ----------
 * �Œ�T�C�Y�� MOJI �^�̃o�b�t�@
 * pushHard() ���g�����Ƃɂ��A�����O�o�b�t�@�Ƃ��Ďg����B
 */
class MojiBuffer {
    int size;                   // �o�b�t�@�̑傫��
    MOJI *buf;                  // size �̑傫�������z��
    int beg;                    // ���e�̐擪�ʒu (0 .. (bufferSize - 1))
    int len;                    // ���e�̒���     (0 .. bufferSize)
    //int pt;                   // ���e�̒��𓮂��|�C���^ (0 .. (len - 1))
    char *str;                  // ���e�𕶎���Ƃ��ĕԂ����Ɏg���ꎞ�o�b�t�@

public:
    MojiBuffer(int);
    ~MojiBuffer();

    void clear();               // ���e�̏���
    int length();               // ������
    int isEmpty();              // �o�b�t�@���󂩂ǂ���
    int isFull();               // �o�b�t�@����t���ǂ���
    void pushSoft(MOJI);        // �}�� (size �𒴂��Ȃ�)
    void pushSoft(char *);      // �V
    void pushSoftN(MOJI, int);  // �V (�������̂� N ��)
    void pushSoftN(char *, int); // �V
    void pushHard(MOJI);        // �}�� (���ߕ��͏㏑��)
    void pushHard(char *);      // �V
    MOJI pop();                 // ������ 1 �����̎��o��
    MOJI popN(int);             // ������ N �����̎��o��
    MOJI moji(int);             // �w��ʒu�̕���
    char *string(int, int);     // �w��ʒu�E�w�蒷���̕�����
    char *string(int);          // �w��ʒu�̕�����
    char *string();             // ������Ƃ��ĕԂ�
};

// -------------------------------------------------------------------
#endif // MOJI_H
