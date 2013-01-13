// -------------------------------------------------------------------
// class Block
//
// �����e�[�u�����i�[���邽�߂̃N���X

#ifndef BLOCK_H
#define BLOCK_H
// -------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

// -------------------------------------------------------------------
// �N���X�̎��

#define STRING_BLOCK  1
#define CONTROL_BLOCK 2
#define SPECIAL_BLOCK 3

// -------------------------------------------------------------------
// ����u���b�N�̋@�\

/* ���[�h�� */
#define F_SWITCH_MODE   10      // @s �� A/B ���[�h�̐؂�ւ� (OBSOLETE)
#define F_HIRAKATA      11      // @K �A �Ђ炪��/��������
#define F_HANZEN        12      // @Z �S ���p/�S�p
#define F_PUNCT         13      // @p �� ��Ǔ_�؂�ւ�
//#define F_HANKANA     14      // @k �� ���p��������
#define F_SHOWWIN       15      // @w �� �E�B���h�E�\����\���̐؂�ւ�
#define F_MAZE2GG       16      // @g �K �������K���[�h
/* �O�u�^ */
#define F_BUSHU_PRE     21      // @b �� �O�u�^�̕��񍇐�
#define F_MAZE_PRE      22      // @m �� �O�u�^�̌��������ϊ�
#define F_HIST          31      // @! �� �q�X�g������
/* ��� */
#define F_QUIT          99      // @q �~ �ϊ��̒��f
/* ��u�^ */
#define F_BUSHU_POST    41      // @B �� ��u�^�̕��񍇐�
#define F_MAZE_POST1    51      // @1 �� ��u�^�̌��������ϊ� (1 ����)
#define F_MAZE_POST2    52      // @2 �� ��u�^�̌��������ϊ� (2 ����)
#define F_MAZE_POST3    53      // @3 �� ��u�^�̌��������ϊ� (3 ����)
#define F_MAZE_POST4    54      // @4 �� ��u�^�̌��������ϊ� (4 ����)
#define F_MAZE_POST5    55      // @5 �� ��u�^�̌��������ϊ� (5 ����)
#define F_MAZE_POST6    56      // @6 �� ��u�^�̌��������ϊ� (6 ����)
#define F_MAZE_POST7    57      // @7 �� ��u�^�̌��������ϊ� (7 ����)
#define F_MAZE_POST8    58      // @8 �� ��u�^�̌��������ϊ� (8 ����)
#define F_MAZE_POST9    59      // @9 �� ��u�^�̌��������ϊ� (9 ����)
#define F_DAKUTEN       61      // @D �J ��u�^�̑��_
#define F_HANDAKUTEN    62      // @P �K ��u�^�̔����_
#define F_KATA_POST0    100     // @- �J ��u�^�̂������ȕϊ� (�Ђ炪�Ȍp����)
#define F_KATA_POST1    101     // @Q �J ��u�^�̂������ȕϊ� (1 ����)
#define F_KATA_POST2    102     // @R �J ��u�^�̂������ȕϊ� (2 ����)
#define F_KATA_POST3    103     // @S �J ��u�^�̂������ȕϊ� (3 ����)
#define F_KATA_POST4    104     // @T �J ��u�^�̂������ȕϊ� (4 ����)
#define F_KATA_POST5    105     // @U �J ��u�^�̂������ȕϊ� (5 ����)
#define F_KATA_POST6    106     // @V �J ��u�^�̂������ȕϊ� (6 ����)
#define F_KATA_POST7    107     // @W �J ��u�^�̂������ȕϊ� (7 ����)
#define F_KATA_POST8    108     // @X �J ��u�^�̂������ȕϊ� (8 ����)
#define F_KATA_POST9    109     // @Y �J ��u�^�̂������ȕϊ� (9 ����)
#define F_KATA_POSTH1   111     // @) �� ��u�^�̂������ȕϊ� (�Ђ炪��1������)
#define F_KATA_POSTH2   112     // @( �� ��u�^�̂������ȕϊ� (�Ђ炪��2������)
#define F_KATA_POSTH3   113     // @' �� ��u�^�̂������ȕϊ� (�Ђ炪��3������)
#define F_KATA_POSTH4   114     // @& �� ��u�^�̂������ȕϊ� (�Ђ炪��4������)
#define F_KATA_POSTH5   115     // @% �� ��u�^�̂������ȕϊ� (�Ђ炪��5������)
#define F_KATA_POSTH6   116     // @$ �� ��u�^�̂������ȕϊ� (�Ђ炪��6������)
#define F_KATA_POSTS1   121     // @@ �� ���O�̌�u�^�̂������ȕϊ��k�� (1����)
#define F_KATA_POSTS2   122     // @[ �� ���O�̌�u�^�̂������ȕϊ��k�� (2����)
#define F_KATA_POSTS3   123     // @; �� ���O�̌�u�^�̂������ȕϊ��k�� (3����)
#define F_KATA_POSTS4   124     // @: �� ���O�̌�u�^�̂������ȕϊ��k�� (4����)
#define F_KATA_POSTS5   125     // @] �� ���O�̌�u�^�̂������ȕϊ��k�� (5����)
/* �����w���v */
#define F_HELP_BACKW    71      // @h �� �����w���v (�O�̕���)
#define F_HELP_FORW     72      // @H �� �����w���v (���̕���)
/* �L�[ */
#define F_VERB_FIRST    82      // @v �E ���Ō��̃L�[
#define F_VERB_THIS     83      // @^ �E ����ł������̃L�[

// -------------------------------------------------------------------
// Block

// visitor �N���X
class BlockVisitor;

// Block �̈�ʌ` (���ۃN���X)
class Block {
 public:
    virtual char *getFace() = 0; // �\���p������
    virtual ~Block() {};
    virtual int kind() = 0;     // �N���X�̎��
    virtual void *accept(BlockVisitor *) = 0;
};

// StringBlock - ���������͂���u���b�N
class StringBlock : public Block {
 public:
    int length;                 // ������̒���
    char *str;                  // ������
    char *face;                 // �\���p������
    //<v127a - gg>
    int flagGG;                 // �n��K�C�h�Ώ�
    //</v127a - gg>
    char *getFace() { return face; }

    StringBlock(char * = 0);
    ~StringBlock();
    int kind() { return STRING_BLOCK; }
    void *accept(BlockVisitor *);
};

// ControlBlock - ����������u���b�N
class ControlBlock : public Block {
 public:
    Block **block;
    //<v127a - gg>
    char *faceGG;
    //</v127a - gg>
    char *getFace() { return (char *)"��"; }

    ControlBlock();
    ~ControlBlock();
    int kind() { return CONTROL_BLOCK; }
    void *accept(BlockVisitor *);
};

// SpecialBlock - �����̋@�\�̃u���b�N
class SpecialBlock : public Block {
 public:
    int function;           // �u���b�N�̋@�\
    char *getFace();
    SpecialBlock(char);
    ~SpecialBlock() {};
    int kind() { return SPECIAL_BLOCK; }
    void *accept(BlockVisitor *);
};

class BlockVisitor {
 public:
    virtual void *visitStringBlock(StringBlock *) = 0;
    virtual void *visitControlBlock(ControlBlock *) = 0;
    virtual void *visitSpecialBlock(SpecialBlock *) = 0;
};

// -------------------------------------------------------------------
#endif // BLOCK_H
