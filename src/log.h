#ifndef LOG_H
#define LOG_H
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

void init_log(WINDOW* log);
void cleanup_log();
void add_message(int color, const char* message);
void printf_message(int color, const char* message, ...);
void draw_log();
void log_scroll(bool up);
bool ask_question(int color, const char* message);

#endif
