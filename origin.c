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
#define CONTROL(k) ((k) & 0x1f)  // 문자의 아스키코드 지정 ctrl-A = 1
struct termios orig_termios;

enum P_key {
    B_space = 10000,
    left,
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
    int cx, cy;
    int rows;
    int cols;
    int currentrows;
};

struct editorRow {
    char *chars;
    int size;
    struct editorRow *next;
};

void disRaw() {  // raw mode 기능 해제
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void Raw() {  // raw mode 기능 켜기
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

struct editorRow *Append(struct editorRow *row, const char *s, int len) {
    row = malloc(sizeof(struct editorRow));
    row->chars = malloc(len + 1);
    memcpy(row->chars, s, len);
    row->chars[len] = '\0';
    row->size = len;
    row->next = NULL;
    return row;
}

struct editorRow *Insert(struct editorRow *row, const char *s, int len) {
    struct editorRow *newRow = malloc(sizeof(struct editorRow));
    newRow->chars = malloc(len + 1);
    memcpy(newRow->chars, s, len);
    newRow->chars[len] = '\0';
    newRow->size = len;
    newRow->next = row;
    return newRow;
}

void free(struct editorRow *row) {
    while (row) {
        struct editorRow *temp = row->next;
        free(row->chars);
        free(row);
        row = temp;
    }
}

void editorDrawRows(struct editorRow *row) {
    int y, rows, cols;
    getmaxyx(stdscr, rows, cols);
    rows -= 2;

     for (y = 0; y < rows; y++) {
        if (row && y == rows / 3) {
            int text_length = strlen(row->chars);
            int x = (cols - text_length) / 2;
            mvprintw(y, x > 0 ? x : 0, "%s", row->chars);
            row = row->next;
        } else {
            mvprintw(y, 0, "~");
        }
    }
    refresh();
}

void editorRefreshScreen(struct editorRow *row) {
    clear();
    editorDrawRows(row);
    move(0, 0);
    refresh();
}

void presskey(struct editorRow **row) {
    int c = getch();

    switch (c) {
    case CONTROL('q'):
        free(*row);
        endwin();
        exit(0); // 프로그램 종료
        break;
        /*
    case CONTROL('s'):
        break;
    case CONTROL('f'):
        break;    
    case Home:
        break;
    case B_space:
        break;
    case Del:
        break;
    case left:
        break;
    case right:
        break;
    case up:
        break;
    case down:
        break;
    case End:
        break;
    case Home:
        break;
    case PgUp:
        break;
    case PgDn:
        break;
    default:
        break;
        */
    }
}

int main() {
    Raw();
    initscr();
    struct editorRow *row = NULL;
    row = Append(row, "visual text editor -- version 0.0.1", strlen("visual text editor -- version 0.0.1"));
    editorRefreshScreen(row);
    
    while (1) {
        presskey(&row);
    }
    return 0;
}
