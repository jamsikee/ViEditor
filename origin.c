#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <math.h>

#ifdef __linux__
// 리눅스에서 Enter 키의 ASCII 값은 10
#define enter 10
#elif _WIN32
// 윈도우즈에서 Enter 키의 ASCII 값은 13
#define enter 13
#elif __APPLE__
// 맥에서 Enter 키의 ASCII 값도 10 (리눅스와 동일)
#define enter 10
#endif
int x = 0, y = 0; // 좌표값
int pressQ = 1;//컨+q가 눌렸는지 확인하는변수 
int pressS = 2;//컨+s가 눌렸는지 체크//2는 아무것도 안한 제일 초기상태

//UI용 전역변수
int linenum = 0;//현재 몇라인인지 알려주는 전역변수
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
    int digits2;
    if (totalnum == 0) {
        digits = 1;
    }
    else digits = (int)(log10(totalnum) + 1);
    if (linenum == 0) {
        digits2 = 1;
    }
    else digits2 = (int)(log10(linenum) + 1);
    attron(A_REVERSE);
    for (int i = 0; i < conCols - 11 - strlen(filename) - digits - digits - 11 - digits2; i++) {
        if (i == 0) {
            mvprintw(conRows - 2, 0, "[%s] - %d lines", filename, totalnum);
        }
        printw(" ");
    }
    printw("no ft | %d / %d\n", linenum, totalnum);
    attroff(A_REVERSE); // 반전을 해제합니다.
    if (pressS == 1) {
        printw("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find | FILE SAVED");
    }
    else printw("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
    move(y, x);
    refresh();
    return;
}
void setWave() {
    for (int i = 0; i < conRows - 2; i++) {
        printw("~\n");
    }
    return;
}

void setFirstUI() {//파일이오픈되면 굳이 실행할필요없음.
    for (int i = 0; i < conRows - 2; i++) {
        printw("~");
        if (i == conRows / 3) {
            for (int j = 0; j <= (conCols - 35) / 2; j++) {
                printw(" ");
            }
            printw("Visual Text editor -- version 0.0.1");
        }
        printw("\n");
    }

    setUI();
    move(0, 0);
}
//여기까지 UI세팅함수

int value(int ch) {//입력된 ch가 문자인지 아닌지여부를판단.
    if (ch >= 32 && ch <= 126 || ch == 9) {
        return 1; // ch가 문자면 1반환 letter에 1저장, 탭도 문자로본다
    }
    else
        return 0; // 아니면 0반환 letter에 0저장
}



node* getN() {
    node* newnode = (node*)malloc(sizeof(node));
    newnode->strsize = 0;
    newnode->str = NULL;
    newnode->up = NULL;
    newnode->down = NULL;
    return newnode;
}

void InsertAtTail() {
    node* tmp = getN();
    if (head == NULL) {
        head = tmp;
        return;
    }
    node* cur = head;
    while (cur->down != NULL) {
        cur = cur->down;
    }
    cur->down = tmp;
    tmp->up = cur;
    curNode = tmp;
    totalnum += 1;
    linenum++;
}
//만약 x가 문자열의 끝이면 인풋str을 하고 아니면 체인지str을 하고그러자
//딜리트도 생각하기>문자열맨끝의경우, 문자열 가운데의 경우, 라인을 다지운경우(노드까지=문자열들고올라가기 등) 
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
    x++;
    refresh();
    return;
}

//만약 x가 맨끝이아니라면. >> curNode->str != NULL, strlen(curNode) != x
void changeStr(int ch) {//이건 curNode로 대치하면될듯, int ch) {
    curNode->strsize++;
    if (x < 0 || x >= curNode->strsize) {
        printw("Invalid position");
        return;
    }
    curNode->str = (char*)realloc(curNode->str, (curNode->strsize + 1) * sizeof(char));
    if (curNode->str == NULL) {
        printw("Memory allocation failed");
        return; // Exit with an error code
    }

    for (int i = curNode->strsize - 1; i > x; i--) {
        curNode->str[i] = curNode->str[i - 1];
    }
    curNode->str[x] = ch;
    curNode->str[curNode->strsize] = '\0'; // Null-terminate the string
    mvprintw(y, 0, "%s", curNode->str);
    x++;
    move(y, x);
    refresh();
    return;
}
void delCurNode() {
    if (curNode == NULL) return;
    node* cur = curNode;
    cur->up->down = cur->down;
    if (cur->down != NULL) {
        cur->down->up = cur->up;
    }
    curNode = curNode->up;
    free(cur);
    return;
}

void rePrintConsol() {
    node* cur = head;
    clear();
    for (int i = 1; i < linenum - y && cur->down != NULL; i++) {
        cur = cur->down;
    }
    for (int i = 0; i <= conRows - 3, cur != NULL; i++) {
        if (cur->str == NULL) {
            mvprintw(i, 0, " ");
        }
        else {
            mvprintw(i, 0, "%s", cur->str);
        }
        cur = cur->down;
    }
    refresh();
    return;
}
void rePrintEnt() {
    node* cur = head;
    clear();
    //콘솔 로우 -3까지출력해야하고, cur!=NULL일때까지고 어디부터냐
    //현재 콘솔 0번부턴데 이거는>>>커서노드를 이동시켜서 0번으로 지정해줘야함
    //내가있는 라인넘 -y값 2-11인데 0번이 2가되죠 그럼 한번만큼 다운한거.2-11인데 내가 3이면
    for (int i = 1; i < linenum - y && cur->down != NULL; i++) {
        cur = cur->down;
    }
    for (int i = 0; i <= conRows - 3, cur != NULL; i++) {
        if (cur->str == NULL) {
            mvprintw(i, 0, " ");
        }
        else {
            mvprintw(i, 0, "%s", cur->str);
        }
        cur = cur->down;
    }
    refresh();
    //linenum-y만큼부터 수행(if linenum이 콘솔 -2보다크면)
    return;
}
void rePrintDel() {//내위치부터재출력. 0,0일때 화면하나올라가는것도해야함.
    node* cur = head;
    clear();
    //콘솔 로우 -3까지출력해야하고, cur!=NULL일때까지고 어디부터냐
    //현재 콘솔 0번부턴데 이거는>>>커서노드를 이동시켜서 0번으로 지정해줘야함
    //내가있는 라인넘 -y값 2-11인데 0번이 2가되죠 그럼 한번만큼 다운한거.2-11인데 내가 3이면
    for (int i = 1; i < linenum - y && cur->down != NULL; i++) {
        cur = cur->down;
    }
    for (int i = 0; i <= conRows - 3, cur != NULL; i++) {
        if (cur->str == NULL) {
            mvprintw(i, 0, " ");
        }
        else {
            mvprintw(i, 0, "%s", cur->str);
        }
        cur = cur->down;
    }
    refresh();
    //linenum-y만큼부터 수행(if linenum이 콘솔 -2보다크면)
    return;
}

void deleteChar() {
    if (linenum == 1 && x == 0) {
        return;
    }
    if (linenum != 1) {
        if (curNode->str == NULL || x == 0) {// 삭제할 문자열이 없거나, x가 0이면 노드삭제하고 위로밀어올려야지.
            //node* cur = curNode;
            //move(y, 0);//위에 거 문자열길이가 conCols보다 길거나 합쳐져서 넘어가면 그만큼만잘라올려야함.
            //clrtoeol();
            //move(y, x);
            //for (int i = y; i <= conRows - 3, cur != NULL; cur = cur->down, i++) {
            //    move(i, 0);
            //    clrtoeol();
            //    move(y, x);
            //}
            if (totalnum > 0)totalnum--;
            if (linenum > 0)linenum--;
            if (y > 0) y--;
            x = curNode->up->strsize;
            curNode->up->str = (char*)realloc(curNode->up->str, (curNode->up->strsize + curNode->strsize) * sizeof(char));
            for (int i = curNode->up->strsize, j = 0; j < curNode->strsize; i++, j++) {
                curNode->up->str[i] = curNode->str[j];
            }
            //strcat(curNode->up->str, curNode->str);  위에가 이거한거랑동일.
            curNode->up->strsize = curNode->up->strsize + curNode->strsize;
            curNode->up->str[curNode->up->strsize] = '\0';
            delCurNode();
            //아래애들도 재출력해야함.
            //rePrintDel();
            rePrintConsol();
            move(y, x);
            setUI();
            return;
        }
    }
    curNode->strsize--;
    curNode->str = (char*)realloc(curNode->str, (curNode->strsize + 1) * sizeof(char));
    if (curNode->str == NULL) {
        printw("Memory allocation failed");
        return; // Exit with an error code
    }
    for (int i = x - 1; i < curNode->strsize - 1; i++) {
        curNode->str[i] = curNode->str[i + 1];
    }
    curNode->str[curNode->strsize] = '\0'; // Null-terminate the string
    mvprintw(y, 0, "%s ", curNode->str); // 삭제 후 화면 갱신
    if (x > 0) {
        x--; // 커서를 왼쪽으로 옮김
    }
    move(y, x);
    refresh();
}

void insertWant(node* tmp) {
    // 빈 리스트인 경우
    if (head == NULL) {
        head = tmp;
        totalnum += 1;
        linenum++;
        return;
    }
    node* cur = head;
    // 리스트의 맨 앞에 삽입
    if (linenum == 1 && x == 0) {
        tmp->down = head;
        head->up = tmp;
        head = tmp;
    }
    //else if (linenum!=1&&x == 0&&curNode->str!=NULL) {//리스트중간삽입인데 x=0일때 엔터의 경우
    //    node* a = curNode;
    //    a->up = tmp;
    //    tmp->down = a;
    //    tmp->up = a->up;
    //    a->up->down = tmp;
    //}
    else {
        // 리스트 중간에 삽입
        cur = head;
        for (int i = 0; i < linenum - 1; i++) {
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
    //cur = curNode;

    /*if (curNode->down != NULL) {

        for (int i = y; cur->down != NULL; cur = cur->down, i++) {
            mvprintw(i, 0, "%s", cur->str);
            refresh();
        }

    }*/
    //엔터들어오면 행이하나 밀려야하는데 어케하지. 엔터가일어나고 행이밀리면
    // 행이밀리면==커서의 넥스트가 널이아니라면 밀릴행이있지.그러면? 새로프린트.
    // 삽입하고 출력하는 라인
    totalnum += 1;
    linenum += 1;
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

// 화살표 이동 함수
void curMove(int ch) {//화면넘어가는 경우도 생각하자. 나중에 화면넘어가는거 구현하고.
    if (head != NULL) {
        switch (ch) {
        case KEY_UP:
            if (curNode->up == NULL) {//위노드가 널이면 이동x
                return;
            }
            if (curNode->up->str == NULL) {//윗줄문자열이 널이면 0으로이동
                x = 0;
            }
            else if (curNode->up->strsize < x) {//위에가 지금보다작으면 위에거 맨끝으로이동
                x = curNode->up->strsize;
            }//화면이동 위로올라갈때
            if (y > 0) {
                y--;
            }
            //만약 화면전환이 필요한 위치에서 있으면 만들어야함
            else if (y == 0 && curNode->up != NULL) {
                node* cur = curNode;
                cur = cur->up;
                clear();
                for (int i = 0; i <= conRows - 3; i++) {
                    if (cur->str == NULL) {
                        mvprintw(i, 0, " ");
                    }
                    else {
                        mvprintw(i, 0, "%s", cur->str);
                    }
                    cur = cur->down;
                }
                refresh();
            }
            curNode = curNode->up;
            linenum--;
            setUI();
            move(y, x);
            break;
        case KEY_DOWN:
            if (curNode->down == NULL) {//아래노드가없으면 이동x
                return;
            }
            if (curNode->down->str == NULL) {//현재줄의 아랫줄이 널이면 0으로이동
                x = 0;
            }
            else if (curNode->down->strsize < x) {//현재줄의 아래문자열길이가 내 커서위치보다 작으면 커서는 문자열길이만큼
                x = (curNode->down->strsize);
            }

            if (y < conRows - 2 - 1) {
                y++;
            }
            //만약 화면전환이 필요한 위치에서 있으면 만들어야함
            else if (y == conRows - 2 - 1 && curNode->down != NULL) {
                node* cur = head;
                for (int i = 1; i <= abs((conRows - 3) - linenum); i++) {//차이 절댓값만큼 돌기
                    cur = cur->down;
                }
                clear();
                for (int i = 0; i <= conRows - 3; i++) {
                    if (cur->str == NULL) {
                        mvprintw(i, 0, " ");
                    }
                    else {
                        mvprintw(i, 0, "%s", cur->str);
                    }
                    cur = cur->down;
                }
                refresh();
            }
            curNode = curNode->down;
            linenum++;
            setUI();
            move(y, x);
            break;
        case KEY_LEFT:
            if (y == 0 && x == 0 && curNode->up == NULL) {
                break;
            }
            else if (y == 0 && x == 0 && curNode->up != NULL) {//x가 0이고 y도 0인데 위에 노드가 문자열을 갖고있으면 그걸 출력해야겠지.
                node* cur = curNode;
                cur = cur->up;
                clear();
                for (int i = 0; i <= conRows - 3; i++) {
                    if (cur->str == NULL) {
                        mvprintw(i, 0, " ");
                    }
                    else {
                        mvprintw(i, 0, "%s", cur->str);
                    }
                    cur = cur->down;
                }
                refresh();
            }
            if (x == 0) {
                if (curNode->up == NULL) {//내거위에가 널이면 안함.
                    break;
                }
                else if (curNode->up->str == NULL) {//내 위의 문자열이 널이면 x는 0임.
                    x = 0;
                }
                else x = curNode->up->strsize;//내 위 문자열이 널아니면 x는 문자열맨끝으로감
                if (y == 0);//y가 0이면 아무것도안함.
                else y--;//y가 0이상이면 y값 -1함.
                linenum--;
                curNode = curNode->up;
            }
            else x--;
            move(y, x);
            break;
        case KEY_RIGHT:
            if (curNode->str == NULL || x == curNode->strsize) {//커서가 현재노드의 끝자락이면
                if (curNode->down == NULL) {//커서밑에뭐없으면브레이크
                    break;
                }
                if (curNode->down->str == NULL) {//커서밑 문자열이 널이면 x는 0
                    x = 0;
                }
                else x = 0;//디폴트는 x=0
                if (y == conRows - 2 - 1);//콘솔창 마지막행이면 y는 그대로
                else y++;
                if (y == conRows - 2 - 1 /* && curNode->down != NULL */) {//커서가 문자열의끝자락이고, 콘솔의 마지막이면 화면전환.
                    node* cur = head;
                    for (int i = 1; i <= abs((conRows - 3) - linenum); i++) {
                        cur = cur->down;
                    }
                    clear();
                    for (int i = 0; i <= conRows - 3; i++) {
                        if (cur->str == NULL) {
                            mvprintw(i, 0, " ");
                        }
                        else {
                            mvprintw(i, 0, "%s", cur->str);
                        }
                        cur = cur->down;
                    }
                    refresh();
                }
                linenum++;
                curNode = curNode->down;
            }
            else x++;
            move(y, x);
            break;
        }
    }
    setUI();
    return;
}

void specialKey(int ch) {
    switch (ch) {
    case KEY_HOME:
        // Home 키 처리
        x = 0;
        move(y, x);
        //printw("Home key\n");
        break;
    case KEY_END:
        if (curNode->str == NULL) {
            x = 0;
        }
        else x = curNode->strsize;
        // End 키 처리
        move(y, x);
        //printw("End key\n");
        break;
    case KEY_PPAGE:
        // Page Up 키 처리
        if (linenum <= conRows - 3) {
            y = 0;
            x = 0;
            linenum = 1;
            curNode = head;
        }
        else {
            linenum = linenum - conRows - 3;
            curNode = head;
            for (int i = 1; i < linenum && curNode->down != NULL; i++) {
                curNode = curNode->down;
            }
            if (curNode->str == NULL) {
                x = 0;
            }
            else if (x > curNode->strsize) {
                x = curNode->strsize;
            }
        }
        move(y, x);
        //printw("Page Up key\n");
        break;
    case KEY_NPAGE:
        if (totalnum <= conRows - 3) {
            y = totalnum;
            linenum = totalnum;
            while (curNode->down != NULL) {
                curNode = curNode->down;
            }
            if (curNode->str == NULL) {//if(x>curNode->strszie) x=curNode->strsize
                x = 0;
            }
            else if (x > curNode->strsize) {
                x = curNode->strsize;
            }
        }
        else {
            linenum = linenum + conRows - 3;
            curNode = head;
            for (int i = 1; i < linenum && curNode->down != NULL; i++) {
                curNode = curNode->down;
            }
            if (curNode->str == NULL) {
                x = 0;
            }
            else if (x > curNode->strsize) {
                x = curNode->strsize;
            }
        }
        // Page Down 키 처리
        //printw("Page Down key\n");
        move(y, x);
        break;
    }
    rePrintConsol();
    return;
}

int main(int argc, char* argv[]) {
    initscr();//curses를 초기화
    start_color();//이거해야 컬러기능사용가능
    cbreak();//라인버퍼비활성화,입력을 즉시 프로그램으로전달>>엔터없이 즉시입력됨
    noecho();//키입력에대한 출력을 비활성화>>입력된 키가 화면에 표시x
    raw();//특수키를 읽지않음.
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, conRows, conCols);//현재터미널의 행열을 저장함.

    int ch;//글자를 받아올 ch>아스키값으로 인식한다.
    int letter;//글자인지아닌지(특수키인가)구분할 변수
    //int stop = 0;
    int firstRun = 1;//제일처음 ch를받아왔는지 확인하는 변수
    int fileCh;
    if (argc > 1) {//파일을 읽어오면
        FILE* file = fopen(argv[1], "r+b"); // "r+b" 모드로 파일 열기
        if (file == NULL) {
            printw("error:file is null");
            return 1; // 에러 코드로 종료
        }
        filename = argv[1];//파일이름저장된곳이 argv[1]임
        while ((fileCh = fgetc(file)) != EOF) {
            if (head == NULL || fileCh == '\n' || curNode->strsize == conCols) {
                InsertAtTail();
            }

            curNode->strsize++;
            curNode->str = (char*)realloc(curNode->str, (curNode->strsize + 1) * sizeof(char));

            if (curNode->str == NULL) {
                printw("메모리 할당 실패");
                return 1; // 에러 코드로 종료
            }

            curNode->str[curNode->strsize - 1] = fileCh;
            curNode->str[curNode->strsize] = '\0'; // 문자열을 null로 종료
            refresh();
        }
        x = 0;
        y = 0;
        curNode = head;
        move(y, x);
        rePrintConsol();
        setUI();
        fclose(file);
    }
    if (argc < 2) {//argc<2 : 파일오픈x
        if (filename == NULL) {
            filename = "Noname";//나중에 파일이름가져오기, 없으면 noname
        }
        setFirstUI();//파일이름이 없는 새 파일의 경우에 실행해야함 수정필요
    }

    ch = getch();
    letter = value(ch);
    while (1) {
        if (ch != 17 && ch != 19 && ch != KEY_HOME && ch != KEY_END && ch != KEY_PPAGE
            && ch != KEY_NPAGE && ch != KEY_UP && ch != KEY_DOWN && ch != KEY_LEFT && ch != KEY_RIGHT)
        {
            pressQ = 0;
            pressS = 0;
            setUI();
        }
        if (firstRun) {//처음실행하는거면 UI초기세팅을 클리어함
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
            setUI();
        }
        else if (letter == 1) {
            //만약 x가 맨끝이아니라면. >> curNode->str != NULL, strlen(curNode) != x
            if (curNode->str != NULL) {
                if (strlen(curNode->str) - 1 != x || x == 0) {
                    changeStr(ch);
                }
                else if (strlen(curNode->str) != x) {
                    changeStr(ch);
                }
            }
            else inputStr(curNode, ch);
        }
        else if (letter == 0) {//문자가아닌 특수키의 경우.
            // ctrl+q면 종료 나중에 두번눌러야 종료되게실행
            if (ch == 17) {
                move(conRows - 1, 0);
                clrtoeol();
                pressQ = 1;
                if (pressS == 0 && pressQ == 1) {
                    mvprintw(conRows - 1, 0, "*Not Saved* press one more to quit");
                    ch = getch();
                }
                else if (pressS == 1 && pressQ == 1) {
                    mvprintw(conRows - 1, 0, "*File Saved* press one more to quit");
                }
                //refresh();

                if (ch == 17) {
                    break;
                }
                else pressQ = 0; setUI(); move(y, x);
            }
            //ctrl+f는 어떡할까. 찾는 문자열 입력하고엔터들어오면(findStr리얼록)
            //linenum==1부터 끝까지 도는데 strlen만큼
            // 배열값하나씩다비교함. 첫번째맞으면 두번째비교...n까지
            //맞으면 원형큐에넣고 없으면 notfind출력 큐말고 원형더블로할까.  어쨋든 새로운 구조체만들고
            // 그 구조체는 해당라인의 라인넘버랑, 배열번호기억해야함
            // 없으면 pressany key move(y,x)로 가고
            //화살표 왼,위 가들어오면 이전거로감. 밑,오가들어오면 다음거로감 그거하이라이트하기
            //화면이넘어가야하면(현재라인넘버, find라인넘버차이의 절댓값이 현재콘솔최대행-현재y축넘버보다
            //                  크면 화면이동해야함) 그럼 그 find라인넘버가 0,0되고 화면새로출력하고
            //배열번호만큼 하이라이트
            //getch()로 "나가고 싶으면 프레스 q해라 등 수행"
            if (ch == 19) {//ctrl+s
                if (filename == "No Name") {
                    //getch()로 받고 리얼록하고파일네임저장하고 등등 setUI도 새로함
                }
                //savefile(); 구현하자
                pressS = 1;
                pressQ = 1;
                setUI();
            }
            if (ch == 8) {
                deleteChar();
            }
            //특수키실행
            if (ch == KEY_HOME || ch == KEY_END || ch == KEY_PPAGE || ch == KEY_NPAGE) {
                specialKey(ch);
            }
            // 화살표면 화살표 실행
            if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
                curMove(ch);
            }
            // 엔터
            if (ch == enter) { // 엔터가 13으로 입력되네, 운영체제마다 엔터 입력값 찾아보자 리눅스면 10임
                if (head == NULL) {//헤드가 널이면 엔터들어올때 노드 두개만들어야함
                    y++;
                    x = 0;
                    curNode = getN();
                    insertWant(curNode);
                    inputStr(curNode, ch);
                    move(y, x);
                    curNode = getN();
                    insertWant(curNode);
                    inputStr(curNode, ch);
                    x = 0;
                    move(y, x);
                    setUI();
                }
                else {
                    if (y < conRows - 3) {//y가 콘솔열보다 작게있으면 y값추가.
                        y++;//원래 줄이 남아있으면 자르고 넘겨야함. x위치기준. 구현하자
                    }
                    if (x == 0 && curNode != NULL) {
                        node* newNode = getN();
                        insertWant(newNode);
                        move(y, x);
                    }
                    //else if (x < curNode->strsize) {
                    //    node*newNode = getN();
                    //    newNode->strsize = curNode->strsize - x;
                    //    newNode->str = (char*)realloc(newNode->str, (newNode->strsize + 1) * sizeof(char));
                    //    for (int i = 0; i<curNode->strsize; i++) {
                    //        newNode->str[i]=curNode->str[curNode->strsize-x+i];
                    //    }
                    //    newNode->str[newNode->strsize] = '\0'; // Null-terminate the string
                    //    curNode->strsize = curNode->strsize - x;
                    //    curNode->str = (char*)realloc(curNode->str, (curNode->strsize + 1) * sizeof(char));
                    //    //curNode->str[x] = ch;
                    //    curNode->str[curNode->strsize] = '\0'; // Null-terminate the string
                    //    //newNode->strsize = strlen(newNode->str-1);
                    //    x = 0;
                    //    insertWant(newNode);
                    //}
                    else {
                        curNode = getN();
                        insertWant(curNode);
                        //inputStr(curNode, ch);
                        x = 0;
                        //rePrintEnt();
                        move(y, x);
                    }
                    move(y, x);
                    rePrintConsol();
                    setUI();

                }
            }
        }
        refresh();
        ch = getch();
        letter = value(ch);
    }

    freeNode();
    endwin();
    return 0;
}