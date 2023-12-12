CC = gcc
TARGET = vite
OBJS = origin.o

PDCURSES_DIR = ../vite/pdcurses

ifeq ($(OS),Windows_NT)
    # 윈도우용 PDCurses 라이브러리 및 헤더 파일 경로
    PDCURSES_INC = -I$(PDCURSES_DIR)
    PDCURSES_DLL = $(PDCURSES_DIR)/pdcurses.dll
    LIBS = -lm $(PDCURSES_DLL)
    CFLAGS = $(PDCURSES_INC)

    $(TARGET): $(OBJS)
	    $(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
else
    LIBS = -lm -lncurses

    $(TARGET): $(OBJS)
	    $(CC) -o $(TARGET) $(OBJS) $(LIBS)
endif

# 오브젝트 파일 생성 규칙
$(OBJS) : origin.c
	$(CC) $(CFLAGS) -c origin.c

# 정리 규칙
clean :
	rm -f $(OBJS) $(TARGET)
