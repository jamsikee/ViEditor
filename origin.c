  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <fcntl.h>
  #include <ctype.h>
  #include <ncurses.h>
  #include <stdbool.h>
  #include <stdarg.h>
  #include <unistd.h>

  #define CONTROL(k) ((k) & 0x1f) // control + k
  #define INIT_ROW_SIZE 1000
  #define INIT_LINE_SIZE 125

int x = 0;
int y = 0; //1 최대 54 
int y_out = 0; // +1
int rows = 0; //54
int cols = 0;
int move_rows = 0;
int move_cols = 0;
int total = 0;
int flag = 0;

typedef struct Row {

  int len;
  char *c;
  int line_capacity;

} Row;

struct Visual_Text_Editor{

  Row *line;
  char *filename;

};

// total function
struct Visual_Text_Editor Edit;
void get_windows_size();
Row *get_line(Row *line, int pos);
void InsertRow(int edit_y, char *line, int line_len);
void FreeRow(Row *line);
void DeleteRow(int pos);
void RowInsertString(Row *line, char *str, size_t del_line_len);
void RowDeletechar(Row *line, int pos);
void RowInsertchar(Row *line, char word, int pos);
void empty_new_line(int pos);
void Insertchar(char word);
void Del_current_line_char();
void Del_current_line();
void DeleteChar();
void contained_new_line(Row *line, int pos_y, int pos_x);
void Newline();  //y_out + 1; line[y+y_out]
void status_bar(); 
void state();
void end_message( const char *format, ...);
void all_refresh();
void clean_and_printing();


Row *get_line(Row *line, int pos) {

    return &line[pos];
    // get line index

}

void welcome(){
  const char *message = "Visual Text editor -- version 0.0.1";
  int len = strlen(message);
  int mid = (cols - len)/2;
  mvprintw(rows/3, mid, "%s", message);
}

void InsertRow(int edit_y, char *line, int line_len) {
  if (edit_y < 0) {
    return;
  }
  else if(edit_y > total){
    return;
  }
  // If y < 0 or y > total then return

  if (total == 0) {
    Edit.line = malloc(sizeof(Row) * INIT_ROW_SIZE);
  } else if (total % INIT_ROW_SIZE == 0) {
    Edit.line = realloc(Edit.line, sizeof(Row) * (total * 2));
  }
  /*
  Edit.line's memory = INIT_ROW_SIZE(1000)
  If total % 1000 == 0 then realloc 1000 * 2
  */
  memmove(&Edit.line[edit_y + 1], &Edit.line[edit_y], sizeof(Row) * (total - edit_y));
  // Memory move line[y] -> line[y + 1]
  Edit.line[edit_y].len = line_len;   
  Edit.line[edit_y].c = malloc(INIT_LINE_SIZE + 1);
  Edit.line[edit_y].line_capacity = INIT_LINE_SIZE + 1; 
  // Line_capacity is (Edit.line[y].c)'s size
  memcpy(Edit.line[edit_y].c, line, line_len);
  Edit.line[edit_y].c[line_len] = '\0'; 
  // The end of the string is null
  total+=1;

}

void FreeRow(Row *line){  // Efficient method for free memory

  free(line->c);

}

void DeleteRow(int pos){

  if (pos < 0){  
    return;
  }
  else if(pos >= total){
    return;
  }
  // If y < 0 or y > total then return

  FreeRow(&Edit.line[pos]); // Line[pos]'s memory free
  memmove(&Edit.line[pos], &Edit.line[pos+1], sizeof(Row) * (total - pos - 1));
  // Line[pos+1]'s memory move to free memory(line[pos]) 
  total-=1;

}

void RowInsertString(Row *line, char *str, size_t del_line_len){

  // This function will use delete char at x = 0 then delete row
  while (line->len + del_line_len > line->line_capacity){   
    line->line_capacity *= 2;
    line->c = realloc(line->c, line->line_capacity);
  }
  /*
    While line_capacity is full then size*=2 and realloc 
    because 126 * 2 = 252 > line_capacity then run until satisfied condition
  */
  memcpy(&line->c[line->len], str, del_line_len);
  line->len += del_line_len;
  line->c[line->len] = '\0';

}

void RowDeletechar(Row *line, int pos){

  if (pos < 0 || pos >= line->len){
    return;
  }
  /*
  If x < 0 or x >= line's len then return
  It means The cursor moved out of its intended position
  */
  memmove(&line->c[pos], &line->c[pos+1], line->len - pos);
  line->len-=1;
  // memory move c[pos+1] -> c[pos]

}


void RowInsertchar(Row *line, char word, int pos){

  if (pos < 0 || pos > line->len){
    pos = line->len;
  }
  
  if (line->len + 1> line->line_capacity){
    line->line_capacity*=2;
    line->c = realloc(line->c, line->line_capacity);
  }
  
  // it seems like RowInsertString capacity*2
  memmove(&line->c[pos+1], &line->c[pos], line->len - pos + 1);
  // memory move line->len - pos + 1 size
  line->len += 1;
  line->c[pos] = word;

}

void empty_new_line(int pos){

  InsertRow(pos, "", 0);
  // If the line is empty or outside the screen add a empty line.

}

void Insertchar(char word){

  if(y == total) {
    empty_new_line(total); 
    // if cursor y = total then add line;
  }
  RowInsertchar(&Edit.line[y], word, x);
  x += 1;
  // Insert char at cursor x
}

void contained_new_line(Row *line, int pos_y, int pos_x) {

    InsertRow(pos_y + 1, &line->c[pos_x], line->len - pos_x);
    // Insert current line's string(pos_x to line->len) to new line
    line = &Edit.line[pos_y];
    line->len = pos_x;
    line->c[line->len] = '\0';

}

void Newline(){

  Row *line = get_line(Edit.line, y);
  // get line Edit.line[y]
  if(x == 0){
    empty_new_line(y);
  }
  else{
    contained_new_line(line, y, x);
  }
  y += 1;
  x = 0;
}

void Del_current_line_char() {

  Row *line = get_line(Edit.line, y);
  // get line Edit.line[y]
  RowDeletechar(line, x - 1);
  x -= 1;

}

void Del_current_line() {

  Row *line = get_line(Edit.line, y);
  // get line Edit.line[y]
  x = Edit.line[y - 1].len;
  RowInsertString(&Edit.line[y - 1], line->c, line->len);
  DeleteRow(y);
  y -= 1;
  // x cursor is prev line's len and y cursor -1 and insert string at line's len

}

void DeleteChar(){    // 수정 필요 백스페이스키 안먹는거 같음 !!

  Row *line = get_line(Edit.line, y);

  if( y == total){
    return;
  }
  if( x == 0 && y == 0){
    return;
  }

  if(x > 0){
    Del_current_line_char();
  }
  else{
    Del_current_line();

  }

}

void Visual_Text_editor__version(){
  char message[40];
  move(y,x);
  int len = snprintf(message, sizeof(message), "Visual Text editor -- version 0.0.1");
  int mid = (cols - len)/2;
  if (total == 0){
    mvprintw(rows/3, mid, "Visual Text editor -- version 0.0.1");
  }
  else {
    mvprintw(rows/3, mid, "                                   ");
  }
}

void status_bar() {
    char total_len[20];
    char st_y[20];
    int y_1 = y + 1;
    snprintf(total_len, sizeof(total_len), "%d", total);
    snprintf(st_y, sizeof(st_y), "%d", y);
    
    int left_len = strlen(total_len) + strlen(Edit.filename) + 13;
    int right_len = strlen(total_len) + strlen(st_y) + 11; // 9은 "no ft | "의 길이

    init_pair(2, COLOR_WHITE, COLOR_BLACK); // Define a color pair for reverse color
    attron(COLOR_PAIR(2) | A_REVERSE); // Enable the defined reverse color pair
    
    for (int i = left_len - 2; i < cols - right_len; i++){
      mvprintw(rows-2, i, " ");
      refresh();
    }

    // 왼쪽에 텍스트 출력
    mvprintw(rows - 2, 0, "[%s] - %d lines", Edit.filename, total);

    // 오른쪽에 텍스트 출력

    mvprintw(rows - 2, cols - right_len, "no ft | %d / %d", y + y_out + 1, total);

    attroff(COLOR_PAIR(2) | A_REVERSE); // Turn off the reverse color pair
}


void end_message(const char *format, ...) {
    va_list args;
    va_start(args, format);
    mvprintw(rows-1, 0, format, args); // 가변 인자들을 printf 형태로 특정 위치에 출력
    va_end(args);
    refresh();
}

void Move(int key) {
    curs_set(0);
    switch (key) {
        case KEY_LEFT:
            if (x != 0) {
                x -= 1;
            } else if (y > 0) {
                y -= 1;
                x = Edit.line[y].len;
            }
            break;
        case KEY_RIGHT:
            x += 1;
            break;
        case KEY_UP:
            if (y != 0) {
                y -= 1;
            }
            break;
        case KEY_DOWN:
            if(y == rows-2){
              if( total == y + y_out) {
                y = rows - 3;
                break;
              } 
              y_out += 1;
              flag = 1;
              y = rows - 3;
            }else{
              if (y < total) {
                y += 1;
              }
            }
            break;
    }
    move(y, x);
    refresh();
    curs_set(1);
}

void presskey() {

    int c = 0;
    
    if (c == 0){
      c = getch();
    }
    if (c < 32 || c > 126){
    switch (c) {
        case CONTROL('q'):
            clear();
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
            x = Edit.line[y].len;
            move(y,x);
            break;

        case KEY_HOME: // Home 키
            x = 0;
            move(y, x);
            break;

        case KEY_NPAGE: // Page Down 키
        case KEY_PPAGE: // Page Up 키
        {
            int temprows = rows;
            while (temprows--) {
                if (c == KEY_PPAGE)
                    Move(KEY_UP);
                else if (c == KEY_NPAGE)
                    Move(KEY_DOWN);
            }
        }
            break;
        // 이부분 해결해야 될듯
        case '\n':
            Newline();
            break;

        case KEY_BACKSPACE:
            DeleteChar();
            break;
      }
    }
    else{
      char ch = (char)c;
      Insertchar(ch);
    }
    
    refresh();
}

void clean_and_printing(){
   for(int i=0; i < rows-2; ++i){
    mvprintw(i, 0, "%*s", cols, "");
    if(Edit.line[i + y_out].c == NULL){
      mvprintw(i, 0, "~");
      continue;
    }
    else
    {
      mvprintw(i, 0, "%s", Edit.line[i + y_out].c);
    }
  }
}

// void scroll(){
//   if (x > cols){
//     move_cols += 1;
//   }
// }
void state() {
    clear();
    for (int i = 0; i < rows - 2; i++) {
        mvprintw(i, 0, "~");
    }
    refresh();
}

void all_refresh(){
  state();
  status_bar();
  end_message("Help: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F  = find");
  move(y,x);
  refresh();
}

int main(int argc, char *argv[]){
  initscr();
  raw();
  start_color();
  noecho();
  clear();
  cbreak();
  keypad(stdscr, TRUE);
  x = 0;
  y = 0;
  rows = 0;
  cols = 0;
  move_rows = 0;
  move_cols = 0;
  total = 0;
  getmaxyx(stdscr, rows, cols); // rows cols

  Edit.filename = NULL;

  if (Edit.filename == NULL){
    Edit.filename = "No Name";
  }
  else{
    Edit.filename = argv[1];
  }
  all_refresh();
  Visual_Text_editor__version();
  move(0,0); // 0, 0
  refresh(); // refresh();
  presskey();
  Visual_Text_editor__version();
  refresh();

  while(true){
    curs_set(0);
    clean_and_printing();
    status_bar();
    end_message("Help: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F  = find");
    move(y,x);
    refresh();
    curs_set(1);
    presskey();
  }

  endwin();
  return 0;

}
