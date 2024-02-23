OBJS_TCC    = main.o
OBJS_GCC_CLANG = maingcc.o
SOURCE_TCC  = main.c
SOURCE_GCC_CLANG = maingcc.c
OUT_TCC     = 0
OUT_GCC = gcc_output/0
OUT_CLANG = clang_output/0

# Compilers
CC_TCC  = tcc
CC_GCC  = gcc
CC_CLANG = clang

# Flags for all compilers
COMMON_FLAGS = -Os -fno-plt -fno-stack-protector -fno-stack-check -fno-unwind-tables -fno-asynchronous-unwind-tables -fomit-frame-pointer -ffast-math -fno-pic -fno-PIE -ffunction-sections -fdata-sections -fno-plt -fmerge-all-constants 

TCCFLAGS = $(COMMON_FLAGS) -mno-fancy-math-387 -mno-ieee-fp
GCCFLAGS = $(COMMON_FLAGS) -nostartfiles -nodefaultlibs -no-pie -mno-fancy-math-387 -mno-ieee-fp
CLANGFLAGS = $(COMMON_FLAGS) -nostartfiles -fno-pie 
# Linker flags
LFLAGS  = -lc -ldl -lm -lasound -lportaudio

.PHONY: all clean run

all: dirs tcc_compile gcc_compile clang_compile

dirs:
	@mkdir -p gcc_output
	@mkdir -p clang_output

tcc_compile: $(OBJS_TCC)
	@$(CC_TCC) -g $(OBJS_TCC) -o $(OUT_TCC) $(LFLAGS) $(TCCFLAGS)
	@strip -s -R .comment -R .gnu.version $(OUT_TCC)
	@sstrip $(OUT_TCC)
	@echo "TCC Uncompressed size:"
	@stat -c %s $(OUT_TCC)
	@./autovndh.py $(OUT_TCC) --zopfli --lzma --vndh_unibin --vndh_no_argv  --vndh "" > $(OUT_TCC)~
	@rm $(OUT_TCC) && mv $(OUT_TCC)~ $(OUT_TCC)
	@chmod +x $(OUT_TCC)
	@echo "TCC Compressed size:"
	@stat -c %s $(OUT_TCC)
	@echo "-----"

gcc_compile: $(OBJS_GCC_CLANG)
	@$(CC_GCC) -g $(OBJS_GCC_CLANG) -o $(OUT_GCC) $(LFLAGS) $(GCCFLAGS)
	@strip -s -R .comment -R .gnu.version $(OUT_GCC)
	@sstrip $(OUT_GCC)
	@echo "GCC Uncompressed size:"
	@stat -c %s $(OUT_GCC)
	@./autovndh.py $(OUT_GCC) --zopfli --lzma --vndh_unibin --vndh_no_argv  --vndh "" > $(OUT_GCC)~
	@rm $(OUT_GCC) && mv $(OUT_GCC)~ $(OUT_GCC)
	@chmod +x $(OUT_GCC)
	@echo "GCC Compressed size:"
	@stat -c %s $(OUT_GCC)
	@echo "-----"

clang_compile: $(OBJS_GCC_CLANG)
	@$(CC_CLANG) -g $(OBJS_GCC_CLANG) -o $(OUT_CLANG) $(LFLAGS) $(CLANGFLAGS) -fPIC
	@strip -s -R .comment -R .gnu.version $(OUT_CLANG)
	@sstrip $(OUT_CLANG)
	@echo "Clang Uncompressed size:"
	@stat -c %s $(OUT_CLANG)
	@./autovndh.py $(OUT_CLANG) --zopfli --lzma --vndh_unibin --vndh_no_argv  --vndh "" > $(OUT_CLANG)~
	@rm $(OUT_CLANG) && mv $(OUT_CLANG)~ $(OUT_CLANG)
	@chmod +x $(OUT_CLANG)
	@echo "Clang Compressed size:"
	@stat -c %s $(OUT_CLANG)
	@echo "-----"

main.o: main.c
	@$(CC_TCC) -g -c $(FLAGS) main.c

maingcc.o: maingcc.c
	@$(CC_GCC) -g -c $(FLAGS) maingcc.c
	@$(CC_CLANG) -g -c $(FLAGS) maingcc.c -fPIC

clean:
	rm -rf $(OBJS_TCC) $(OBJS_GCC_CLANG) gcc_output clang_output $(OUT_TCC) 0_tcc

run_tcc:
	./$(OUT_TCC)

run_gcc:
	./$(OUT_GCC)

run_clang:
	./$(OUT_CLANG)
