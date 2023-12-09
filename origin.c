#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>

int x = 0, y = 0; // 좌표값


//UI용 전역변수
int linenum = 1;//현재 몇라인인지 알려주는 전역변수
int conRows, conCols;//콘솔의 행열
char* filename = NULL;//파일이름
char* ft = NULL;//현재라인/전체라인 no ft | y / totamnum;
int frontLength;//파일이름포함한 길이+라인넘버포함한길이
int backLength;//뒤에 f|t라인 길이
int totalnum = 0;

typedef struct DNode {
    int strsize;
    char* str;
    struct DNode* up;
    struct DNode* down;
} node;

node* head = NULL;     // 맨 위의 노드를 가리키는 포인터
node* curNode = NULL;  // 현재 위치한 노드를 가리키는 포인터

//UI세팅함수들
void setUI() {
    // 로그를 취한 후 1을 더하여 자릿수 계산
    int digits;
    if (totalnum == 0) {
        digits = 1;
    }
    else digits = (int)(log10(totalnum) + 1);
    attron(A_REVERSE);
    for (int i = 0; i < conCols - 11 - strlen(filename) - digits - strlen(ft); i++) {
        if (i == 0) {
            mvprintw(conRows - 2, 0, "[%s] - %d lines", filename, totalnum);
        }
        printw(" ");
    } 
    printw("%s\n", ft);
    attroff(A_REVERSE); // 반전을 해제합니다.
    printw("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
    refresh();
    return;
}
void setWave(){
    for (int i = 0; i < conRows - 2; i++) {
        printw("~\n");
    }
    return;
}

void setFirstUI() {//파일이오픈되면 굳이 실행할필요없음.
    for (int i = 0; i < conRows - 2; i++) {
        printw("~");
        if (i == conRows/3) {
            for (int j = 0; j <= (conCols - 35) / 2; j++) {
                printw(" ");
            }
            printw("Visual Text editor -- version 0.0.1");
        }
        printw("\n");
    }
    if (filename == NULL) {
        filename = "Noname";//나중에 파2일이름가져오기, 없으면 noname
      //  frontLength = snprintf(NULL, 0, "[%s] - %d lines", filename, linenum);//나중에 no name도 수정해야함
        ft = "no ft | 1 / 0";
       // backLength = snprintf(NULL, 0, "%s", ft); // 나중에 변경되는값가져와서 수정
    }
    setUI();
    move(0, 0);
}
//여기까지 UI세팅함수

int value(int ch) {//입력된 ch가 문자인지 아닌지여부를판단.
    if (ch >= 32 && ch <= 126 || ch==9) {
        return 1; // ch가 문자면 1반환 letter에 1저장, 탭도 문자로본다
    }
    else
        return 0; // 아니면 0반환 letter에 0저장
}

// 화살표 이동 함수
void curMove(int ch) {
    if (head != NULL) {
        switch (ch) {
        case KEY_UP:
            if (curNode->up == NULL) {
                return;
            }
            curNode = curNode->up;
            y--;
            move(y, x);
            break;
        case KEY_DOWN:
            if (curNode->down == NULL) {
                return;
            }
            curNode = curNode->down;
            y++;
            move(y, x);
            break;
        case KEY_LEFT:
            x--;
            move(y, x);
            break;
        case KEY_RIGHT:
            x++;
            move(y, x);
            break;
        }
    }
    
    return;
}

node* getN() {
    node* newnode = (node*)malloc(sizeof(node));
    newnode->strsize = 0;
    newnode->str = NULL;
    newnode->up = NULL;
    newnode->down = NULL;
    return newnode;
}

void inputStr(node* tmp, int ch) {
    tmp->strsize++;
    tmp->str = (char*)realloc(tmp->str, (tmp->strsize + 1) * sizeof(char));
    if (tmp->str == NULL) {
        printw("Memory allocation failed");
        return; // Exit with an error code
    }
    tmp->str[tmp->strsize - 1] = ch;
    tmp->str[tmp->strsize] = '\0'; // Null-terminate the string
    mvprintw(y, 0, "%s", tmp->str);
    refresh();
    return;
}

void insertWant(node* tmp) {
    // 빈 리스트인 경우
    if (head == NULL) {
        head = tmp;
        return;
    }

    node* cur = head;

    // 리스트의 맨 앞에 삽입
    if (y == 0) {
        tmp->down = head;
        head->up = tmp;
        head = tmp;
    }
    else {
        // 리스트 중간에 삽입
        for (int i = 0; i < y - 1; i++) {
            cur = cur->down;
        }
        if (cur == NULL) {
            printw("삽입할 위치가 리스트의 크기를 벗어났습니다.\n");
            refresh();
            return;
        }
        tmp->down = cur->down;
        tmp->up = cur;
        if (cur->down != NULL) {
            cur->down->up = tmp;
        }
        cur->down = tmp;
        curNode = tmp;
    }
    cur = curNode;
    
    if (curNode->down != NULL) {
        
        for (int i = y; cur->down != NULL; cur = cur->down, i++) {
            mvprintw(i, 0, "%s", cur->str);
            refresh();
        }
        
    }
    //엔터들어오면 행이하나 밀려야하는데 어케하지. 엔터가일어나고 행이밀리면
    // 행이밀리면==커서의 넥스트가 널이아니라면 밀릴행이있지.그러면? 새로프린트.
    // 삽입하고 출력하는 라인
    totalnum += 1;

    return;
}

void freeNode() {
    node* cur = head;
    if (head == NULL) {
        return; // 빈 리스트라면 프리돼있는거
    }
    while (head != NULL) {
        head = cur->down;
        free(cur);
        cur = head;
    }
    return;
}

int main() {
    initscr();//curses를 초기화
    start_color();//이거해야 컬러기능사용가능
    cbreak();//라인버퍼비활성화,입력을 즉시 프로그램으로전달>>엔터없이 즉시입력됨
    noecho();//키입력에대한 출력을 비활성화>>입력된 키가 화면에 표시x
    raw();//특수키를 읽지않음.
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, conRows, conCols);//현재터미널의 행열을 저장함.
    

    int ch;
    int letter;
    int stop = 0;
    int firstRun = 1;

    setFirstUI();
    ch = getch();
    letter = value(ch);

    while (1) {
        if (firstRun) {
            if (letter == 1 || ch == 13) {//13은 엔터, 9는 탭
                clear();
            }
            setWave();
            setUI();
            firstRun = 0;
        }
        
        if (head == NULL && letter == 1) {
            curNode = getN();
            insertWant(curNode);
            inputStr(curNode, ch);
            totalnum += 1;
            setUI();
        }
        else if (letter == 1) {
            inputStr(curNode, ch);
        }
        else if (letter == 0) {
            // F2 누르면 종료
            if (ch == KEY_F(2)) {
                break;
            }
            // 화살표면 화살표 실행
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                curMove(ch);
            }
            if(ch == KEY_HOME){
                move(y, 0);
            }
            // 엔터
            if (ch == 13) { // 엔터가 13으로 입력되네, 운영체제마다 엔터 입력값 찾아보자
                y++;
                move(y, 0);
                curNode = getN();
                insertWant(curNode);
            }
        }
        ch = getch();
        letter = value(ch);
    }

    freeNode();
    endwin();
    return 0;
}