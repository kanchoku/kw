#include "debug.h"

void debug(char *fmt, ...) {
    FILE *f;
    va_list ap;
    va_start(ap, fmt);
    f = fopen("debug.txt", "a");
    vfprintf(f, fmt, ap);
    fclose(f);
    va_end(ap);
}

void debug(HWND hwnd, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char buf[256];
    vsprintf(buf, fmt, ap);
    MessageBox(hwnd, buf, "debug", MB_OK);
    va_end(ap);
}
