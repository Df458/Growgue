#ifndef DRAW_H
#define DRAW_H
#ifdef PDCURSES
#ifdef Linux
#include <xcurses/curses.h>
#include <xcurses/panel.h>
#else
#include <curses.h>
#include <panel.h>
#endif
#else
#include <curses.h>
#include <panel.h>
#endif

enum alignment
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

void draw_text_aligned(WINDOW* win, int row, const char* text, int align);

#endif
