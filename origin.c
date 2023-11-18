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
    int rows
    int cols;
    int currentrows;
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

void tilde() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    rows -= 2;

    for (int y = 0; y < rows; y++) {
        move(y, 0);
        printw("~");
    }

    char* version = "My Editor Version";
    int len = strlen(version);
    int padding = (cols - len) / 2;
    move(rows / 3, padding > 0 ? padding : 0);
    printw(version);

    refresh();
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
    tilde();
    while (1) {
    presskey();
    }
  return 0;
}