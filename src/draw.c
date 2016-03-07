#include "draw.h"
#include "reqs.h"
#include <string.h>

void draw_text_aligned(WINDOW* win, int row, const char* text, int align)
{
    int rows, cols;
    max_size(win, rows, cols);

    // If we're out of bounds, or we don't have enough space, return
    if(row > rows || strlen(text) > cols) {
#ifdef DEVEL
        endwin();
        fprintf(stderr, "Can't draw text, out of bounds: Length is %lu, Window width is %d\n", strlen(text), cols);
        getch();
#endif
        return;
    }

    switch(align) {
        case ALIGN_LEFT:
            mvwaddstr(win, row, 0, text);
            break;
        case ALIGN_CENTER:
            mvwaddstr(win, row, (cols / 2) - (strlen(text) / 2), text);
            break;
        case ALIGN_RIGHT:
            mvwaddstr(win, row, cols - strlen(text), text);
            break;
    }
}
