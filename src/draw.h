#ifndef DRAW_H
#define DRAW_H
#ifdef PDCURSES
#include <xcurses/curses.h>
#else
// #include <curses.h>
#endif

enum alignment
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

void draw_text_aligned(WINDOW* win, int row, const char* text, int align);

#endif
