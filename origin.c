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

    if (*row == NULL) {
        *row = newRow;
        return;
    }

    struct editorRow *current = *row;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newRow;
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

void editorDrawRows(struct editorRow *row) {
    int y;
    clear();
    for (y = 0; y < C.rows; y++) {
        mvprintw(y, 0, "~");
    }

    if (C.rows / 3 >= 0 && C.rows / 3 < C.rows) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome), "Visual Text editor -- version 0.0.1");
        if (welcomelen > C.cols) welcomelen = C.cols;
        int padding = (C.cols - welcomelen) / 2;
        mvprintw(C.rows / 3, padding > 0 ? padding : 0, "%s", welcome);
    }

    struct editorRow *current = row;
    int row_count = 0;
    while (current != NULL && row_count < C.rows) {
        mvprintw(row_count, 0, current->chars);
        current = current->next;
        row_count++;
    }

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

void presskey(struct editorRow **row) {
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
        default:
            attron(A_REVERSE); // 흰색 바탕으로 설정
            move(C.y, C.x);
            addch(' '); // 현재 커서 위치에 공백 문자 출력
            attroff(A_REVERSE); // 흰색 바탕 해제
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

        Append(row, newRow->chars, newRow->size);

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
