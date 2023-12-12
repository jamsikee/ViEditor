CC = gcc
TARGET = vite
OBJS = origin.o

# OS 변수를 사용하여 운영체제 확인
OS := $(shell uname -s)

# 운영체제에 따라 라이브러리 선택
ifeq ($(OS),Windows_NT)
    # 윈도우용 PDCurses 라이브러리 경로
    # PDCurses 라이브러리 경로를 올바르게 설정하세요
    LIBS = -L/path/to/pdcurses/lib -lpdcurses
else
    LIBS = -lm -lncurses
endif

# 컴파일 타겟
$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

# 오브젝트 파일 생성 규칙
$(OBJS) : origin.c
	$(CC) -c -o $(OBJS) origin.c

# 정리 규칙
clean :
	rm -f $(OBJS) $(TARGET)
