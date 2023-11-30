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

#define CAPACITY 10
#define CONTROL(k) ((k) & 0x1f)

struct termios orig_termios;

enum P_key {
    left = 1000,
    right,
    up,
    down
};

typedef struct{
    size_t length;
    char *string;
} Row;

typedef struct{
  Row *rows;
  int capacity;
  int len;
} Editor;

typedef struct {
    int x, y;
    int rows;
    int cols;
    int totalrows;
} Cursor;

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
        mvprintw(row_count, 0, current->string);
        current = current->next;
        row_count++;
    }

    move(C.y, C.x);
    refresh();
}

void Init(Editor *editor){
    editor->rows = malloc(sizeof(Row) * CAPACITY);
    editor->capacity = CAPACITY;
    editor->len = 0;
}

void Free(Editor *editor){
    for (int i = 0; i < editor->len, ++i){
      free(editor->rows[i].string);
    }
    free(editor->rows);
}

void Edit_Insert_row(Editor *editor, int pos, char *line, ssize_t line_len) {

    if (pos < 0 || pos > editor->len) return;

    if (editor->len >= editor->capacity) {
        editor->capacity *= 2;
        editor->rows = realloc(editor->rows, sizeof(Row) * editor->capacity);
    }

    memmove(&editor->rows[pos + 1], &editor->rows[pos], sizeof(Row) * (editor->len - pos));

    editor->rows[pos].string = malloc(line_len + 1);
    strncpy(editor->rows[pos].string, line, line_len);
    editor->rows[pos].string[line_len] = '\0';
    editor->rows[pos].length = line_len;

    editor->len++;

}

void Edit_Del_row(Editor *editor, int pos){

    if (pos < 0 || pos >= editor->len) return;

     free(editor->rows[pos].string);

     memmove(&editor->rows[pos], &editor->rows[pos + 1], sizeof(Row) * (editor->len - pos - 1));

     editor->len--;

}

void Edit_Del_Char_row(Editor *editor, int pos_x, int pos_y) {

    if (pos_y < 0 || pos_y >= editor->len || pos_x < 0 || pos_x >= editor->rows[pos_y].length) return;

    memmove(&editor->rows[pos_y].string[pos_x], &editor->rows[pos_y].string[pos_x + 1], editor->rows[pos_y].length - pos_x);

    editor->rows[pos_y].length--;

}

void Edit_Insert_Char_row(Editor *editor, int pos_x, int pos_y, char str) {

    if (pos_y < 0 || pos_y >= editor->len || pos_x < 0 || pos_x > editor->rows[pos_y].length) return;

    editor->rows[pos_y].string = realloc(editor->rows[pos_y].string, (editor->rows[pos_y].length + 2) * sizeof(char));

    memmove(&editor->rows[pos_y].string[pos_x + 1], &editor->rows[pos_y].string[pos_x], editor->rows[pos_y].length - pos_x + 1);

    editor->rows[pos_y].string[pos_x] = str;
    editor->rows[pos_y].length++;

}

void Insert_Char(Editor *editor, int str){

    if (C.x == C.totalrows) 
      Edit_Insert_row(editor, C.totalrows, "", 0);
    else 
      Edit_Insert_Char_row(editor, C.x, C.y, str);
    C.x += 1;

}

void New_Line_Beginning(Editor *editor, int pos_y){

    Edit_Insert_row(editor, pos_y, "", 0);

}

void New_Line_Mid(Editor *editor, int pos_x, int pos_y){

    char* temp = strdup(&editor->rows[pos_y].string[pos_x]);
    editor->rows[pos_y].string[pos_x] = '\0';
    editor->rows[pos_y].length = pos_x;
    Edit_Insert_row(editor, pos_y + 1, temp, strlen(temp));
    free(temp);

}

void New_Line_End(Editor *editor, int pos_y){

    Edit_Insert_row(editor, pos_y + 1, "", 0);

}

void Insert_New_Line(Editor *editor, int pos_x, int pos_y){

    if (pos_x == 0)
        New_Line_Beginning(editor, pos_y);
    else if (pos_x == editor->rows[pos_y].length)
        New_Line_End(editor, pos_y);
    else
        New_Line_Mid(editor, pos_x, pos_y);

    C.x = 0;
    C.y += 1;

}

void Delete_Char_Beginning(Editor *editor, int pos_y){

    if (pos_y > 0) {
        editor_row *row = &editor->rows[pos_y];
        editor_row *prev_row = &editor->rows[pos_y - 1];
        prev_row->chars = realloc(prev_row->chars, prev_row->length + row->length + 1);
        memcpy(&prev_row->chars[prev_row->length], row->chars, row->length + 1);
        prev_row->length += row->length;
        Edit_Del_row(editor, pos_y);
    }

}

void Delete_Char_Middle(Editor *editor, int pos_x, int pos_y){

    Edit_Del_Char_row(editor, pos_x, pos_y);

}

void Delete_Char(Editor *editor, int pos_x, int pos_y){

    if (pos_x == 0) {
        Delete_Char_Beginning(editor, pos_y);
        C.y -= 1;
    }
    else {
        Delete_Char_Middle(editor, pos_x, pos_y);
        C.x -= 1;
    }

}

void Move(int key) {
    
    Row *row = &Editor.rows[C.y];

    switch (key) {
        case left:
            if (C.x != 0) {
                C.x--;
            } else if (C.y > 0) {
                C.y--;
                Row *prev_row = C.rows;
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


void presskey() {

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
            Insert_New_Line(&Editor, C.x, C.y);
            break;

        case KEY_DC:
            Move(right);
            Delete_Char(&Editor, C.x, C.y);
            break;

        case KEY_BACKSPACE:
            Delete_Char(&Editor, C.x, C.y);
            break;

        default:
            Insert_Char(&Editor, c);
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
        presskey();
        editorDrawRows(row);
    }
    return 0;

}