//<v127a - gg>
#ifndef GG_H
#define GG_H
// -------------------------------------------------------------------

#include <iostream.h>
#include <fstream.h>
#include <windows.h>
//<gg-defg2>
#include <stdlib.h>
//</gg-defg2>

#include "moji.h"

#define GGDIC_MAXENT 8000

class GgDic {
public:
    //<gg-defg>
    //MOJI *ent[GGDIC_MAXENT];
    char *ent[GGDIC_MAXENT][2];
    //<gg-defg2>
    char *ent4del[GGDIC_MAXENT];
    int index[256];
    //</gg-defg2>
    //</gg-defg>
    int nent;

    GgDic();
    ~GgDic();

    void readFile(ifstream *);
    //<gg-defg>
    //MOJI *look(MojiBuffer *);
    char *look(MojiBuffer *);
    //</gg-defg>
};

// -------------------------------------------------------------------
#endif // GG_H
//</v127a - gg>