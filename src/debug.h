#ifndef DEBUG_H
#define DEBUG_H

// -------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

void debug(char *, ...);
void debug(HWND, char *, ...);

// -------------------------------------------------------------------
#endif // DEBUG_H
