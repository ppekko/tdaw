OBJS	= main.o
SOURCE	= main.c
OUT	= 0
CC	 = gcc
FLAGS	 = -Os -nostartfiles -nodefaultlibs -fno-plt -fno-stack-protector -fno-stack-check -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -ffast-math -no-pie -fno-pic -fno-PIE -ffunction-sections -fdata-sections -fno-plt -fmerge-all-constants -mno-fancy-math-387 -mno-ieee-fp
LFLAGS	 = -lc -ldl -lm -lasound 


all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS) $(FLAGS)
	strip -s -R .comment -R .gnu.version $(OUT)
	sstrip $(OUT)
	./autovndh.py $(OUT) --zopfli --lzma --vndh_unibin --vndh_no_argv  --vndh "" > $(OUT)~
	rm $(OUT) && mv $(OUT)~ $(OUT)
	

main.o: main.c
	$(CC) -g -c $(FLAGS) main.c

clean:
	rm -f $(OBJS) $(OUT) $(OUT)~

run: $(OUT)
	./$(OUT)
