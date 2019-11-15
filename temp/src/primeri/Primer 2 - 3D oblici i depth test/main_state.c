#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include <rafgl.h>

#include <game_constants.h>

/* ovaj put umesto trougla imamo cetvorostranu piramidu */

static GLfloat vertices[] =
{
    -1, -1, -1,     0, 1, 0,    /* z- */
     1, -1, -1,     1, 0, 0,
     0,  1,  0,     0, 0, 1,

    -1, -1,  1,     1, 0, 0,    /* z+ */
     1, -1,  1,     0, 1, 0,
     0,  1,  0,     0, 0, 1,

    -1, -1, -1,     0, 1, 0,    /* x- */
    -1, -1,  1,     1, 0, 0,
     0,  1,  0,     0, 0, 1,

     1, -1, -1,     1, 0, 0,    /* x+ */
     1, -1,  1,     0, 1, 0,
     0,  1,  0,     0, 0, 1,

    -1, -1, -1,     0, 1, 0,    /* dno 1. polovina */
    -1, -1,  1,     1, 0, 0,
     1, -1,  1,     0, 1, 0,

     1, -1,  1,     0, 1, 0,    /* dno 2. polovina */
     1, -1, -1,     1, 0, 0,
    -1, -1, -1,     0, 1, 0,
};

static GLuint vao, vbo, shader_program_id, uni_M, uni_VP;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{

    shader_program_id = rafgl_program_create_from_name("first_shader");
    uni_M = glGetUniformLocation(shader_program_id, "uni_M");
    uni_VP = glGetUniformLocation(shader_program_id, "uni_VP");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
                        /* 6 trouglova po 3 vertexa po 6 floatova */
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 3 * sizeof(GLfloat));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);


}


mat4_t model, view, projection, view_projection;

float fov = 90.0f;

vec3_t camera_position = vec3m(0.0f, 0.0f, -4.0f);
vec3_t camera_target = vec3m(0.0f, 0.0f, 0.0f);
vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{


    if(game_data->keys_down['W']) camera_position.z += 1.0f * delta_time;
    if(game_data->keys_down['S']) camera_position.z -= 1.0f * delta_time;

    if(game_data->keys_down['A']) camera_position.x += 1.0f * delta_time;
    if(game_data->keys_down['D']) camera_position.x -= 1.0f * delta_time;

    if(game_data->keys_down[RAFGL_KEY_SPACE]) camera_position.y += 1.0f * delta_time;
    if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) camera_position.y -= 1.0f * delta_time;

    if(game_data->keys_pressed[RAFGL_KEY_KP_ADD]) glEnable(GL_DEPTH_TEST);
    if(game_data->keys_pressed[RAFGL_KEY_KP_SUBTRACT]) glDisable(GL_DEPTH_TEST);

    float aspect = ((float)(game_data->raster_width)) / game_data->raster_height;    projection = m4_perspective(fov, aspect, 0.01f, 100.0f);

    view = m4_look_at(camera_position, camera_target, camera_up);

    model = m4_identity();

    view_projection = m4_mul(projection, view);



}


void main_state_render(GLFWwindow *window, void *args)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_program_id);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(vao);

    glUniformMatrix4fv(uni_M, 1, GL_FALSE, model.m);
    glUniformMatrix4fv(uni_VP, 1, GL_FALSE, view_projection.m);

    glDrawArrays(GL_TRIANGLES, 0, 6 * 3);

    glBindVertexArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteShader(shader_program_id);
}
