#ifndef LOG_H
#define LOG_H
#ifdef PDCURSES
#include <xcurses/curses.h>
#else
#include <curses.h>
#endif

void init_log(WINDOW* log);
void cleanup_log();
void add_message(int color, const char* message);
void printf_message(int color, const char* message, ...);
void draw_log();
void log_scroll(bool up);
bool ask_question(int color, const char* message);

#endif
