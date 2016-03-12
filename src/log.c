#define true TRUE
#define false FALSE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "color.h"
#include "log.h"
#include "reqs.h"

WINDOW* log_win = 0;

#define LOG_WIDTH MIN_W - 2
#define LOG_SIZE 256

static char log_data[LOG_SIZE][LOG_WIDTH + 1] = { 0 };
static int log_colors[LOG_SIZE] = { 0 };
static int log_start = 0;
static int log_end = 0;
static int message_count = 0;
static int scrollback_pos = 0;
static bool dirty = true;

void init_log(WINDOW* log)
{
    log_win = log;
    wrefresh(log_win);
}

void cleanup_log()
{
    log_win = 0;
    log_start = 0;
    log_end = 0;
    message_count = 0;
    scrollback_pos = 0;
    dirty = true;
}

static void _add_message(int color, char* message)
{
    dirty = true;
    if(message_count < 256)
        ++message_count;
    log_end++;
    if(log_end >= LOG_SIZE - 1) {
        log_end = log_start;
        log_start++;
        if(log_start >= LOG_SIZE - 1) {
            log_start = 0;
        }
    }
    strcpy(log_data[log_end], message);
    log_colors[log_end] = color;
    free(message);
}

void add_message(int color, const char* message)
{
    scrollback_pos = 0;
    ssize_t mlen = strlen(message);
    ssize_t len = 0;
    while(mlen - len > LOG_WIDTH) {
        int i = LOG_WIDTH;
        int offset = 0;
        for(i = LOG_WIDTH; i >= 0; --i)
            if(message[len + i] == ' ')
                break;
        if(i <= 0)
            i = LOG_WIDTH;
        else
            offset = 1;
        _add_message(color, strndup(message + len, i));
        len += i + offset;
    }
    _add_message(color, strdup(message + len));
}

void printf_message(int color, const char* message, ...)
{
	va_list args;
	va_start(args, message);
    int length = vsnprintf(0, 0, message, args);
    va_end(args);
    ++length;
    char* mess = calloc(length, sizeof(char));
	va_start(args, message);
    vsnprintf(mess, length, message, args);
    va_end(args);
    add_message(color, mess);
    free(mess);
}

void draw_log()
{
    if(!dirty)
        return;
    wclear(log_win);
    wborder(log_win, 179, 179, 205, 196, 198, 181, 192, 217);
    mvwaddch(log_win, 0, 44, 207);
    mvwaddch(log_win, 0, 45, 207);
    int i = log_end;
    int j = 0;
    for(; j < scrollback_pos; ++j) {
        i--;
        if(i < 0)
            i = 255;
    }
    if(scrollback_pos == -1) {
        i = log_end;
    }
    j = 0;
    for(; i != log_start && j < 8; --i) {
        if(i < 0)
            i = LOG_SIZE - 1;
        set_color(log_win, log_colors[i]);
        mvwprintw(log_win, 8 - j, 1, log_data[i]);
        ++j;
    }
    char perc[6];
    int dist = 0;
    if(message_count > 1)
        dist = scrollback_pos * 100 / (message_count - 1);
    snprintf(perc, 6, "%3d%%%%", dist);
    set_color(log_win, COLOR_DEFAULT);
    mvwprintw(log_win, 9, LOG_WIDTH - 3, perc);
    wrefresh(log_win);
}

void log_scroll(bool up)
{
    dirty = true;
    if(up && scrollback_pos < message_count - 1) {
        scrollback_pos++;
    } else if(!up && scrollback_pos != 0) {
        scrollback_pos--;
    }
}

bool ask_question(int color, const char* message)
{
    char* m = calloc(strlen(message) + 8, sizeof(char));
    strcpy(m, message);
    strcat(m, " (y/n)");
    add_message(color, m);
    draw_log();
    free(m);
    while(true) {
        int ret = getch();
        if(ret == 'y') {
            add_message(COLOR_DEFAULT, "y");
            return true;
        } else if(ret == 'n') {
            add_message(COLOR_DEFAULT, "n");
            return false;
        }
    }
}
