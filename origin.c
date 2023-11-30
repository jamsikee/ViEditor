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
    int length;
    char *string;
} Row;

typedef struct{
  Row *rows;
  int capacity;
  int len;
} Editor;

int x = 0;
int y = 0;
int rows = 0;
int cols = 0;
int totalrows = 0;

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


void editorDrawRows() {
    int y;
    clear();
    for (y = 0; y < rows; y++) {
        mvprintw(y, 0, "~");
    }

    if (rows / 3 >= 0 && rows / 3 < rows && totalrows == 0) {
        char welcome[80];
        int welcomelen = snprintf(welcome, sizeof(welcome), "Visual Text editor -- version 0.0.1");
        if (welcomelen > cols) welcomelen = cols;
        int padding = (cols - welcomelen) / 2;
        mvprintw(rows / 3, padding > 0 ? padding : 0, "%s", welcome);
    }

    refresh();
}

void Free(Editor *editor){
    for (int i = 0; i < editor->len; ++i){
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

    if (x == totalrows) 
      Edit_Insert_row(editor, totalrows, "", 0);
    else 
      Edit_Insert_Char_row(editor, x, y, str);
    x += 1;

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

    x = 0;
    y += 1;

}

void Delete_Char_Beginning(Editor *editor, int pos_y){

    if (pos_y > 0) {
        Row *row = &editor->rows[pos_y];
        Row *prev_row = &editor->rows[pos_y - 1];
        prev_row->string = realloc(prev_row->string, prev_row->length + row->length + 1);
        memcpy(&prev_row->string[prev_row->length], row->string, row->length + 1);
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
        y -= 1;
    }
    else {
        Delete_Char_Middle(editor, pos_x, pos_y);
        x -= 1;
    }

}

void Move(int key) {
    Editor *editor;
    Row *row = &(editor->rows[y]);

    switch (key) {
        case left:
            if (x > 0) {
                x--;
            } else if (y > 0) {
              y -= 1;
              x = (editor->rows[y]).length;
            }
            break;
        case right:
            if (row && x < row->length) {
                x++;
            } else if (row && x == row->length) {
                y++;
                x = 0;
            }
            break;
        case up:
            if (y != 0) {
                y--;
            }
            break;
        case down:
            if (y < totalrows) {
                y++;
            }
            break;
    }
}


void presskey() {
    Editor *editor;
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
            x = cols - 1;
            break;

        case KEY_HOME: // Home 키
            x = 0;
            break;

        case KEY_NPAGE: // Page Down 키
        case KEY_PPAGE: // Page Up 키
        {
            int temprows = rows;
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
            Insert_New_Line(editor, x, y);
            break;

        case KEY_DC:
            Move(right);
            Delete_Char(editor, x, y);
            break;

        case KEY_BACKSPACE:
            Delete_Char(editor, x, y);
            break;

        default:
            Insert_Char(editor, c);
            break;
    }
}


void init() {
    Raw();
    initscr();
    getmaxyx(stdscr, rows, cols);
    keypad(stdscr, TRUE);
    x = 0;
    y = 0;
    totalrows = 0;
}

int main(int argc, char *argv[]) {

    init();
    editorDrawRows();
    x = 0;
    y = 0;

    while (1) {
        presskey();
        write(STDOUT_FILENO, "\x1b[H", 3);  
        editorDrawRows();
    }
    return 0;

}