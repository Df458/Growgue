#ifndef DRAW_H
#define DRAW_H
#include <curses.h>

enum alignment
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

void draw_text_aligned(WINDOW* win, int row, const char* text, int align);

#endif
