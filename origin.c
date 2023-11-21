#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

/*** data ***/

typedef struct erow {
    int size;
    char *chars;
} erow;

/*** Global Variables ***/
int cx = 0, cy = 0;
int screenrows, screencols;
erow row;

/*** File I/O ***/

void editorOpen() {
    char *line = "Hello, world!";
    ssize_t linelen = 13;

    row.size = linelen;
    row.chars = malloc(linelen + 1);
    memcpy(row.chars, line, linelen);
    row.chars[linelen] = '\0';
}

/*** Output ***/

void editorDrawRows() {
    for (int y = 0; y < screenrows; y++) {
        if (y >= 1) {
            if (y == screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                                          "Kilo editor -- version 0.0.1");
                if (welcomelen > screencols) welcomelen = screencols;
                int padding = (screencols - welcomelen) / 2;
                if (padding) {
                    mvaddch(y, 0, '~');
                    padding--;
                }
                while (padding--) mvaddch(y, 0, ' ');
                mvprintw(y, padding, welcome);
            } else {
                mvaddch(y, 0, '~');
            }
        } else {
            int len = row.size;
            if (len > screencols) len = screencols;
            mvprintw(y, 0, "%s", row.chars);
        }

        clrtoeol();
    }
}

void editorRefreshScreen() {
    clear();
    editorDrawRows();
    move(cy, cx);
    refresh();
}

/*** Input ***/

void editorProcessKeypress() {
    int c = getch();

    switch (c) {
        case 'q':
        case 'Q':
            endwin();
            exit(0);
            break;

        case KEY_LEFT:
            if (cx != 0) {
                cx--;
            }
            break;

        case KEY_RIGHT:
            if (cx != screencols - 1) {
                cx++;
            }
            break;

        case KEY_UP:
            if (cy != 0) {
                cy--;
            }
            break;

        case KEY_DOWN:
            if (cy != screenrows - 1) {
                cy++;
            }
            break;
    }
}

/*** Initialization ***/

void initEditor() {
    if (getmaxyx(stdscr, screenrows, screencols) == ERR) {
        endwin();
        fprintf(stderr, "Failed to get screen size!\n");
        exit(1);
    }
    editorOpen();
}

int main() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    endwin();
    return 0;
}
