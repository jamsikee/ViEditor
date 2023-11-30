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

typedef struct Row{
    int length;
    char *string;
} Row;

typedef struct Editor{
  Row *rows;
  int totalrows;
} Editor;

int x = 0;
int y = 0;
int rows = 0;
int cols = 0;

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

void Edit_Insert_row(Editor *editor, int pos, char *line, ssize_t len) {

    if (pos < 0 || pos > editor->totalrows) return;

    editor->rows = realloc(editor->rows, sizeof(Row)*(editor -> totalrows + 1));
    
    memmove(&editor->rows[pos + 1], &editor->rows[pos], sizeof(Row) * (editor -> totalrows - pos));

    editor->rows[pos].string = malloc(len + 1);
   
    memcpy(editor->rows[pos].string, line, len);
    editor->rows[pos].string[len] = '\0';
    editor->rows[pos].length = len;

    editor->totalrows++;

}

void Edit_Del_row(Editor *editor, int pos){

    if (pos < 0 || pos >= editor->totalrows) return;

     free(&editor->rows[pos].string);

     memmove(&editor->rows[pos], &editor->rows[pos + 1], sizeof(Row) * (editor->totalrows - pos - 1));

     editor->totalrows-=1;

}

void Edit_Del_Char_row(Row *rows, int pos) {

    if (pos < 0 || pos >= rows->length){
      return;
    }

    memmove(&rows-> string[pos], &rows-> string[pos + 1], rows->length - pos);
    rows ->length--;

}

void Edit_Insert_Char_row(Row *rows, int pos, char str) {

    if(pos < 0 || pos > rows->length) {
      pos = rows->length;
    }

    char *temp = realloc(rows->string, (rows->length + 2) * sizeof(char));
    rows->string = temp;
    memmove(&rows->string[pos + 1], &rows-> string[pos], rows->length - pos + 1);

    rows->string[pos] = str;
    rows->length++;

}

void Insert_Char(Editor *editor, int str){

    if (y == editor->totalrows) {
      Edit_Insert_row(editor, editor->totalrows, "", 0);
    }
    else {
      Edit_Insert_Char_row(&editor->rows[y], x, str);
    }
    x += 1;

}

void New_Line_Beginning(Editor *editor, int pos){

    Edit_Insert_row(editor, pos, "", 0);
    // pos = y
}

void New_Line_Mid(Editor *editor, int pos){

    Row *rows =  &(editor->rows[pos]);
    Edit_Insert_row(editor, pos+1, &rows->string[x], rows->length - x);
    rows = &(editor->rows[pos]);
    rows->length = x;
    rows->string[rows->length] = '\0';

}

void Insert_New_Line(Editor *editor, int pos){

    if (pos == 0) {
        New_Line_Beginning(editor, pos);
    }
    else {
        New_Line_Mid(editor, pos);
    }
        
    x = 0;
    y += 1;

}

void Prev_Del(Row *rows, char *str, size_t len) {

  rows->string = realloc(rows->string, rows->length + len + 1);
  memcpy(&rows->string[rows->length], str, len);
  rows->length += len;
  rows->string[rows->length] = '\0';

}

void Delete_Char(Editor *editor){

      if( y == editor->totalrows) {
        return;
      }

      if( x == 0 && y == 0) {
        return;
      }

    Row *rows = &(editor->rows[y]);

    if (x > 0) {
        Edit_Del_Char_row(rows, x-1);
    }
    else {
        x = (editor->rows[y].length);
        Prev_Del(&editor->rows[y-1], rows->string, rows->length);
        Edit_Del_row(editor, y);
    }
    x -=1;
}

void Move(int key) {
    Editor *editor;
    Row *rows = &(editor->rows[y]);

    switch (key) {
        case left:
            if (x > 0) {
                x--;
            } else if (y > 0) {
              y -= 1;
              x = editor->rows[y].length;
            }
            break;
        case right:
            if (rows && x < rows->length) {
                x++;
            } else if (rows && x == rows->length) {
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
            if (y < editor->totalrows) {
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
            Insert_New_Line(editor, y);
            break;

        case KEY_DC:
            Move(right);
            Delete_Char(editor);
            break;

        case KEY_BACKSPACE:
            Delete_Char(editor);
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
    
}

int main(int argc, char *argv[]) {
    printf("sibal");
    init();
    while (1) {
        presskey(); 
        refresh();
    }
    return 0;

}