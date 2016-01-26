CC = clang
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Werror -Wno-unused-parameter
IFLAGS = -I/usr/local/include/SDL2 -D_THREAD_SAFE
LFLAGS = -L/usr/local/lib -lSDL2
FILES = src/main.c src/common.c src/sdl_helper.c

all: handmadequake_osx

handmadequake_osx: $(FILES)
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $(FILES) -o handmadequake_osx

tests: test/main.c common.o
	$(CC) $(CFLAGS) $(IFLAGS) test/main.c common.o -o tests

common.o: src/common.c
	$(CC) $(CFLAGS) $(IFLAGS) -c src/common.c

clean:
	rm -rf *.o *.dSYM handmadequake_osx tests
