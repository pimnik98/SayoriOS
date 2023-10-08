SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

CFLAGS=-march=native -O3 -Wextra -Wall -Wno-switch -std=c99 -I../../../include
LDFLAGS=

all: gameboy

debug: CFLAGS += -g -DDEBUG=1
debug: clean gameboy

gameboy: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o gameboy $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -flto $^ -c

clean:
	$(RM) -f gameboy gameboy.exe *.o
