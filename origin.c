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
    left = 1000,
    right,
    up,
    down
};

typedef struct Row {
    int index;
    char *chars;
    int size;
} Row;

Row *editorRows;
int totalRows;

struct Cursor {
    int x, y;
    int rows;
    int cols;
    int totalrows;
    Row *row;
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


void editorDrawRows(struct Row *row) {
    int y;
    clear();
    for (y = 0; y < C.rows; y++) {
        mvprintw(y, 0, "~");
    }

    if (C.rows / 3 >= 0 && C.rows / 3 < C.rows && C.totalrows == 0) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome), "Visual Text editor -- version 0.0.1");
        if (welcomelen > C.cols) welcomelen = C.cols;
        int padding = (C.cols - welcomelen) / 2;
        mvprintw(C.rows / 3, padding > 0 ? padding : 0, "%s", welcome);
    }

    struct Row *current = row;
    int row_count = 0;
    while (current != NULL && row_count < C.rows) {
        mvprintw(row_count, 0, current->chars);
        current = current->next;
        row_count++;
    }

    move(C.y, C.x);
    refresh();
}

void editorInsertRow(int at, char *s, size_t len) {
  if (at < 0 || at > C.totalrows) return;

  Row *new_row = (Row*)malloc(sizeof(Row));
  new_row->index = at;
  new_row->size = len;
  new_row->chars = malloc(len + 1);
  memcpy(new_row->chars, s, len);
  new_row->chars[len] = '\0';
  new_row->next = NULL;

  if (C.row == NULL) {
    C.row = new_row;
  } else {
    Row *current = C.row;
    for (int i = 0; i < at - 1; i++) {
      current = current->next;
    }
    if (current->next != NULL) {
      new_row->next = current->next;
      current->next = new_row;
    } else {
      current->next = new_row;
    }
  }

  C.totalrows++;
}


void editorFreRow(Row *row) {
  free(row->chars);
  free(row);
}

void editorDelRow(int at) {
  if (at < 0 || at >= C.totalrows) return;
  Row *current = C.row;
  Row *prev = NULL;
  for (int i = 0; i < at; i++) {
    prev = current;
    current = current->next;
  }
  if (prev == NULL) {
    C.row = current->next;
  } else {
    prev->next = current->next;
  }
  editorFreRow(current);
  C.totalrows--;
}

void editorRowInsertChar(Row *row, int at, int c) {
  if (at < 0 || at > row->size) at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
}

void editorRowAppendString(Row *row, char *s, size_t len) {
  row->chars = realloc(row->chars, row->size + len + 1);
  memcpy(&row->chars[row->size], s, len);
  row->size += len;
  row->chars[row->size] = '\0';
}

void editorRowDelChar(Row *row, int at) {
  if (at < 0 || at >= row->size) return;
  memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
  row->size--;
}

void editorInsertChar(int c) {
  if (C.y == C.totalrows) {
    editorInsertRow(C.totalrows, "", 0);
  }
  editorRowInsertChar(&C.row[C.y], C.x, c);
  C.x++;
}

void editorInsertNewline() {
  if (C.x == 0) {
    editorInsertRow(C.y, "", 0);
  } else {
    Row *row = &C.row[C.y];
    editorInsertRow(C.y + 1, &row->chars[C.x], row->size - C.x);
    row = &C.row[C.y];
    row->size = C.x;
    row->chars[row->size] = '\0';
  }
  C.y++;
  C.x = 0;
}

void editorDelChar() {
  if (C.y == C.totalrows) return;
  if (C.x == 0 && C.y == 0) return;

  Row *row = &C.row[C.y];
  if (C.x > 0) {
    editorRowDelChar(row, C.x - 1);
    C.x--;
  } else {
    C.x = C.row[C.y - 1].size;
    editorRowAppendString(&C.row[C.y - 1], row->chars, row->size);
    editorDelRow(C.y);
    C.y--;
  }
}


void Move(int key) {
    
    Row *row = C.row;
    for (int i = 0; i < C.y; ++i) {
        row = row->next;
    }

    switch (key) {
        case left:
            if (C.x != 0) {
                C.x--;
            } else if (C.y > 0) {
                C.y--;
                Row *prev_row = C.row;
                for (int i = 0; i < C.y - 1; ++i) {
                    prev_row = prev_row->next;
                }
                C.x = prev_row->size;
            }
            break;
        case right:
            if (row && C.x < row->size) {
                C.x++;
            } else if (row && C.x == row->size) {
                C.y++;
                C.x = 0;
            }
            break;
        case up:
            if (C.y != 0) {
                C.y--;
            }
            break;
        case down:
            if (C.y < C.totalrows) {
                C.y++;
            }
            break;
    }
}



void presskey(struct Row **row) {
    int c = getch();

    switch (c) {
        case CONTROL('q'):
            endwin();
            exit(0);
            break;

        case CONTROL('s'):
            break;

        case CONTROL('f'):
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

        case KEY_ENTER:
        case '\n':
            editorInsertNewline();
            break;

        case KEY_DC:
            Move(right);
            editorDelChar();
            break;

        case KEY_BACKSPACE:
            editorDelChar();
            break;

        default:
            editorInsertChar(c);
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
    C.totalrows = 0;
}

int main(int argc, char *argv[]) {

    struct Row *row = NULL;
    init();
    editorDrawRows(row);

    while (1) {
        presskey(&row);
        editorDrawRows(row);
    }
    return 0;
}