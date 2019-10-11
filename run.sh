gcc main.c src/main_state.c src/glad/glad.c -lglfw -ldl -lm -I. -I./include -o main.out -DGLFW_INCLUDE_NONE && ./main.out
