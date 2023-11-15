#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctype.h>
#define CONTROL(k) ((k) & 0x1f)  // 문자의 아스키코드 지정 ctrl-A = 1

enum P_key{
    B_space,
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

void Raw() {
    struct termios raw;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
}

void presskey(){
    char c;
    switch (c)
    {
    case CONTROL('q'):
        exit(0); // 프로그램 종료
        break;
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
    }
}

int main(){
    Raw();
    char c;
    while (1) {
    presskey();
  }
  return 0;
}