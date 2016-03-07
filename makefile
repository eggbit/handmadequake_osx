CC = clang
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers
IFLAGS = -I/usr/local/include/SDL2 -D_THREAD_SAFE
LFLAGS = -L/usr/local/lib -lSDL2
FILES = src/sys_sdl.c src/common.c src/vid_sdl.c src/host.c src/file.c

all: handmadequake_osx

handmadequake_osx: $(FILES)
	@$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $(FILES) -o handmadequake_osx

tests: test/main.c common.o
	@$(CC) $(CFLAGS) $(IFLAGS) test/main.c common.o -o tests

common.o: src/common.c
	@$(CC) $(CFLAGS) $(IFLAGS) -c src/common.c

clean:
	rm -rf *.o *.dSYM handmadequake_osx tests
