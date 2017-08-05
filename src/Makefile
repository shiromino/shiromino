CFLAGS=-W -Wall -g3 -Xlinker -lm -lSDL2 -lSDL2_image -lSDL2_mixer -D_REENTRANT 
#above should include -m32 if forcing 32 bit

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: game

game: $(OBJ)
	gcc $(OBJ) $(CFLAGS) -o ../game

%.o: %.c
	gcc $^ -c -o $@ $(CFLAGS)

clean:
	rm -f *.o
