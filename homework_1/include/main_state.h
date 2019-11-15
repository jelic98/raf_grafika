#ifndef MAIN_STATE_H_INCLUDED
#define MAIN_STATE_H_INCLUDED

#include <GLFW/glfw3.h>
#include <rafgl.h>
#include <commands.h>

void main_state_init(GLFWwindow *window, void *args);
void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args);
void main_state_render(GLFWwindow *window, void *args);
void main_state_cleanup(GLFWwindow *window, void *args);

void args_parse(int, char**);
void commands_parse();
void image_init();
void image_update(rafgl_game_data_t*);
void image_reload();
void buttons_init();
void buttons_update(rafgl_game_data_t*);
void selector_update(rafgl_game_data_t*);

#endif // MAIN_STATE_H_INCLUDED
