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

#define _XOPEN_SOURCE 700

// #ifdef _WIN32
    
// #elif __linux__

#define BUFFER_SIZE 8000000

typedef struct Node {
    char *line;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct {
    char **lines;  // 각 라인을 저장하는 배열
    int capacity;
    int num_lines;  // 현재 라인 수
    int *line_lengths;  // 각 라인의 길이를 저장하는 배열
    int cursor_position_row; 
    int cursor_position_column;  
} TextBuffer;


typedef struct {
    int row;
    int column;
} CursorPosition;

typedef struct TextEditor {
    Node *head;
    Node *tail;
    Node *current;
    char *filename;
    int line_count;
    CursorPosition cursor_position; 
    TextBuffer text_buffer;
} TextEditor;


void state();
void handle_command(TextEditor *editor);
void initBuffer(TextBuffer *buffer);
void insertCharacter(TextBuffer *buffer, char character);
void deleteCharacter(TextBuffer *buffer);
void moveCursorToTop();
void moveCursorUp(TextBuffer *buffer);
void moveCursorDown(TextBuffer *buffer);
void moveCursorLeft(TextBuffer *buffer);
void moveCursorRight(TextBuffer *buffer);
void moveCursor(TextBuffer *buffer, int direction);
void print_status(const TextEditor *editor, int rows);
void end_message(const char *message, const TextEditor *editor, int rows);


int main() {
    initscr();
    noecho();  // 입력한 키를 화면에 표시하지 않음

    int rows, column;
    getmaxyx(stdscr, rows, column);  // 터미널 창의 행과 열 크기 가져오기 

    TextEditor editor;
    editor.line_count = 10;
    editor.filename = "No name";
    state();

    initBuffer(&editor.text_buffer);
    print_status(&editor, rows);
    
    end_message("Help: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F  = find", &editor, rows);
    moveCursorToTop();  // 이 부분을 추가하여 커서를 맨 위로 이동시킴

    while (1) {
        handle_command(&editor);  // 명령을 계속해서 처리
    }
    endwin();   // ncurses 종료
    return 0;
}


void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void moveCursorToTop() {
    gotoxy(1, 1);
}

void print_status(const TextEditor *editor, int rows) {
    gotoxy(1, rows - 1);
    printf("\e[7m [%s] - %d lines - Cursor: (%d, %d)", editor->filename, editor->text_buffer.num_lines, 
            editor->text_buffer.cursor_position_row, 
            editor->text_buffer.cursor_position_column);
    printf("\x1b[0m");
}



void state() {
    clear(); // 기존 내용을 지우고 새로 그림
    int columns = 80;
    int i = 0;
    for (i = 0; i < rows; i++) {
            mvprintw(i, 0, "~");
        
    }
}


void end_message(const char *message, const TextEditor *editor, int rows) {
    int msg_length = strlen(message);
    mvprintw(rows - 1, 0, "%s", message);
    print_status(editor, rows);
    refresh();
}// handle_command 함수 일부



void handle_command(TextEditor *editor) {
    int command = getch();

    if (command == '\n') {
        // Enter 키 입력 시, 문자열을 입력받고 텍스트 버퍼에 추가
        char input_buffer[BUFFER_SIZE];
        mvgetnstr(editor->text_buffer.cursor_position_row, editor->text_buffer.cursor_position_column, input_buffer, sizeof(input_buffer));

        int i = 0;
        for (i = 0; i < strlen(input_buffer); i++) {
            if (input_buffer[i] != '\n') {
                insertCharacter(&editor->text_buffer, input_buffer[i]);
                // 입력한 문자를 화면에 출력
                mvprintw(editor->text_buffer.cursor_position_row, editor->text_buffer.cursor_position_column, "%c", input_buffer[i]);
                // 커서를 다음 위치로 이동
                moveCursorRight(&editor->text_buffer);
            }
        }
    } else if (command == 127) {
        // 백스페이스 키 입력 시, 문자열에서 한 글자 삭제
        deleteCharacter(&editor->text_buffer);

        // 삭제 후 텍스트 버퍼의 내용을 화면에 업데이트
        int row = editor->text_buffer.cursor_position_row;
        int col = editor->text_buffer.cursor_position_column;

        // 백스페이스 입력 시 현재 위치에 공백 출력
        mvprintw(row, col, " ");

        // 커서 위치 조정
        move(row, col);
        refresh(); // 화면 갱신 추가
    } else {
        // 다른 키 입력 시, Insert mode에서의 처리
        insertCharacter(&editor->text_buffer, (char) command);

        // 입력한 문자를 화면에 출력
        mvprintw(editor->text_buffer.cursor_position_row, editor->text_buffer.cursor_position_column - 1, "%c", (char) command);
        moveCursorRight(&editor->text_buffer); // 커서를 다음 위치로 이동
        refresh(); // 화면 갱신 추가
    }
}



void initBuffer(TextBuffer *buffer) {
    buffer->capacity = BUFFER_SIZE;
    buffer->lines = (char **)malloc(buffer->capacity * sizeof(char *));
    buffer->line_lengths = (int *)malloc(buffer->capacity * sizeof(int));
    if (buffer->lines == NULL || buffer->line_lengths == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    buffer->num_lines = 0;
    buffer->cursor_position_row = 0;
    buffer->cursor_position_column = 0;

    // 초기에 25개의 라인을 생성
    /*int i =0;
    for (i = 0; i < 25; ++i) {
        insertCharacter(buffer, '\n');
    }*/
}



void deleteCharacter(TextBuffer *buffer) {
    if (buffer->cursor_position_column > 0) {
        char *line = buffer->lines[buffer->cursor_position_row];
        int length = buffer->line_lengths[buffer->cursor_position_row];

        // 커서 위치의 문자를 왼쪽으로 이동
        memmove(line + buffer->cursor_position_column - 1, line + buffer->cursor_position_column, length - buffer->cursor_position_column);

        // 라인 길이 갱신
        buffer->line_lengths[buffer->cursor_position_row]--;

        // 커서 위치 이동
        buffer->cursor_position_column--;

        // 만약 라인이 비어있다면 라인을 삭제
        if (buffer->line_lengths[buffer->cursor_position_row] == 0) {
            free(line);
            buffer->lines[buffer->cursor_position_row] = NULL;

            // 라인 수 갱신
            buffer->num_lines--;

            // 커서 위치 위의 라인을 현재 라인으로 복사
            if (buffer->cursor_position_row < buffer->num_lines) {
                buffer->lines[buffer->cursor_position_row] = buffer->lines[buffer->cursor_position_row + 1];
                buffer->line_lengths[buffer->cursor_position_row] = buffer->line_lengths[buffer->cursor_position_row + 1];
                buffer->lines[buffer->cursor_position_row + 1] = NULL;
                buffer->line_lengths[buffer->cursor_position_row + 1] = 0;
            }
        }
    }
}


void moveCursorUp(TextBuffer *buffer) {
    if (buffer->cursor_position_row > 0) {
        buffer->cursor_position_row--;
        int prev_line_length = buffer->line_lengths[buffer->cursor_position_row];
        if (buffer->cursor_position_column > prev_line_length) {
            // 이전 라인의 길이보다 커서 열이 크면 커서 열을 이전 라인의 끝으로 조정
            buffer->cursor_position_column = prev_line_length;
        }
    }
}

void moveCursorDown(TextBuffer *buffer) {
    if (buffer->cursor_position_row < buffer->num_lines - 1) {
        buffer->cursor_position_row++;
        int next_line_length = buffer->line_lengths[buffer->cursor_position_row];
        if (buffer->cursor_position_column > next_line_length) {
            // 다음 라인의 길이보다 커서 열이 크면 커서 열을 다음 라인의 끝으로 조정
            buffer->cursor_position_column = next_line_length;
        }
    }
}

void moveCursorLeft(TextBuffer *buffer) {
    if (buffer->cursor_position_column > 0) {
        buffer->cursor_position_column--;
    } else if (buffer->cursor_position_row > 0) {
        // 현재 라인의 맨 앞에 도달했으면 이전 라인으로 이동
        buffer->cursor_position_row--;
        buffer->cursor_position_column = buffer->line_lengths[buffer->cursor_position_row];
    }
}


void moveCursorRight(TextBuffer *buffer) {
    char *line = buffer->lines[buffer->cursor_position_row];
    int length = buffer->line_lengths[buffer->cursor_position_row];

    if (buffer->cursor_position_column < length) {
        buffer->cursor_position_column++;
    } else if (buffer->cursor_position_row < buffer->num_lines - 1) {
        // 현재 라인의 끝에 도달했으면 다음 라인으로 이동
        buffer->cursor_position_row++;
        buffer->cursor_position_column = 0;
    }
}


void moveCursor(TextBuffer *buffer, int direction) {
    switch (direction) {
        case 65:  // 위쪽 화살표 키의 ASCII 코드
            moveCursorUp(buffer);
            break;
        case 66:  // 아래쪽 화살표 키
            moveCursorDown(buffer);
            break;
        case 68:  // 왼쪽 화살표 키
            moveCursorLeft(buffer);
            break;
        case 67:  // 오른쪽 화살표 키
            moveCursorRight(buffer);
            break;
        default:
            // 다른 키에 대한 처리
            break;
    }
}

void insertCharacter(TextBuffer *buffer, char character) {
    if (buffer->num_lines == 0) {
        // 초기에 라인이 없을 때는 새로운 라인을 할당
        buffer->lines = (char **)malloc(sizeof(char *));
        buffer->line_lengths = (int *)malloc(sizeof(int));
        if (buffer->lines == NULL || buffer->line_lengths == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        buffer->lines[0] = (char *)malloc(sizeof(char) * 2);  // 초기 길이는 1
        if (buffer->lines[0] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        buffer->line_lengths[0] = 0;
        buffer->num_lines = 1;
    }

     if (buffer->cursor_position_row >= buffer->num_lines) {
        int new_line_count = buffer->num_lines + 1;  // 다음 라인까지 포함
        buffer->lines = (char **)realloc(buffer->lines, sizeof(char *) * new_line_count);
        buffer->line_lengths = (int *)realloc(buffer->line_lengths, sizeof(int) * new_line_count);
        if (buffer->lines == NULL || buffer->line_lengths == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        // 새로 추가된 라인을 초기화
        buffer->lines[buffer->num_lines] = (char *)malloc(sizeof(char));  // 초기 길이는 0
        if (buffer->lines[buffer->num_lines] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }
        buffer->lines[buffer->num_lines][0] = '\0';  // 빈 문자열로 초기화
        buffer->line_lengths[buffer->num_lines] = 0;

        buffer->num_lines = new_line_count;
    }
    // 커서 위치에 문자를 삽입
    char *line = buffer->lines[buffer->cursor_position_row];
    int length = buffer->line_lengths[buffer->cursor_position_row];

    if (length + 1 >= buffer->capacity) {
        buffer->capacity *= 2;
        line = (char *)realloc(line, buffer->capacity * sizeof(char));
        if (line == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        buffer->lines[buffer->cursor_position_row] = line;
    }

    // 커서 위치 뒤의 문자들을 오른쪽으로 이동
    memmove(line + buffer->cursor_position_column + 1, line + buffer->cursor_position_column,
            length - buffer->cursor_position_column);

    // 문자 삽입
    line[buffer->cursor_position_column] = character;
    buffer->line_lengths[buffer->cursor_position_row]++;
    buffer->cursor_position_column++;
}





/*
void saveFile(TextEditor *editor) {
   FILE *file = fopen(editor->filename, "w");
   if (file == NULL) {
          fprintf(stderr, "Error: Unable to open file for writing.\n");
          exit(EXIT_FAILURE);
   }

   Node *currentNode = editor->head;
   while (currentNode != NULL) {
      fprintf(file, "%s\n", currentNode->line);
      currentNode = currentNode->next;
   }

   fclose(file);
}

void handleSaveCommand(TextEditor *editor) {
   if (editor->head == NULL) {
      printf("Error: No content to save.\n");
      return;
   }

   if (editor->filename == NULL) {
      printf("Enter a new file name: ");
      char filename[256];
      scanf(" %s[^\n]", filename);
      editor->filename = strdup(filename);
   }

   saveFile(editor);

   editor->isModified = 0;

   displayStatusBar(editor);
   printf("File saved successfully.\n");
}
*/