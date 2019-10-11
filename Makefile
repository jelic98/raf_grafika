CC = gcc
IN = main.c src/main_state.c src/glad/glad.c
OUT = main.o
LIB = -lglfw
PATH = -I. -I./include

.SILENT all: clean build run

clean:
	rm -f $(OUT)

build: $(IN)
	$(CC) $(IN) -o $(OUT) $(LIB) $(PATH) -DGLFW_INCLUDE_NONE

run: $(OUT)
	./$(OUT)
