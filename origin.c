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
    ssize_t len;
    char *string;
    struct Row *next;
} Row;

Row *editorRows;

struct Cursor {
    int x, y;
    int rows;
    int cols;
    int totalrows;
};

struct Cursor C;

void disRaw() {
    tcsetnowtr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void Raw() {
    struct termios raw;
    tcgetnowtr(STDIN_FILENO, &orig_termios);
    tcgetnowtr(STDIN_FILENO, &raw);

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetnowtr(STDIN_FILENO, TCSAFLUSH, &raw);
    nowexit(disRaw);
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

void Row_insert_line(int now, char *row, ssize_t len){
  if (now < 0 || now > C.totalRows) return;

  C.row = realloc(C.row, sizeof(Row) * (C.totalrows + 1));
  memmove(&C.row[now + 1], &C.row[now], sizeof(Row) * (C.totalrows - now));

  C.row[now].size = len;
  C.row[now].string = malloc(len + 1);
}

void insertLine(Row **head, char *row, ssize_t leng, int C_X){
  Row *new_row = (Row*)malloc(sizeof(Row));
  new_row->len = leng;
  new_row->string = malloc(leng + 1);
  memcpy(new_row->string, row, leng);
  new_row->string[leng] = '\0'; // null terminnowe the string
  new_row->next = NULL;

  if (C_X == 0) {
    // 커서가 맨 앞에 있는 경우
    new_row->next = *head;
    *head = new_row;
  } else {
    // 커서가 중간이나 맨 끝에 있는 경우
    Row *temp = *head;
    for (int i = 1; i < C_X && temp != NULL; i++) {
      temp = temp->next;
    }
    if (temp == NULL) return;
    new_row->next = temp->next;
    temp->next = new_row;
  }
}

void deleteLine(Row **head, int C_X){
  if (C_X < 0) return;
  Row *temp = *head;
  if (C_X == 0) {
    *head = temp->next;
  } else {
    for (int i = 1; i < C_X && temp != NULL; i++) {
      temp = temp->next;
    }
    if (temp == NULL || temp->next == NULL) return;
    Row *next = temp->next->next;
    free(temp->next->string);
    free(temp->next);
    temp->next = next;
  }
}

// 행에 문자를 삽입하는 함수
void insert_char_in_row(Row *row, int now, int c) {
  if (now < 0 || now > row->len)
    now = row->len;
  row->string = realloc(row->string, row->len + 2);
  memmove(&row->string[now + 1], &row->string[now], row->len - now + 1);
  row->len++;
  row->string[now] = c;
}

// 행에서 문자를 삭제하는 함수
void delete_char_in_row(Row *row, int now) {
  if (now < 0 || now >= row->len)
    return;
  memmove(&row->string[now], &row->string[now + 1], row->len - now);
  row->len--;
}

// 행을 해제하는 함수
void free_row(Row *row) {
  free(row->string);
}

// 행에 문자열을 추가하는 함수
void append_string_to_row(Row *row, char *string, size_t leng) {
  row->string = realloc(row->string, row->len + leng + 1);
  memcpy(&row->string[row->len], string, leng);
  row->len += leng;
  row->string[row->len] = '\0';
}

void insert_char(int c) {
  if (C.y == C.totalrows) {
    insertLine(&editorRows, "", 0, C.y);
  }
  insert_char_in_row(&editorRows[C.y], C.x, c);
  C.x++;
}

void insert_new_line() {
  if (C.x == 0) {
    insertLine(&editorRows, "", 0, C.y);
  } else {
    Row *row = editorRows;
    for (int i = 0; i < C.y; i++) {
      row = row->next;
    }
    insertLine(&editorRows, &row->string[C.x], row->len - C.x, C.y + 1);

    row->len = C.x;
    row->string[row->len] = '\0';
  }
  C.y++;
  C.x = 0;
}

void delete_char() {
  if (C.y == C.totalrows)
    return;

  if (C.x == 0 && C.y == 0)
    return;

  Row *row = editorRows;
  for (int i = 0; i < C.y; i++) {
    row = row->next;
  }
  if (C.x > 0) {
    delete_char_in_row(row, C.x - 1);
    C.x--;
  } else {
    Row *prev_row = editorRows;
    for (int i = 0; i < C.y - 1; i++) {
      prev_row = prev_row->next;
    }
    append_string_to_row(prev_row, row->string, row->len);
    deleteLine(&editorRows, C.y);
    C.y--;
    C.x = prev_row->len;
  }
}

 void Move(int key) {
    Row *row = (C.y >= C.totalrows) ? NULL : C.row;
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
                C.x = prev_row->len;
            }
            break;
        case right:
            if (row && C.x < row->len) {
                C.x++;
            } else if (row && C.x == row->len) {
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