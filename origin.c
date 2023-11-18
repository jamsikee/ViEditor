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

enum P_key{
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

struct Cursor
{
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

struct editorBuffer {
    struct editorRow *head;
    struct editorRow *tail;
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

void abAppend(struct editorBuffer *ab, const char *s, int len) {
    struct editorRow *newRow = malloc(sizeof(struct editorRow));
    newRow->chars = malloc(len + 1);
    memcpy(newRow->chars, s, len);
    newRow->chars[len] = '\0';
    newRow->size = len;
    newRow->next = NULL;

    if (ab->tail == NULL) {
        ab->head = ab->tail = newRow;
    } else {
        ab->tail->next = newRow;
        ab->tail = newRow;
    }
}

void editorDrawRows() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    rows -= 2;

    for (int y = 0; y < rows; y++) {
        move(y, 0);
        if (y == rows / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
                "Kilo editor -- version %s", "0.0.1"); // 여기에 버전을 넣으세요
            if (welcomelen > cols) welcomelen = cols;
            editorBufferAppendRow(welcome, welcomelen);
            printw("%.*s", welcomelen, welcome);
        } else {
            editorBufferAppendRow("~", 1);
            printw("~");
        }
    }
    refresh();
}

void editorRefreshScreen() {
    clear();
    editorDrawRows();
}

void presskey(){
    int c = getch();

    switch (c)
    {
    case CONTROL('q'):
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

int main(){
    Raw();
    initscr();
    editorRefreshScreen();
    while (1) {
    presskey();
    }
  return 0;
}