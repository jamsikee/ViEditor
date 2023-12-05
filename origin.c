#ifdef __WIN32
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <fcntl.h>
  #include <ctype.h>
  #include <stdbool.h>
  #define CLEAR "cls"
#elif __linux__
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

#elif __APPLE__

#endif

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

// gloval value
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
  // Store_Row_Information
} Row;

struct Visual_Text_Editor{

  int total;
  Row *line;
  char *store_file;
  // Editor Struct
};  

struct Visual_Text_Editor Edit;

typedef struct {

    char *temp;
  size_t size;
    int length;
    // Store_File_Information
} File_Inf;



void disRaw() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void Raw() {

    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    tcgetattr(STDIN_FILENO, &raw);

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); 
    // Non Sigint sign, change ctrl-M, Non INPCK, erase 8bit, ctrl-s, ctrl-q
    raw.c_oflag &= ~(OPOST); 
    // Non Output processing
    raw.c_cflag |= (CS8); 
    // Set 8 bit
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); 
    // Non canonical, Echo, ctrl-v, ctrl-c, ctrl-z
    raw.c_cc[VMIN] = 0;  
    // If input then return read( )
    raw.c_cc[VTIME] = 1; 
    // Maximum time before read( )

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    atexit(disRaw); 
    // If exit a program then automatically invoked

}

void for_quit(){

    system(CLEAR);
    write(STDOUT_FILENO, "\x1b[H", 3);  // cursor (0, 0)

}

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

        case '\r':  // Enter KEY
            Newline();
            break;

        case del:  // Delete KEY
            DeleteChar();
            Move(right);
            break;

        case b_s:  // Backspace KEY
            DeleteChar();
            break;

        default:  // Input( )
            Insertchar(key_val);
            break;
    }
}

void open_file(char *store_file) {
    free(Edit.store_file);
    Edit.store_file = malloc(strlen(store_file) + 1);
    strcpy(Edit.store_file, store_file);

    FILE *file = fopen(store_file, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", store_file);
        exit(EXIT_FAILURE);
    }

    File_Inf Inf;
    Inf.temp = NULL;
    Inf.size = 0;
    Inf.length = 0;

    while ((Inf.length = getline(&(Inf.temp), &(Inf.size), file)) != -1) {
        int read = Inf.length;
        while (Inf.length > 0 && (Inf.temp[Inf.length - 1] == '\r' || Inf.temp[Inf.length - 1] == '\n')) {
            Inf.length--;
        }
        InsertRow(Edit.total, Inf.temp, read);
    }

    free(Inf.temp);
    fclose(file);
}

void tilde(){
  for (int i; i < rows - 2; ++i){
    printf("~\r\n");
  }
}

void status_bar(char* file_name) {
    char left_Inf[50];
    char right_Inf[40];

    if (file_name == NULL || file_name[0] == '\0') {
        snprintf(left_Inf, sizeof(left_Inf), "[No Name] - %d lines", Edit.total);
    } else {
        snprintf(left_Inf, sizeof(left_Inf), "%.20s - %d lines", file_name, Edit.total);
    }

    int remained_len = cols - strlen(left_Inf) - strlen(right_Inf);
    snprintf(right_Inf, sizeof(right_Inf), "no ft / %d/%d", y + 1, Edit.total);

    printf("\x1b[%d;0H", rows - 2); // 상태바 위치로 커서 이동
    printf("\x1b[K"); // 해당 라인 지우기

}

void init() {
  
    Raw();
    initscr();
    getmaxyx(stdscr, rows, cols);
    x = 0;
    y = 0;
    Edit.total = 0;
    move_cols = 0;
    move_rows = 0;
    Edit.store_file = '\0';
}

void refresh(){
  tilde();
  status_bar(Edit.store_file);
}

int main(int argc, char *argv[]) {
  system(CLEAR);
  init();
  Edit.store_file = argv[1];
  if (argc >= 2) {
    open_file(argv[1]);
  }

  for (int i = 0; i < Edit.total; i++) {
    printf("%s\r",  Edit.line[i].c);
  }

  while (1) {
    presskey();
    refresh();
  }

  endwin();
  return 0;

}