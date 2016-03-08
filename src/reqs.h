#ifndef REQS_H
#define REGQS_H
#include <curses.h>

#define MIN_W 90
#define MIN_H 35
#define MIN_COLS   16
#define BEST_COLS  256
#define MIN_PAIRS  16
#define BEST_PAIRS 256

// Awful hack. For some reason, PDCurses always returns 1x1 from getmaxyx, so
// this is the best we can do :/
#if defined PDCURSES
#define max_size(win, y, x) x = MIN_W; y = MIN_H
#else
#define max_size(win, y, x) getmaxyx(win, y, x)
#endif

#endif
