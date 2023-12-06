// #ifdef __WIN32
//   #include <stdio.h>
//   #include <stdlib.h>
//   #include <string.h>
//   #include <fcntl.h>
//   #include <ctype.h>
//   #include <stdbool.h>
//   #define CLEAR "cls"
// #elif __linux__
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
  #define CLEAR "clear"

// #elif __APPLE__

// #endif

#define INIT_ROW_SIZE 500
#define INIT_LINE_SIZE 125
#define CONTROL(k) ((k) & 0x1f) // control + k

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
  int line_capacity;

} Row;

struct Visual_Text_Editor{

  int total;
  Row *line;
  char *filename;

};

struct Visual_Text_Editor Edit;


Row *get_line(Row *line, int pos) {

    return &line[pos];
    // get line index

}


void InsertRow(int edit_y, char *line, int line_len) {
  if (edit_y < 0) {
    return;
  }
  else if(edit_y > Edit.total){
    return;
  }
  // If y < 0 or y > total then return

  if (Edit.total == 0) {
    Edit.line = malloc(sizeof(Row) * INIT_ROW_SIZE);
  } else if (Edit.total % INIT_ROW_SIZE == 0) {
    Edit.line = realloc(Edit.line, sizeof(Row) * (Edit.total * 2));
  }
  /*
  Edit.line's memory = INIT_ROW_SIZE(1000)
  If total % 1000 == 0 then realloc 1000 * 2
  */
  memmove(&Edit.line[edit_y + 1], &Edit.line[edit_y], sizeof(Row) * (Edit.total - edit_y));
  // Memory move line[y] -> line[y + 1]
  Edit.line[edit_y].len = line_len;   
  Edit.line[edit_y].c = malloc(INIT_LINE_SIZE + 1);
  Edit.line[edit_y].line_capacity = INIT_LINE_SIZE + 1; 
  // Line_capacity is (Edit.line[y].c)'s size
  memcpy(Edit.line[edit_y].c, line, line_len);
  Edit.line[edit_y].c[line_len] = '\0'; 
  // The end of the string is null
  Edit.total+=1;

}

void FreeRow(Row *line){  // Efficient method for free memory

  free(line->c);

}

void DeleteRow(int pos){

  if (pos < 0){  
    return;
  }
  else if(pos >= Edit.total){
    return;
  }
  // If y < 0 or y > total then return

  FreeRow(&Edit.line[pos]); // Line[pos]'s memory free
  memmove(&Edit.line[pos], &Edit.line[pos+1], sizeof(Row) * (Edit.total - pos - 1));
  // Line[pos+1]'s memory move to free memory(line[pos]) 
  Edit.total-=1;

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

void RowInsertchar(Row *line, int word, int pos){

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

void Insertchar(int word){

  if(y == Edit.total) {
    empty_new_line(Edit.total); 
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

void DeleteChar(){

  Row *line = get_line(Edit.line, y);

  if( y == Edit.total){
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

void C_M(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void move_cursor_init(){
  C_M(1,1);
}

void status_bar(int rows) {
    C_M(1, rows - 1);
    printf("\e[7m [%s] - %d lines - Cursor: (%d, %d)", Edit.filename, Edit.total, 
            10, 
            10);
    printf("\x1b[0m");
}

void state() {
    int columns = 80;
    int i = 0;
    for (i = 0; i < rows; i++) {
        if (i == rows / 2 && i == 7) {
            int padding = (columns - strlen("visual text editor — version 0.0.1")) / 2;
            mvprintw(i, 0, "~%*cvisual text editor — version 0.0.1", padding, ' ');
        } else {
            mvprintw(i, 0, "~");
        }
        refresh(); // 화면 갱신
    }

}

void end_message(const char *message, int rows) {
    int msg_length = strlen(message);
    mvprintw(rows - 1, 0, "%s", message);
    print_status(rows);
    refresh();
}

int main() {
  system(CLEAR);
  initscr();
  noecho();
  getmaxyx(stdscr, rows, cols);
  x = 0;
  y = 0;
  Edit.total = 0;
  move_cols = 0;
  move_rows = 0;
  Edit.filename = "No Name";
  state();
  status_bar(rows);
  end_message(rows);
  move_cursor_init();
  // char* filename = argv[1];
  // if (argc >= 2) {
  //   open_file(argv[1]);
  // }

  // while (1) {
  //   presskey();
    
  // }

  endwin();
  return 0;

}