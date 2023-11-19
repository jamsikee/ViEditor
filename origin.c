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

void Append(struct editorRow **row, const char *s, int len) {
    struct editorRow *newRow = malloc(sizeof(struct editorRow));
    if (newRow == NULL) return;

    newRow->chars = malloc(len + 1);
    if (newRow->chars == NULL) {
        free(newRow);
        return;
    }

    memcpy(newRow->chars, s, len);
    newRow->chars[len] = '\0';
    newRow->size = len;
    newRow->next = NULL;

    if (*row == NULL) { // 비어 있으면 현재 행을 첫번쨰 행으로 인지
        *row = newRow;
        return;
    }

    struct editorRow *current = *row;
    while (current->next != NULL) {  // 마지막 노드까지 찾아가기
        current = current->next;
    }
    current->next = newRow; // 마지막 노드의 다음을 새로운 노드로 인지
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

void freeRow(struct editorRow *row) {
    while (row) {
        struct editorRow *temp = row->next;
        free(row->chars);
        free(row);
        row = temp;
    }
}

void editorDrawRows(struct editorRow *ab) {
    int y, rows, cols;
    getmaxyx(stdscr, rows, cols);
    rows -= 2;

    if (E.numrows == 0) {
        for (y = 0; y < rows; y++) {
            if (y == rows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                                          "Visual Text editor -- version %s", KILO_VERSION);
                if (welcomelen > cols) welcomelen = cols;
                int padding = (cols - welcomelen) / 2;
                if (padding > 0) {
                    Append(&ab, "~", 1);
                    padding--;
                }
                while (padding--) Append(&ab, " ", 1);
                Append(&ab, welcome, welcomelen);
                break;
            } else {
                Append(&ab, "~", 1);
            }
        }
    }

    for (y = 0; y < rows; y++) {
        if (ab && y == rows / 3) {
            int text_length = strlen(ab->chars);
            int x = (cols - text_length) / 2;
            mvprintw(y, x > 0 ? x : 0, "%s", ab->chars);
            ab = ab->next;
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
        freeRow(*row);
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
    editorRefreshScreen(row);
    
    while (1) {
        presskey(&row);
    }
    return 0;
}
