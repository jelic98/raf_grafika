#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include <rafgl.h>

#include <game_constants.h>


static GLfloat pyramid_vertices[] =
{
    -1, -1, -1,     0, 0.5, 0,    /* z- */
     1, -1, -1,     0.5, 0, 0,
     0,  1,  0,     0, 0, 0.8,

    -1, -1,  1,     0.5, 0, 0,    /* z+ */
     1, -1,  1,     0, 0.5, 0,
     0,  1,  0,     0, 0, 0.8,

    -1, -1, -1,     0, 0.5, 0,    /* x- */
    -1, -1,  1,     0.5, 0, 0,
     0,  1,  0,     0, 0, 0.8,

     1, -1, -1,     0.5, 0, 0,    /* x+ */
     1, -1,  1,     0, 0.5, 0,
     0,  1,  0,     0, 0, 0.8,

    -1, -1, -1,     0, 0.5, 0,    /* dno 1. polovina */
    -1, -1,  1,     0.5, 0, 0,
     1, -1,  1,     0, 0.5, 0,

     1, -1,  1,     0, 0.5, 0,    /* dno 2. polovina */
     1, -1, -1,     0.5, 0, 0,
    -1, -1, -1,     0, 0.5, 0,
};

static GLuint pyramid_vao, pyramid_vbo;
static GLuint grid_vao, grid_vbo;
static GLuint shader_program_id, uni_M, uni_VP;

int gridw, gridh;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{

    shader_program_id = rafgl_program_create_from_name("first_shader");
    uni_M = glGetUniformLocation(shader_program_id, "uni_M");
    uni_VP = glGetUniformLocation(shader_program_id, "uni_VP");

    glGenVertexArrays(1, &pyramid_vao);
    glGenBuffers(1, &pyramid_vbo);

    glBindVertexArray(pyramid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, pyramid_vbo);
                        /* 6 trouglova po 3 vertexa po 6 floatova */
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * 6 * sizeof(GLfloat), pyramid_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 3 * sizeof(GLfloat));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* generisanje grida */

    int x, y, v = 0;
    gridw = 10;
    gridh = 10;
    vec3_t *grid_vertices = malloc(gridw * gridh * 2 * 3 * 2 * sizeof(vec3_t));

    vec3_t gray20 = vec3(0.2f, 0.2f, 0.2f);
    vec3_t gray30 = vec3(0.3f, 0.3f, 0.3f);
    vec3_t gray40 = vec3(0.4f, 0.4f, 0.4f);

    vec3_t gray60 = vec3(0.6f, 0.6f, 0.6f);
    vec3_t gray70 = vec3(0.7f, 0.7f, 0.7f);
    vec3_t gray80 = vec3(0.8f, 0.8f, 0.8f);

    for(y = 0; y < gridh; y++)
    {
        for(x = 0; x < gridw; x++, v += 2 * 3 * 2)
        {

            grid_vertices[v +  0] = vec3(-0.5f * gridw + x       , -2.0f, -0.5f * gridh + y       );
            grid_vertices[v +  2] = vec3(-0.5f * gridw + x + 1.0f, -2.0f, -0.5f * gridh + y       );
            grid_vertices[v +  4] = vec3(-0.5f * gridw + x       , -2.0f, -0.5f * gridh + y + 1.0f);

            grid_vertices[v +  6] = vec3(-0.5f * gridw + x + 1.0f, -2.0f, -0.5f * gridh + y       );
            grid_vertices[v +  8] = vec3(-0.5f * gridw + x + 1.0f, -2.0f, -0.5f * gridh + y + 1.0f);
            grid_vertices[v + 10] = vec3(-0.5f * gridw + x       , -2.0f, -0.5f * gridh + y + 1.0f);

            if((x + y) % 2 == 0)
            {
                /* svetlije polje */
                grid_vertices[v +  1] = gray80;
                grid_vertices[v +  3] = gray70;
                grid_vertices[v +  5] = gray70;

                grid_vertices[v +  7] = gray70;
                grid_vertices[v +  9] = gray60;
                grid_vertices[v + 11] = gray70;
            }
            else
            {
                /* tamnije polje */
                grid_vertices[v +  1] = gray20;
                grid_vertices[v +  3] = gray30;
                grid_vertices[v +  5] = gray30;

                grid_vertices[v +  7] = gray30;
                grid_vertices[v +  9] = gray40;
                grid_vertices[v + 11] = gray30;
            }

        }

    }


    /* spustanje grida na graficku */

    glGenVertexArrays(1, &grid_vao);
    glGenBuffers(1, &grid_vbo);

    glBindVertexArray(grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
                        /* 6 trouglova po 3 vertexa po 6 floatova */
    glBufferData(GL_ARRAY_BUFFER, gridw * gridh * 2 * 3 * 2 * sizeof(vec3_t), grid_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 3 * sizeof(GLfloat));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    free(grid_vertices);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);


}


mat4_t model, view, projection, view_projection;

float fov = 90.0f;

vec3_t camera_position = vec3m(0.0f, 0.0f, -4.0f);
vec3_t camera_target = vec3m(0.0f, 0.0f, 0.0f);
vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);

float cam_angle = M_PI * 0.5f;
vec3_t aim_dir;

float angle_speed = 1.0f, move_speed = 4.0f;

float time = 0;

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{

    time += delta_time;

    float reverse = game_data->keys_down['S'] && !game_data->keys_down['W']? -1.0f : 1.0f;

    if(game_data->keys_down['A']) cam_angle -= angle_speed * delta_time * reverse;
    if(game_data->keys_down['D']) cam_angle += angle_speed * delta_time * reverse;

    aim_dir = vec3(cosf(cam_angle), 0.0f, sinf(cam_angle));

    if(game_data->keys_down['W']) camera_position = v3_add(camera_position, v3_muls(aim_dir, move_speed * delta_time));
    if(game_data->keys_down['S']) camera_position = v3_add(camera_position, v3_muls(aim_dir, move_speed * delta_time * -1.0f));

    if(game_data->keys_down[RAFGL_KEY_SPACE]) camera_position.y += move_speed * delta_time;
    if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) camera_position.y -= move_speed * delta_time;

    if(game_data->keys_pressed[RAFGL_KEY_KP_ADD]) glEnable(GL_DEPTH_TEST);
    if(game_data->keys_pressed[RAFGL_KEY_KP_SUBTRACT]) glDisable(GL_DEPTH_TEST);

    if(game_data->keys_pressed[RAFGL_KEY_ESCAPE]) glfwSetWindowShouldClose(window, GLFW_TRUE);

    camera_target = v3_add(camera_position, aim_dir);

    /* jedinicni vektor pogleda kamere (pravac u kom kamera gleda) */


    float aspect = ((float)(game_data->raster_width)) / game_data->raster_height;    projection = m4_perspective(fov, aspect, 0.01f, 100.0f);

    view = m4_look_at(camera_position, camera_target, camera_up);

    model = m4_identity();

    model = m4_mul(model, m4_rotation(time, vec3(0.0f, 1.0f, 0.0f)));
    model = m4_mul(model, m4_translation(vec3(0.0f, 1.0f * sinf(time), 0.0f)));

    view_projection = m4_mul(projection, view);



}


void main_state_render(GLFWwindow *window, void *args)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_program_id);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(pyramid_vao);

    glUniformMatrix4fv(uni_M, 1, GL_FALSE, model.m);
    glUniformMatrix4fv(uni_VP, 1, GL_FALSE, view_projection.m);

    glDrawArrays(GL_TRIANGLES, 0, 6 * 3);

    glBindVertexArray(grid_vao);

    glUniformMatrix4fv(uni_M, 1, GL_FALSE, m4_identity().m);
    glUniformMatrix4fv(uni_VP, 1, GL_FALSE, view_projection.m);

    glDrawArrays(GL_TRIANGLES, 0, gridw * gridh * 6);

    glBindVertexArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteBuffers(1, &pyramid_vbo);
    glDeleteVertexArrays(1, &pyramid_vao);
    glDeleteShader(shader_program_id);
}
