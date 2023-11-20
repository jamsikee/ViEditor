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

struct editorRow {
    char *chars;
    int size;
    struct editorRow *next;
};

struct gapbuf {
    char *buf;
    int size;
    int gap_start;
    int gap_end;
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

void gapbufInit(struct gapbuf *gb, int size) {
    gb->buf = malloc(size);
    gb->size = size;
    gb->gap_start = 0;
    gb->gap_end = size - 1;
}

void gapbufFree(struct gapbuf *gb) {
    free(gb->buf);
}

void gapbufAppend(struct gapbuf *gb, const char *s, int len) {
    // Gap이 충분하지 않으면 확장해야 함
    if (len > (gb->size - (gb->gap_end - gb->gap_start + 1))) {
        int new_size = gb->size + len;
        char *new_buf = realloc(gb->buf, new_size);
        if (new_buf == NULL) {
            return; // 실패 시 처리
        }

        // Gap을 옮기고 크기 조정
        memmove(&new_buf[gb->gap_end + len + 1], &new_buf[gb->gap_end + 1], gb->size - gb->gap_end - 1);
        gb->buf = new_buf;
        gb->size = new_size;
        gb->gap_end += len;
    }

    // Gap에 텍스트 복사
    memcpy(&gb->buf[gb->gap_start], s, len);
    gb->gap_start += len;
}

void editorDrawRows(struct gapbuf *gb) {
    int y;
    for (y = 0; y < C.rows; y++) {
        if (y == C.rows / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome), "Visual Text editor -- version 0.0.1");
            if (welcomelen > C.cols) welcomelen = C.cols;
            int padding = (C.cols - welcomelen) / 2;
            if (padding) {
                gapbufAppend(gb, "~", 1);
                padding--;
            }
            while (padding--) gapbufAppend(gb, " ", 1);
            gapbufAppend(gb, welcome, welcomelen);
        } else {
            gapbufAppend(gb, "~", 1);
        }
        gapbufAppend(gb, "\x1b[K", 3);
        if (y < C.rows - 1) {
            gapbufAppend(gb, "\r\n", 2);
        }
    }
    gapbufAppend(gb, "", 0);
    move(C.x, C.y); 
    printw(gb->buf);
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

void presskey(struct editorRow **row) {
    int c = getch();

    switch (c) {
        case CONTROL('q'):
            freeRow(*row);
            endwin();
            exit(0);
            break;
        case KEY_LEFT: // Left Arrow
        case KEY_RIGHT: // Right Arrow
        case KEY_UP: // Up arrow
        case KEY_DOWN: // Down arrow
            Move(c);
            break;
        case KEY_END: // End 
            C.x = C.cols - 1;
            break;
        case KEY_HOME: // Home 
            C.x = 0;
            break;
        case KEY_NPAGE: // Page Down 
        case KEY_PPAGE: // Page Up 
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

void editorOpen(char *filename, struct editorRow **row) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) {
            linelen--;
        }

        struct editorRow *newRow = malloc(sizeof(struct editorRow));
        if (!newRow) {
            fclose(fp);
            free(line);
            exit(1);
        }

        newRow->chars = malloc(linelen + 1);
        if (!newRow->chars) {
            fclose(fp);
            free(line);
            free(newRow);
            exit(1);
        }

        memcpy(newRow->chars, line, linelen);
        newRow->chars[linelen] = '\0';
        newRow->size = linelen;
        newRow->next = NULL;

        gapbufAppend(row, newRow->chars, newRow->size);

        C.currentrows++;
    }

    free(line);
    fclose(fp);
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

    struct editorRow *row = NULL;
    init();
    editorDrawRows(row);
    if (argc >= 2) {
    editorOpen(argv[1], &row);
    }

    while (1) {
        presskey(&row);
        editorDrawRows(row);
    }
    return 0;
}
