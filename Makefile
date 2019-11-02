CC = gcc
IN = main.c src/main_state.c src/commands.c src/glad/glad.c
OUT = main.out
CFLAGS = -Wall -DGLFW_INCLUDE_NONE
LFLAGS = -lglfw -ldl -lm
IFLAGS = -I. -I./include

.SILENT all: clean build run

clean:
	rm -f $(OUT)

build: $(IN) include/main_state.h include/commands.h include/stb_image.h 
	$(CC) $(IN) -o $(OUT) $(CFLAGS) $(LFLAGS) $(IFLAGS)

run: $(OUT)
	./$(OUT)
