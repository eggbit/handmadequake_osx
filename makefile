CC = clang
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Werror -Wno-missing-field-initializers
IFLAGS = $(shell sdl2-config --cflags)
LFLAGS = $(shell sdl2-config --libs)
FILES = src/sys_sdl.c src/common.c src/vid_sdl.c src/host.c src/draw.c src/pak.c src/wad.c src/zone.c

all: handmadequake_osx

handmadequake_osx: $(FILES)
	@$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $(FILES) -o handmadequake_osx

tests: test/main.c common.o
	@$(CC) $(CFLAGS) $(IFLAGS) test/main.c common.o -o tests

common.o: src/common.c
	@$(CC) $(CFLAGS) $(IFLAGS) -c src/common.c

clean:
	rm -rf *.o *.dSYM handmadequake_osx tests
