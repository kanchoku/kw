// -------------------------------------------------------------------
// class MgTable
// 交ぜ書き辞書を格納するクラス

#ifndef MG_TABLE_H
#define MG_TABLE_H
// -------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <windows.h>

using namespace std;

// 文字列比較関数 (STL のマニュアルまんま)
struct ltstr {
    bool operator()(const char *s1, const char *s2) const {
        return strcmp(s1, s2) < 0;
    }
};

typedef std::map<char *, char *, ltstr> MgMap;

class MgTable {
 public:
    std::vector<char *> *cand;
    HWND hwnd;
    MgMap *mgMap;
    MgTable(HWND);
    ~MgTable();

    void readFile(ifstream *);
    int setCand(char *key);
    int setCand(char *, int);
    int setCand(char *, int, int);
};

// -------------------------------------------------------------------
#endif // MG_TABLE_H


