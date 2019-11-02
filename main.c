#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define RAFGL_IMPLEMENTATION
#include <rafgl.h>

#include <main_state.h>
#include <commands.h>

int main(int argc, char *argv[]) {
	args_parse(argc, argv);

	rafgl_game_t game;
    
	rafgl_game_init(&game, "main", raster_width = 700, raster_height = 700, 0);
    rafgl_game_add_named_game_state(&game, main_state);
    rafgl_game_start(&game, NULL);

    return 0;
}
