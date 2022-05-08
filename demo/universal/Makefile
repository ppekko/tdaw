OBJS	= main.o
SOURCE	= main.c
OUT	= 0
CC	 = gcc
FLAGS	 = -nostartfiles -e _start -Wall -fomit-frame-pointer -mfpmath=387 -mfancy-math-387 -fmerge-all-constants -fsingle-precision-constant  -fno-math-errno -Wall -ffast-math  -fno-unroll-loops -Os -nodefaultlibs
LFLAGS	 = -lc -ldl -lportaudio -lm


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) $(FLAGS)


main.o: main.c
	$(CC) -g -c $(FLAGS) main.c -lcuni


clean:
	rm -f $(OBJS) $(OUT) $(OUT)~

run: $(OUT)
	./$(OUT)
