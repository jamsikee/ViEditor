#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <ncurses.h>
#include <signal.h>

#define CONTROL(k) ((k) & 0x1f)

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

struct LineRow {
    char *chars;
    int size;
    struct LineRow *next;
    int row_position;
};


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

struct LineRow *InsertRow(struct LineRow *row, const char *chars, int size, int position) {
    struct LineRow *new_row = malloc(sizeof(struct LineRow));
    new_row->chars = malloc(size + 1); // NULL 문자 고려
    strncpy(new_row->chars, chars, size);
    new_row->chars[size] = '\0';
    new_row->size = size;
    new_row->row_position = position;
    new_row->next = NULL;

    if (!row) {
        return new_row;
    }

    struct LineRow *current = row;
    while (current->next) {
        current = current->next;
    }

    current->next = new_row;
    return row;
}

void UpdateRowPositions(struct LineRow *row) {
    int position = 0;
    struct LineRow *current = row;
    while (current) {
        current->row_position = position;
        position++;
        current = current->next;
    }
}

void editorDrawRows(struct LineRow *row) {
    int y = 0;
    struct LineRow *current = row;
    while (current) {
        if (y >= C.rows) {
            break;
        }
        mvprintw(y, 0, "%s", current->chars);
        current = current->next;
        y++;
    }
}

void Refresh(struct LineRow *row){
    editorDrawRows(row);
    move(C.y, C.x);
    refresh();
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
            freeRow(*row);
            endwin();
            exit(0);
            break;
        case KEY_LEFT: // 왼쪽 화살표 키
        case KEY_RIGHT: // 오른쪽 화살표 키
        case KEY_UP: // 위쪽 화살표 키
        case KEY_DOWN: // 아래쪽 화살표 키
            Move(c);
            break;
        case KEY_END: // End 키
            C.x = C.cols - 1;
            break;
        case KEY_HOME: // Home 키
            C.x = 0;
            break;
        case '\n': // Enter 키
            row = InsertRow(row, "", 0, C.y + 1);
            Move(down);
            C.x = 0;
            UpdateRowPositions(row);
            break;
        case KEY_NPAGE: // Page Down 키
        case KEY_PPAGE: // Page Up 키
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
        Refresh(row);
    }
    return 0;
}