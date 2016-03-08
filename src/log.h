#ifndef LOG_H
#define LOG_H
#include <curses.h>

void init_log(WINDOW* log);
void add_message(int color, const char* message);
void draw_log();
void log_scroll(bool up);
bool ask_question(int color, const char* message);

#endif
