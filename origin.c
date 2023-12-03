#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdbool.h>

#define CONTROL(k) ((k) & 0x1f)

struct termios orig_termios;

enum P_key {

    left = 1000,
    right,
    up,
    down,
    del,
    home,
    end,
    pg_up,
    pg_dn,
    b_s = 127
    
};

bool error = false;

int x;
int y;
int rows;
int cols;
int move_rows;
int move_cols;

typedef struct Row {

  int len;
  char *c;

} Row;

struct Visual_Text_Editor{

  int total;
  Row *line;
  char *filename;

};

struct Visual_Text_Editor Edit;

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

void for_quit(){

    write(STDOUT_FILENO, "\x1b[2J", 4); // clear UI
    write(STDOUT_FILENO, "\x1b[0;0H", 3);  // cursor (0, 0)

}

int Read_Key() {

  int Return_value;
  char c;
  while ((Return_value = read(STDIN_FILENO, &c, 1)) != 1) {
    error = true;
  }

  char ESCAPE = '\x1b';                  // For defualt value ANSI ESCAPE SEQUENCE 

  if (c == ESCAPE) {
    char List[3];
    if ((Return_value = read(STDIN_FILENO, &List[0], 1)) != 1)
      return ESCAPE;
    if ((Return_value = read(STDIN_FILENO, &List[1], 1)) != 1)
      return ESCAPE;

    if (List[0] == '[') {             // For processing ANSI ESCAPE SEQUENCE
      if (List[1] >= '0' && List[1] <= '9') {
        if ((Return_value = read(STDIN_FILENO, &List[2], 1)) != 1) {
          return ESCAPE;
        }
        if (List[2] == '~') {
          if (List[1] == '1') {
            return home;              // \x1b[1~
          } else if (List[1] == '3') {
            return del;               // \x1b[3~
          } else if (List[1] == '4') {
            return end;               // \x1b[4~
          } else if (List[1] == '5') {
            return pg_up;             // \x1b[5~
          } else if (List[1] == '6') {
            return pg_dn;             // \x1b[6~
          }
        }
      } else {
        if (List[1] == 'A') {
          return up;                  // \x1b[A
        } else if (List[1] == 'B') {
          return down;                // \x1b[B
        } else if (List[1] == 'C') {
          return right;               // \x1b[C
        } else if (List[1] == 'D') {
          return left;                // \x1b[D
        } else if (List[1] == 'H') {
          return home;                // \x1b[H
        } else if (List[1] == 'F') {
          return end;                 // \x1b[F
        } else {
          return ESCAPE;
        }
      }
    } else if (List[0] == '0') {
      if (List[1] == 'H') {
       return home;                 // \x1bOH
      } else if (List[1] == 'F') {
        return end;                 // \x1bOF
      }
    }
      return ESCAPE;
  } else {
    return c;
  }
}

void Move(int key) {

    switch (key) {
        case left:
            if (x != 0) {
                x--;
            } /*
            else if (y > 0) {
                y--;
            }
            */
            break;
        case right:
        /*
            if (row && x < row->size) {
                x++;
            } else if (row && x == row->size) {
                y++;
                x = 0;
            }
            */
            break;
        case up:
            if (y != 0) {
                y--;
            }
            break;
        case down:
            if (y < Edit.total) {
                y++;
            }
            break;
    }
}


void presskey() {

    int key_val = Read_Key();

    switch (key_val) {
        case CONTROL('q'):  // Ctrl + Q
            for_quit();
            exit(0);
            break;

        case CONTROL('s'):  // Ctrl + S
            break;

        case CONTROL('f'):  // Ctrl + F
            break;

        case left: // Arrow Left KEY
        case right: // Arrow Right KEY
        case up: // Arrow Up KEY
        case down: // Arrow Down KEY
            Move(key_val);
            break;
            
        case end: // End KEY
            x = cols - 1;
            break;

        case home: // Home KEY
            x = 0;
            break;

        case pg_up: // Page Down KEY
        case pg_dn: // Page Up KEY
        {
            int temprows = rows;
            while (temprows--) {
                if (key_val == pg_up)
                    Move(up);
                else if (key_val == pg_dn)
                    Move(down);
            }
        }
            break;

        case '\r':  // ENTER KEY
            //editorInsertNewline();
            break;

        case del:
            //editorDelChar();
            //Move(right);
            break;

        case b_s:
            //editorDelChar();
            break;

        default:
            //editorInsertChar(key_val);
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
    Edit.total = 0;
    move_cols = 0;
    move_rows = 0;

}

void open_file(char *filename) {
  
  free(Edit.filename);
  Edit.filename = strdup(filename);

  FILE *file = fopen(filename, "r");

  char *row = NULL;
  size_t size = 0;
  ssize_t line_len;

  while ((line_len = getline(&row, &size, file)) != -1) {
    while (line_len > 0 && (row[line_len - 1] == '\r' ||
                               row[line_len - 1] == '\n'))
      line_len--;
    //insert_editor_row_at(Edit.total, row, line_len);
  }

  free(row);
  fclose(file);

}

int main(int argc, char *argv[]) {

  init();

  if (argc >= 2) {
    open_file(argv[1]);
  }

  while (1) {
    presskey();
  }

  endwin();
  return 0;

}