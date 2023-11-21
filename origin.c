#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <signal.h>

#define CONTROL(k) ((k) & 0x1f)
#define BUFF_INIT {NULL, 0}

struct termios orig_termios;

enum P_key {
    B_space = 127,
    left = 1000,
    right,
    up,
    down,
    Del,
    End,
    Home,
    PgUp,
    PgDn
};

struct Cursor {
    int x, y;
    int rows;
    int cols;
    int currentrows;
};

struct Cursor C;

void disRaw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void Raw() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    tcgetattr(STDIN_FILENO, &raw);

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    atexit(disRaw);
}

struct buffer {
    char *buf;
    int len;
};

void abAppend(struct buffer *buff, const char *string, int len) {
    char *new = malloc(buff->len + len + 1);

    if (new == NULL) return;

    if (buff->buf != NULL) {
        memcpy(new, buff->buf, buff->len);
        free(buff->buf);
    }

    memcpy(&new[buff->len], string, len); // ab -> buff로 수정
    new[buff->len + len] = '\0';
    buff->buf = new;
    buff->len += len;
}

void abFree(struct buffer *buff) {
    free(buff->buf);
    buff->buf = NULL;
    buff->len = 0;
}

void editorDrawRows() {
    int y;
    for (y = 0; y < LINES; y++) {
        if (y == LINES / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
                                      "Kilo editor -- version %s", KILO_VERSION);
            if (welcomelen > COLS) welcomelen = COLS;
            int padding = (COLS - welcomelen) / 2;
            if (padding) {
                mvprintw(y, 0, "%*s~", padding - 1, "");
                padding--;
            }
            mvprintw(y, padding, "%s", welcome);
        } else {
            mvprintw(y, 0, "~");
        }
        clrtoeol(); // Clear to the end of line
    }
}

void editorRefreshScreen() {
    initscr(); // Initialize ncurses mode
    clear(); // Clear the screen

    editorDrawRows(); // Draw the rows

    move(C.y, C.x); // Move the cursor
    refresh(); // Refresh the screen
    getch(); // Wait for a key press

    endwin(); // End ncurses mode
}

void Move(int key) {
    switch (key) {
        case left:
            if (C.x != 0) {
                C.x--;
            }
            break;
        case right:
            if (C.x != C.cols - 1) {
                C.x++;
            }
            break;
        case up:
            if (C.y != 0) {
                C.y--;
            }
            break;
        case down:
            if (C.y != C.rows - 1) {
                C.y++;
            }
            break;
    }
}

void presskey(struct LineRow **row) {
    int c = getch();

    switch (c) {
        case CONTROL('q'):
            endwin();
            disRaw();
            exit(1);
            break;
        case KEY_LEFT: // Left arrow key
        case KEY_RIGHT: // Right arrow key
        case KEY_UP: // Up arrow key
        case KEY_DOWN: // Down arrow key
            Move(c);
            break;
        case KEY_END: // End key
            C.x = C.cols - 1;
            break;
        case KEY_HOME: // Home key
            C.x = 0;
            break;
        case '\n': // Enter key
            // Add your logic for handling Enter key here
            break;
        case KEY_NPAGE: // Page Down key
        case KEY_PPAGE: // Page Up key
        {
            int temprows = C.rows;
            while (temprows--) {
                if (c == KEY_PPAGE)
                    Move(up);
                else if (c == KEY_NPAGE)
                    Move(down);
            }
        }
            break;
    }
}

void init() {
    Raw();
    initscr();
    getmaxyx(stdscr, C.rows, C.cols);
    keypad(stdscr, TRUE);
    C.x = 0;
    C.y = 0;
    C.currentrows = 0;
}

int main(int argc, char *argv[]) {
    struct LineRow *row = NULL;
    init();
    editorDrawRows(row);
    if (argc >= 2) {
        editorOpen(argv[1], &row);
    }

    while (1) {
        presskey(&row);
        editorRefreshScreen();
    }
    return 0;
}
