#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef _WIN32
#include <curses.h>
#define BACKSPACE 8
#define ENTER 13
#elif __APPLE
#include <ncurses.h>
#define BACKSPACE 127
#define ENTER '\n'
#else
#include <ncurses.h>
#define BACKSPACE KEY_BACKSPACE
#define ENTER '\n'
#endif

#define CONTROL(k) ((k) & 0x1f) // control + k
#define INIT_ROW_SIZE 1000
#define INIT_LINE_SIZE 125
#define MAX_FILENAME 50
#define MAX_SEARCHNAME 20
#define Search_SIZE 10000

// global
int x = 0;
int y = 0;          // 1 최대 54
int cursor_out = 0; // +1
int rows = 0;       // 54
int cols = 0;
int total = 0;
int flag = 0;
int q_press = 0;

// total suruct
typedef struct Row
{

  int len;
  char *c;
  int line_capacity;

} Row;

struct Visual_Text_Editor
{

  Row *line;  // "*" is Dynamic memory 
  char *filename;
  char *store_file;
};

typedef struct
{

  char *temp;
  size_t size;
  int length;
  // Store_File_Information
} File_Inf;

struct Visual_Text_Editor Edit;

// total function
void get_windows_size();
Row *get_line(Row *line, int pos);
void insert_row(int edit_y, char *line, int line_len);
void free_row(Row *line);
void del_row(int pos);
void row_insert_remined(Row *line, char *str, size_t del_line_len);
void row_del_char(Row *line, int pos);
void row_insert_char(Row *line, char word, int pos);
void empty_new_line(int pos);
void insert_char(char word);
void del_current_line_char();
void del_current_line();
void delete_char();
void contained_new_line(Row *line, int pos_y, int pos_x);
void new_line(); // cursor_out + 1; line[y+cursor_out]
void status_bar();
void state();
void end_message(const char *format, ...);
void all_refresh();
void scroll_clean_and_printing(int pos);
void open_file(char *store_file);
void delete_clean_and_printing(int pos);
void get_filename(char *filename);
void get_searchname(char *search);

Row *get_line(Row *line, int pos)
{

  return &line[pos];
  // get line index
}

void insert_row(int edit_y, char *line, int line_len)
{
  if (edit_y < 0)
  {
    return;
  }
  else if (edit_y > total)
  {
    return;
  }
  // If y < 0 or y > total then return

  if (total == 0)
  {
    Edit.line = malloc(sizeof(Row) * INIT_ROW_SIZE);
  }
  else if (total % INIT_ROW_SIZE == 0)
  {
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
  Edit.line[edit_y].line_capacity = INIT_LINE_SIZE;
  // Line_capacity is (Edit.line[y].c)'s size = 125
  if (line_len > Edit.line[edit_y].line_capacity)
  {
    while (line_len > Edit.line[edit_y].line_capacity)
    {
      Edit.line[edit_y].line_capacity *= 2;
    }
    Edit.line[edit_y].c = realloc(Edit.line[edit_y].c, Edit.line[edit_y].line_capacity + 1);
    if (Edit.line[edit_y].c == NULL)
    {
      // error
      return;
    }
  }
  else
  {
    Edit.line[edit_y].c = malloc(Edit.line[edit_y].line_capacity + 1);
  }
  // realloc (125)*2
  memcpy(Edit.line[edit_y].c, line, line_len);
  Edit.line[edit_y].c[line_len] = '\0';
  // The end of the string is null
  total += 1;
}

void free_row(Row *line)
{ // Efficient method for free memory

  free(line->c);
}

void del_row(int pos)
{

  if (pos < 0)
  {
    return;
  }
  if (pos >= total)
  {
    return;
  }
  // If y < 0 or y > total then return

  free_row(&Edit.line[pos]);
  Edit.line[pos].c = NULL;
  Edit.line[pos].len = 0;
  Edit.line[pos].line_capacity = 0;
  // Line[pos]'s memory free
  for (int i = pos; i < total; ++i)
  {
    Edit.line[i] = Edit.line[i + 1];
    Edit.line[i].c = Edit.line[i + 1].c;
    Edit.line[i].len = Edit.line[i + 1].len;
    Edit.line[i].line_capacity = Edit.line[i + 1].line_capacity;
  }
  // Line[pos+1]'s memory move to free memory(line[pos])
  total -= 1;
}

void row_insert_remined(Row *line, char *str, size_t del_line_len)
{

  // This function will use delete char at x = 0 then delete row
  while (line->len + del_line_len > line->line_capacity)
  {
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

void row_del_char(Row *line, int pos)
{

  if (pos < 0 || pos >= line->len)
  {
    return;
  }
  /*
  If x < 0 or x >= line's len then return
  It means The cursor moved out of its intended position
  */
  memmove(&line->c[pos], &line->c[pos + 1], line->len - pos);
  line->len -= 1;
  // memory move c[pos+1] -> c[pos]
}
// RowInserchar need

void row_insert_char(Row *line, char word, int pos)
{

  if (pos < 0 || pos > line->len)
  {
    pos = line->len;
  }

  if (line->len + 1 > line->line_capacity)
  {
    line->line_capacity *= 2;
    line->c = realloc(line->c, line->line_capacity);
  }
  // it seems like row_insert_char capacity*2
  memmove(&line->c[pos + 1], &line->c[pos], line->len - pos + 1);
  // memory move line->len - pos + 1 size
  line->len += 1;
  line->c[pos] = word;
  // pos = x
}

void empty_new_line(int pos)
{

  insert_row(pos, "", 0);
  // If the line is empty or outside the screen add a empty line.
}

void insert_char(char word)
{

  if (y + cursor_out == total)
  {
    empty_new_line(total);
    // if cursor y = total then add line;
  }
  row_insert_char(&Edit.line[y + cursor_out], word, x);
  x += 1;
  // Insert char at cursor x
  scroll_clean_and_printing(y);
}

void contained_new_line(Row *line, int pos_y, int pos_x)
{

  insert_row(pos_y + 1, &line->c[pos_x], line->len - pos_x);
  // Insert current line's string(pos_x to line->len) to new line
  line = &Edit.line[pos_y];
  line->len = pos_x;
  line->c[line->len] = '\0';
}

void new_line()
{

  Row *line = get_line(Edit.line, y + cursor_out);
  // get line Edit.line[y]
  if (x == 0)
  {
    empty_new_line(y + cursor_out);
    if (y == rows - 3)
    {
      y = rows - 3;
      cursor_out += 1;
    }
    else
    {
      y += 1;
    }
    x = 0;
    // scroll if y > screen_rows then cursor out += 1;
    if (y == rows - 3)
    {
      scroll_clean_and_printing(0);
      // if y == rows - 3 then scroll
    }
    else
    {
      scroll_clean_and_printing(y - 1);
      // if y == !rows-3 then no scroll and cursor y-1 ~ rows-3 clear and printing
    }
  }
  else
  {
    contained_new_line(line, y + cursor_out, x);
    if (y == rows - 3)
    {
      y = rows - 3;
      cursor_out += 1;
    }
    else
    {
      y += 1;
    }
    // scroll plus
    x = 0;
    if (y == rows - 3)
    {
      scroll_clean_and_printing(0);
      // if y == rows - 3 then scroll
    }
    else
    {
      scroll_clean_and_printing(y - 1);
      // if y == !rows-3 then no scroll and cursor y-1 to rows-3 clear and printing
    }
  }
}

void del_current_line_char()
{

  Row *line = get_line(Edit.line, y + cursor_out);
  // get line Edit.line[y]
  row_del_char(line, x - 1);
  x -= 1;
}

void del_current_line()
{

  Row *line = get_line(Edit.line, y + cursor_out);
  // get line Edit.line[y]
  x = Edit.line[y - 1 + cursor_out].len;
  row_insert_remined(&Edit.line[y - 1 + cursor_out], line->c, line->len);
  del_row(y + cursor_out);
  // if current row is delete then remained string positioning y-1 line's len
  if (y == 0 && cursor_out > 0)
  {
    y = 0;
    cursor_out -= 1;
    // scroll
  }
  else
  {
    y -= 1;
  }
  // x cursor is prev line's len and y cursor -1 and insert string at line's len
}

void delete_char()
{ // 수정 필요 백스페이스키 안먹는거 같음 !!

  Row *line = get_line(Edit.line, y);

  if (y + cursor_out == total)
  {
    return;
  }
  if (x == 0 && y == 0 && cursor_out == 0)
  {
    return;
  }
  // return 0, 0, 0

  if (x > 0)
  {
    del_current_line_char();
    scroll_clean_and_printing(y);
  }
  else
  {
    del_current_line();
    scroll_clean_and_printing(y);
  }
  // printing y to rows-3
}

void Visual_Text_editor__version()
{
  char message[40];
  move(y, x);
  int len = snprintf(message, sizeof(message), "Visual Text editor -- version 0.0.1");
  int mid = (cols - len) / 2;
  if (total == 0)
  {
    mvprintw(rows / 3, mid, "Visual Text editor -- version 0.0.1");
  }
  else
  {
    mvprintw(rows / 3, mid, "                                   ");
  }
  // if total == 0 print else NULL
}

void status_bar()
{
  char total_len[20];
  char st_y[20];
  int y_1 = y + 1;
  snprintf(total_len, sizeof(total_len), "%d", total);
  snprintf(st_y, sizeof(st_y), "%d", y + cursor_out + 1);

  int left_len = strlen(total_len) + strlen(Edit.filename) + 13; // 13 is [] spacing - spacing lines len
  int right_len = strlen(total_len) + strlen(st_y) + 11;         // 11 is "no ft | "'s len

  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  // white and black
  attron(COLOR_PAIR(2) | A_REVERSE);
  // reverse
  if (flag == 1)
  {
    left_len += 11;
  }
  // if modified then len += 11 becuase [spacing and (modified)] is 11
  for (int i = left_len - 2; i < cols - right_len; i++)
  {
    mvprintw(rows - 2, i, " ");
    refresh();
  }
  // status_bar clean
  if (flag == 1)
  {
    mvprintw(rows - 2, 0, "[%s] - %d lines (modified)", Edit.filename, total);
    // if modified print modified
  }
  else
  {
    mvprintw(rows - 2, 0, "[%s] - %d lines", Edit.filename, total);
    // else do not print modified
  }

  mvprintw(rows - 2, cols - right_len, "no ft | %d / %d", y + cursor_out + 1, total);
  // no ft and y+cursor_out +1 and total

  attroff(COLOR_PAIR(2) | A_REVERSE);
  // turn off reverse
}

void end_message(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  mvprintw(rows - 1, 0, "%*s", cols, "");
  mvprintw(rows - 1, 0, format, args);
  va_end(args);
  refresh();
  // end message like "Help:"
}

void Move(int key)
{
  curs_set(0);
  switch (key)
  {
  case KEY_LEFT:
    if (x != 0)
    {
      x -= 1;
    }
    else if (y > 0)
    {
      y -= 1;
      x = Edit.line[y + cursor_out].len;
      // x is -y's len
    }
    else if (x == 0 && y == 0)
    {
      if (cursor_out > 0)
      {
        y = 0;
        cursor_out -= 1;
        x = Edit.line[y + cursor_out].len;
        // scroll
      }
    }
    move(y, x);
    break;
  case KEY_RIGHT:
    if (Edit.line[y].c == NULL)
    {
      break;
    }
    else
    {
      if (x < Edit.line[y + cursor_out].len)
      {
        x += 1;
      }
      else if (x == Edit.line[y + cursor_out].len)
      {
        if (y == rows - 3 && y + cursor_out < total)
        {
          cursor_out += 1;
          y = rows - 3;
          x = 0;
          // scroll
        }
        else
        {
          if (y != rows - 3)
          {
            y += 1;
            x = 0;
            // no scroll
          }
          else
          {
            y = rows - 3;
            // y doesn't go out rows - 3
          }
        }
      }
    }
    move(y, x);
    break;
  case KEY_UP:
    if (y != 0)
    {
      y -= 1;
    }
    else if (y == 0 && cursor_out > 0)
    {
      cursor_out -= 1;
      // scroll
    }
    if (x > Edit.line[y + cursor_out].len)
    {
      x = Edit.line[y + cursor_out].len;
      // if arrow up than if line's len is shorter than line+1's len move cursor
    }
    move(y, x);
    break;
  case KEY_DOWN:
    if (y == rows - 3)
    {
      if (total == y + cursor_out)
      {
        y = rows - 3;
        break;
      }
      cursor_out += 1;
      y = rows - 3;
      // scroll and y doesn't go out
    }
    else
    {
      if (y < total)
      {
        y += 1;
      }
    }
    if (x > Edit.line[y + cursor_out].len)
    {
      x = Edit.line[y + cursor_out].len;
      // // if arrow down than if line's len is shorter than line-1's len move cursor
    }
    move(y, x);
    break;
  }
  curs_set(1);
}

// file
void open_file(char *store_file)
{
  free(Edit.store_file);
  Edit.store_file = malloc(strlen(store_file) + 1);
  strcpy(Edit.store_file, store_file);
  // copy store_file to Edit.store_file

  FILE *file = fopen(Edit.store_file, "rt"); // read text
  if (!file)
  {
    fprintf(stderr, "no file");
    exit(EXIT_FAILURE);
  }

  File_Inf Inf; // init file structure
  Inf.temp = NULL;
  Inf.size = 0;
  Inf.length = 0;

  while ((Inf.length = getline(&(Inf.temp), &(Inf.size), file)) != -1) // get line's string
  {
    while (Inf.length > 0 && (Inf.temp[Inf.length - 1] == '\r' || Inf.temp[Inf.length - 1] == '\n'))
    { // if '\r' or '\n' then len - 1
      Inf.length -= 1;
    }
    int read = Inf.length;
    insert_row(total, Inf.temp, read); // insert my structure
  }

  free(Inf.temp);
  fclose(file);
  y = 0;
  cursor_out = total - (rows - 2); // screen rows + cursor_out = total
  if (cursor_out < 0)
    cursor_out = 0;
}

void save_file(char *filename)
{
  FILE *file = fopen(filename, "wt"); // write text to file

  for (int i = 0; i < total; ++i)
  {
    if (Edit.line[i].c != NULL)
    {
      fprintf(file, "%s\n", Edit.line[i].c); // Edit.line[i].c fprintf
    }
    else
    {
      fprintf(file, "\n"); // if line is null then '\n'
    }
  }

  fclose(file);
}

void get_filename(char *filename)
{
  int ch, pos = 0;
  mvprintw(rows - 1, 0, "%*s", cols, "");      // rows - 1 clear
  mvprintw(rows - 1, 0, "ENTER FILE NAME : "); // rows - 1 write name
  while (1)
  {
    ch = getch();
    if (ch == ENTER)
    {
      break;
    }
    else if (ch == BACKSPACE)
    {
      if (pos > 0)
      {
        pos -= 1;
        filename[pos] = '\0';
      }
    }
    else if (ch >= 32 && ch <= 126)
    { // 문자 및 숫자만 받기
      if (pos < MAX_FILENAME - 1)
      { // filename's max size is 50
        filename[pos++] = ch;
        filename[pos] = '\0';
      }
    }

    mvprintw(rows - 1, 0, "%*s", cols, "");
    mvprintw(rows - 1, 0, "ENTER FILE NAME : %s", filename);
    refresh();
  }
}

/*
void get_searchname(char *Query)
{
  int ch, pos = 0;
  mvprintw(rows - 1, 0, "%*s", cols, ""); // clear
  mvprintw(rows - 1, 0, "Search   (ESC/Arrows/Enter)");
  while ((ch = getch()) != '\n')
  {
    if (ch == BACKSPACE)
    {
      if (pos > 0)
      {
        pos -= 1;
        Query[pos] = '\0';
      }
    }
    else
    {
      if (pos < MAX_SEARCHNAME - 1) // Query's max size is 20
      {
        Query[pos++] = ch;
        Query[pos] = '\0';
      }
    }
    mvprintw(rows - 1, 0, "%*s", cols, "");
    mvprintw(rows - 1, 0, "Search  %s (ESC/Arrows/Enter)", Query);
    refresh();
  }
}
*/

typedef struct
{
  int s_y;
  int s_x;
  int s_out;
} SearchPosition;

SearchPosition *s_pos;  // Dynamic memory

int s_total = 0;
int s_now = 0;
int s_size = 0;

void search_text(char *Query)
{
  s_total = 0;
  s_now = 0;
  s_size = Search_SIZE;
  s_pos = malloc(sizeof(SearchPosition) * s_size); // first malloc 10000

  for (int i = 0; i < total; ++i)
  {
    char *data = Edit.line[i].c;
    char *sub = strstr(data, Query);  // strstr line[i].c to Query

    while (sub)
    {
      if (s_total >= s_size)
      {
        s_size *= 2;
        s_pos = realloc(s_pos, sizeof(SearchPosition) * s_size); // realloc *2
      }
      s_pos[s_total].s_x = sub - data;  // s_x = data's head
      if (i > rows - 3)
      {
        s_pos[s_total].s_out = i - (rows - 2);  // s_out = scroll
        s_pos[s_total].s_y = 0;                 // if s_out > 0 then s_y = 0
      }
      else if (i >= 0 && i <= rows - 3)
      {
        s_pos[s_total].s_out = 0;               // if s_out = 0
        s_pos[s_total].s_y = i;                 // s_y = i -> no scroll
      }
      s_total += 1;
      sub = strstr(sub + 1, Query);             // if line[y].c has more substring then while 
    }
  }
}

// 화면 상의 커서는 옮겨 졌지만 데이터 상의 커서가 안옮겨짐
void presskey()
{
  // flag = 1 is modified
  int c = 0;

  if (c == 0)
  {
    c = getch();
  }
  if (c < 32 || c > 126)
  {
    switch (c)
    {
    case CONTROL('q'):
      if (flag == 1)
      {
        q_press += 1; // if modified press q
        if (q_press == 2)
        {
          clear();  // clear ncurses creen
          endwin(); // turn off encurses
          exit(0);  // exit
        }
      }
      else
      {
        clear();
        endwin();
        exit(0);
      }
      break;

    case CONTROL('s'):
    {
      if (flag == 1)
      {
        char filename[MAX_FILENAME + 1];
        get_filename(filename);
        Edit.filename = malloc(strlen(filename) + 1);
        strcpy(Edit.filename, filename);
        save_file(filename);
        mvprintw(rows - 1, 0, "%*s", cols, "");
        q_press = 0; // if save then init q_press
      }
      flag = 0; // no modified
    }
    break;
    /*
    case CONTROL('f'):
    {
      char Sub[MAX_SEARCHNAME + 1] = {0};
      int s_c = 0;
      
      mvprintw(rows - 1, 0, "%*s", cols, ""); // clear
      mvprintw(rows - 1, 0, "Search   (ESC/Arrows/Enter)");
      while (1)
      {
        s_c = getch();
        if(s_c == ENTER){
          break;
        }
        if(s_c == 27){
          break;
        }
        if(s_c == BACKSPACE){
          if (strlen(Sub) > 0) {

          }
        }
      }
      get_searchname(Sub);
    }
    break;
    */

    case KEY_LEFT:  // arrow_left
    case KEY_RIGHT: // arrow_right
    case KEY_UP:    // arrow_up
    case KEY_DOWN:  // arrow_down

      if (total == 0)
        return;
      Move(c);
      scroll_clean_and_printing(0);
      break;
    case KEY_END: // end
      if (total == 0)
        return;
      x = Edit.line[y + cursor_out].len;
      move(y, x);
      break;

    case KEY_HOME: // home
      if (total == 0)
        return;
      x = 0;
      move(y, x);
      break;

    case KEY_NPAGE: // pgUp
    case KEY_PPAGE: // pgDn
    {
      if (total == 0)
        return;
      int temprows = rows - 3;
      while (temprows--)
      {
        if (c == KEY_PPAGE)
          Move(KEY_UP);
        else if (c == KEY_NPAGE)
          Move(KEY_DOWN);
      }
      scroll_clean_and_printing(0);
    }
    // pgUp and pgDn imitates visual studio code
    break;
    // 이부분 해결해야 될듯
    case ENTER:
      new_line();
      flag = 1;
      break;

    case BACKSPACE:
      delete_char();
      flag = 1;
      break;
    }
  }
  else
  {
    char ch = (char)c;
    insert_char(ch);
    flag = 1;
  }

  refresh();
}

void scroll_clean_and_printing(int pos)
{
  for (int i = pos; i < rows - 2; ++i)
  {
    mvprintw(i, 0, "%*s", cols, "");
  }
  // pos to rows - 3 clear
  for (int i = 0; i < rows - 2; ++i)
  {
    if (Edit.line[i + cursor_out].c == NULL)
    {
      mvprintw(i, 0, "%*s", cols, "");
      mvprintw(i, 0, "~");
      continue;
      // if null print ~ and continue it like vim
    }
    else
    {
      mvprintw(i, 0, "%s", Edit.line[i + cursor_out].c);
      // if cursor_out is zero then line[y].c
      // if corsor_out is 1 or upper then line[y + cursor_out].c it likes scroll
    }
  }
}

void state()
{
  clear();
  for (int i = 0; i < rows - 2; i++)
  {
    mvprintw(i, 0, "~");
  }
  refresh();
}

void all_refresh()
{
  state();
  status_bar();
  end_message("Help: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F  = find");
  move(y, x);
  refresh();
}

int main(int argc, char *argv[])
{
  initscr();            // ncurses and curses init screen
  raw();                // raw mode
  start_color();        // can reverse color
  noecho();             // no echo
  clear();              // clear ncurses and curses screen
  cbreak();             // no line break
  keypad(stdscr, TRUE); // can like special key like KEY_END
  x = 0;
  y = 0;          // 1 최대 54
  cursor_out = 0; // +1
  rows = 0;       // 54
  cols = 0;
  total = 0;
  flag = 0;
  q_press = 0;

  getmaxyx(stdscr, rows, cols); // get screen rows and cols

  if (argc >= 2)
  {
    Edit.filename = argv[1];
    open_file(argv[1]);
    end_message("Help: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F  = find");
    status_bar();
    move(y, x);
    scroll_clean_and_printing(0);
    refresh();
  }
  else
  {
    Edit.filename = "No Name";
    all_refresh();
    Visual_Text_editor__version();
    move(0, 0);
    refresh();
  }

  while (true)
  {
    curs_set(0);  // curs_set(0) = hide cursor
    status_bar(); // status_bar update
    if (q_press == 1)
    {
      end_message("Warning!!! File has unsaved changes. Press Ctrl-Q 1 more times to quit.");
      // if editor is modified then printing
    }
    move(y, x);                             // move cursor
    refresh();                              // refresh
    curs_set(1);                            // curs_set(1) = show cursor
    presskey();                             // getch()
    mvprintw(rows - 1, 0, "%*s", cols, ""); // clear message bar
  }

  endwin();             // turn off ncurses
  Edit.filename = NULL; // init filename
  return 0;             // return 0
}