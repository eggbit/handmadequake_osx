CC = clang
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Werror -Wno-unused-parameter
IFLAGS = -I/usr/local/include/SDL2 -D_THREAD_SAFE
LFLAGS = -L/usr/local/lib -lSDL2

# Unit tests
TEST_IFLAGS = -I/src

all: handmadequake_osx

handmadequake_osx: src/main.c src/common.c src/sdl_helper.c
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) src/main.c src/common.c src/sdl_helper.c -o handmadequake_osx

tests: test/main.c common.o
	$(CC) $(CFLAGS) $(TEST_IFLAGS) test/main.c common.o -o tests

common.o: src/common.c
	$(CC) $(CFLAGS) $(TEST_IFLAGS) -c src/common.c

clean:
	rm -rf *.o *.dSYM handmadequake_osx tests
