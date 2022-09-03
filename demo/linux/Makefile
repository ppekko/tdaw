OBJS	= main.o
SOURCE	= main.c
OUT	= 0
CC	 = clang
FLAGS	 = -nostartfiles -Wall -fomit-frame-pointer -fmerge-all-constants  -fno-math-errno -Wall -ffast-math  -fno-unroll-loops -Os -nodefaultlibs
LFLAGS	 = -lc -ldl -lm -lasound 


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) $(FLAGS)
	strip -s -R .comment -R .gnu.version $(OUT)
	sstrip $(OUT)
	./bettergz.sh $(OUT)
	@echo "Size of $(OUT):"
	@ls -l $(OUT) | awk '{print $$5}'
	

main.o: main.c
	$(CC) -g -c $(FLAGS) main.c

clean:
	rm -f $(OBJS) $(OUT) $(OUT)~

run: $(OUT)
	./$(OUT)
