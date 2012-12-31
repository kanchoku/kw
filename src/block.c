#include "block.h"
#include "tc.h"
#include "debug.h"
// -------------------------------------------------------------------

StringBlock::StringBlock(char *s) {
    //<v127a - gg>
    flagGG = 0;
    //</v127a - gg>
    if (s == 0) {               // �����񂪂Ȃ��ꍇ
        str = 0;
    } else {                    // �����񂪂���ꍇ - �������ۑ�����
        length = strlen(s);
        int l = length + 1;
        str = new char[l];
        strcpy(str, s);

        // 2 �o�C�g�� face ��p�ӂ���
        // 2 �o�C�g�ȉ��̏ꍇ�́A�󔒂Ŗ��߂�
        // 2 �o�C�g�ȏ�̏ꍇ�́Astr �����̂܂܎g��
#if 0
        if (length < 2) {
            face = new char[3];
            if (length == 0) { face[0] = ' '; }
            else             { face[0] = str[0]; }
            face[1] = ' ';
            face[2] = 0;
        } else {
            face = str;
        }
#endif
        // �c�c�Ǝv�������A1 �o�C�g�� 1 �o�C�g�̂܂܂Ƃ����d�l�ɕύX
        switch (length) {
        case 0: face = NULL; break;
        case 1: face = new char[2]; strcpy(face, str); break;
        default:
            if (strlen(str) >= 6 && str[0] == 'U' && str[1] == '+') {
                face = new char[strlen(str)+1];
                strcpy(face, str);
                break;
            }
            face = new char[3];
            strncpy(face, str, 2); face[2] = 0;
            break;
        }
    }
}

StringBlock::~StringBlock() {
    if (str != face) { delete(face); }
    delete(str);
}

// -------------------------------------------------------------------

ControlBlock::ControlBlock() {
    //<v127a - gg>
    faceGG = NULL;
    //</v127a - gg>
    /* �Ԃ炳����u���b�N���i�[����̈���m�ۂ� 0 �ɏ�����
     */
    block = new Block *[TC_NKEYS*2];
    for (int i = 0; i < TC_NKEYS*2; i++) { block[i] = 0; }
}

ControlBlock::~ControlBlock() {
    /* �Ԃ炳�����Ă�u���b�N��S������
     */
    for (int i = 0; i < TC_NKEYS*2; i++) { delete(block[i]); }
}

// -------------------------------------------------------------------

SpecialBlock::SpecialBlock(char c) {
    switch (c) {
    case 's':                   // A/B ���[�h�؂�ւ� (OBSOLETE)
        function = F_SWITCH_MODE; break;
    case 'K':                   // �Ђ炪��/��������
        function = F_HIRAKATA; break;
    case 'Z':                   // ���p/�S�p
        function = F_HANZEN; break;
    case 'p':                   // ��Ǔ_�؂�ւ�
        function = F_PUNCT; break;
    case 'g':                   // �������K���[�h�؂�ւ�
        function = F_MAZE2GG; break;
    case 'w':                   // �E�B���h�E�\����\���؂�ւ�
        function = F_SHOWWIN; break;

    case 'b':                   // �O�u�^�̕��񍇐��ϊ�
        function = F_BUSHU_PRE; break;
    case 'm':                   // �O�u�^�̌��������ϊ�
        function = F_MAZE_PRE; break;
    case '!':                   // �q�X�g������
        function = F_HIST; break;

    case 'q':                   // �ϊ��̒��f
        function = F_QUIT; break;

    case 'B':                   // ��u�^�̕��񍇐��ϊ�
        function = F_BUSHU_POST; break;
    case '1': function = F_MAZE_POST1; break; // ��u�^�̌��������ϊ� (1 ����)
    case '2': function = F_MAZE_POST2; break; // ��u�^�̌��������ϊ� (2 ����)
    case '3': function = F_MAZE_POST3; break; // ��u�^�̌��������ϊ� (3 ����)
    case '4': function = F_MAZE_POST4; break; // ��u�^�̌��������ϊ� (4 ����)
    case '5': function = F_MAZE_POST5; break; // ��u�^�̌��������ϊ� (5 ����)
    case '6': function = F_MAZE_POST6; break; // ��u�^�̌��������ϊ� (6 ����)
    case '7': function = F_MAZE_POST7; break; // ��u�^�̌��������ϊ� (7 ����)
    case '8': function = F_MAZE_POST8; break; // ��u�^�̌��������ϊ� (8 ����)
    case '9': function = F_MAZE_POST9; break; // ��u�^�̌��������ϊ� (9 ����)
    case 'D':                   // ��u�^�̑��_
        function = F_DAKUTEN; break;
    case 'P':                   // ��u�^�̔����_
        function = F_HANDAKUTEN; break;
    case '-': function = F_KATA_POST0; break; // ��u�^�̂������ȕϊ� (X ����)
    case 'Q': function = F_KATA_POST1; break; // ��u�^�̂������ȕϊ� (1 ����)
    case 'R': function = F_KATA_POST2; break; // ��u�^�̂������ȕϊ� (2 ����)
    case 'S': function = F_KATA_POST3; break; // ��u�^�̂������ȕϊ� (3 ����)
    case 'T': function = F_KATA_POST4; break; // ��u�^�̂������ȕϊ� (4 ����)
    case 'U': function = F_KATA_POST5; break; // ��u�^�̂������ȕϊ� (5 ����)
    case 'V': function = F_KATA_POST6; break; // ��u�^�̂������ȕϊ� (6 ����)
    case 'W': function = F_KATA_POST7; break; // ��u�^�̂������ȕϊ� (7 ����)
    case 'X': function = F_KATA_POST8; break; // ��u�^�̂������ȕϊ� (8 ����)
    case 'Y': function = F_KATA_POST9; break; // ��u�^�̂������ȕϊ� (9 ����)
    case ')': function = F_KATA_POSTH1; break; // ��u�^�������ȕϊ� (1 ������)
    case '(': function = F_KATA_POSTH2; break; // ��u�^�������ȕϊ� (2 ������)
    case '\'': function = F_KATA_POSTH3; break; // ��u�^�������ȕϊ� (3 ������)
    case '&': function = F_KATA_POSTH4; break; // ��u�^�������ȕϊ� (4 ������)
    case '%': function = F_KATA_POSTH5; break; // ��u�^�������ȕϊ� (5 ������)
    case '$': function = F_KATA_POSTH6; break; // ��u�^�������ȕϊ� (6 ������)
    case '@': function = F_KATA_POSTS1; break; // ��u�^�������ȕϊ��k (1 ����)
    case '[': function = F_KATA_POSTS2; break; // ��u�^�������ȕϊ��k (2 ����)
    case ';': function = F_KATA_POSTS3; break; // ��u�^�������ȕϊ��k (3 ����)
    case ':': function = F_KATA_POSTS4; break; // ��u�^�������ȕϊ��k (4 ����)
    case ']': function = F_KATA_POSTS5; break; // ��u�^�������ȕϊ��k (5 ����)

    case 'h':                   // �����w���v (�O�̕���)
        function = F_HELP_BACKW; break;
    case 'H':                   // �����w���v (���̕���)
        function = F_HELP_FORW; break;

    case 'v':                   // ���Ō��̃L�[
        function = F_VERB_FIRST; break;
    case '^':                   // ����ł������̃L�[
        function = F_VERB_THIS; break;

    default:                    // ���̑� (�������̂���)
        function = 0; break;
    }
}

char *SpecialBlock::getFace() {
    switch (function) {
    case F_SWITCH_MODE: return "��";
    case F_HIRAKATA:    return "�A";
    case F_HANZEN:      return "�S";
    case F_PUNCT:       return "��";
    case F_MAZE2GG:     return "�K";
    case F_SHOWWIN:     return "��";

    case F_BUSHU_PRE:   return "��";
    case F_MAZE_PRE:    return "��";
    case F_HIST:        return "��";

    case F_QUIT:        return "�~";

    case F_BUSHU_POST:  return "��";
    case F_MAZE_POST1:  return "��";
    case F_MAZE_POST2:  return "��";
    case F_MAZE_POST3:  return "��";
    case F_MAZE_POST4:  return "��";
    case F_MAZE_POST5:  return "��";
    case F_MAZE_POST6:  return "��";
    case F_MAZE_POST7:  return "��";
    case F_MAZE_POST8:  return "��";
    case F_MAZE_POST9:  return "��";
    case F_DAKUTEN:     return "�J";
    case F_HANDAKUTEN:  return "�K";
    case F_KATA_POST1:  return "�J";
    case F_KATA_POST2:  return "�J";
    case F_KATA_POST3:  return "�J";
    case F_KATA_POST4:  return "�J";
    case F_KATA_POST5:  return "�J";
    case F_KATA_POST6:  return "�J";
    case F_KATA_POST7:  return "�J";
    case F_KATA_POST8:  return "�J";
    case F_KATA_POST9:  return "�J";
    case F_KATA_POSTH1:  return "��";
    case F_KATA_POSTH2:  return "��";
    case F_KATA_POSTH3:  return "��";
    case F_KATA_POSTH4:  return "��";
    case F_KATA_POSTH5:  return "��";
    case F_KATA_POSTH6:  return "��";
    case F_KATA_POSTS1:  return "��";
    case F_KATA_POSTS2:  return "��";
    case F_KATA_POSTS3:  return "��";
    case F_KATA_POSTS4:  return "��";
    case F_KATA_POSTS5:  return "��";

    case F_HELP_BACKW:  return "��";
    case F_HELP_FORW:   return "��";

    case F_VERB_FIRST:  return "�E";
    case F_VERB_THIS:   return "�E";

    default:            return "��";
    }
}

// -------------------------------------------------------------------

void *StringBlock::accept(BlockVisitor *v) {
    return v->visitStringBlock(this);
}

void *ControlBlock::accept(BlockVisitor *v) {
    return v->visitControlBlock(this);
}

void *SpecialBlock::accept(BlockVisitor *v) {
    return v->visitSpecialBlock(this);
}

// -------------------------------------------------------------------
// EOF
