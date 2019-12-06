#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include <rafgl.h>

#include <game_constants.h>

/* slicno kao linearna interpolacija */
float cosine_interpolationf(float a, float b, float s)
{
    float f = (1.0f - cosf(s * M_PI))* 0.5f;
    return a + (b - a) * f;
}

/* slicno kao bilinearno semplovanje */
void cosine_float_map_rescale(float *dst, int dst_width, int dst_height, float *src, int src_width, int src_height)
{
    int x, y;
    float xn, yn;
    float fxs, fys;
    int ixs0, iys0;
    int ixs1, iys1;
    float upper_middle, lower_middle;
    float sample_left, sample_right;
    float result;

    for(y = 0; y < dst_height; y++)
    {
        yn = 1.0f * y / dst_height;
        fys = yn * src_height;
        iys0 = fys;
        iys1 = iys0 + 1;
        fys -= iys0;
        if(iys1 >= src_height) iys1 = src_height - 1;
        for(x = 0; x < dst_width; x++)
        {
            xn = 1.0f * x / dst_width;
            fxs = xn * src_width;
            ixs0 = fxs;
            ixs1 = ixs0 + 1;
            if(ixs1 >= src_width) ixs1 = src_width - 1;
            fxs -= ixs0;

            sample_left = src[iys0 * src_width + ixs0];
            sample_right = src[iys0 * src_width + ixs1];
            upper_middle = cosine_interpolationf(sample_left, sample_right, fxs);

            sample_left = src[iys1 * src_width + ixs0];
            sample_right = src[iys1 * src_width + ixs1];
            lower_middle = cosine_interpolationf(sample_left, sample_right, fxs);

            result = cosine_interpolationf(upper_middle, lower_middle, fys);

            dst[y * dst_width + x] = result;


        }
    }


}

void float_map_multiply_and_add(float *dst, float *src, int w, int h, float multiplier)
{
    int x, y;
    for(y = 0; y < h; y++)
    {
        for(x = 0; x < w; x++)
        {
            dst[y * w + x] += src[y * w + x] * multiplier;
        }
    }
}


rafgl_raster_t generate_perlin_noise(int octaves, float persistance)
{
    int octave_size = 2;
    float multiplier = 1.0f;

    rafgl_raster_t r;

    int width = pow(2, octaves);
    int height = width;

    int i, octave, x, y;
    float *tmp_map = malloc(height * width * sizeof(float));
    float *final_map = calloc(height * width, sizeof(float));
    float *octave_map;
    rafgl_pixel_rgb_t pix;
    rafgl_raster_init(&r, width, height);


    for(octave = 0; octave < octaves; octave++)
    {
        octave_map = malloc(octave_size * octave_size * sizeof(float));
        for(y = 0; y < octave_size; y++)
        {
            for(x = 0; x < octave_size; x++)
            {
                octave_map[y * octave_size + x] = randf() * 2.0f - 1.0f;
            }
        }

        cosine_float_map_rescale(tmp_map, width, height, octave_map, octave_size, octave_size);
        float_map_multiply_and_add(final_map, tmp_map, width, height, multiplier);

        octave_size *= 2;
        multiplier *= persistance;

        free(octave_map);
    }

    float sample;
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            sample = final_map[y * width + x];
            sample = (sample + 1.0f) / 2.0f;
            if(sample < 0.0f) sample = 0.0f;
            if(sample > 1.0f) sample = 1.0f;
            pix.r = sample * 255;
            pix.g = sample * 255;
            pix.b = sample * 255;

            pixel_at_m(r, x, y) = pix;
        }
    }


    free(final_map);
    free(tmp_map);

    return r;

}


static rafgl_meshPUN_t meshes[6];

static vec3_t object_colour = RAFGL_BLUE;
static vec3_t light_colour = RAFGL_WHITE;
static vec3_t light_direction = vec3m(-0.55f, -0.55f, -0.63f);
static vec3_t ambient = RAFGL_GRAYX(0.16f);

#define NUM_SHADERS 8
static GLuint object_shader[NUM_SHADERS], object_uni_M[NUM_SHADERS], object_uni_VP[NUM_SHADERS], object_uni_object_colour[NUM_SHADERS], object_uni_light_colour[NUM_SHADERS], object_uni_light_direction[NUM_SHADERS], object_uni_ambient[NUM_SHADERS], object_uni_camera_position[NUM_SHADERS];
static const char *mesh_names[6] = {"res/models/monkey-adv.obj", "res/models/monkey-adv.obj", "res/models/suzanne.obj", "res/models/bunny.obj", "res/models/armadillo.obj", "res/models/dragon.obj"};

static int current_shader = 0;

static int num_meshes;

static rafgl_texture_t skybox_tex, perlin_tex;

static GLuint skybox_shader, skybox_shader_cell;
static GLuint skybox_uni_P, skybox_uni_V;
static GLuint skybox_cell_uni_P, skybox_cell_uni_V;

static GLuint uni_visibility_factor;

static rafgl_meshPUN_t skybox_mesh;

static rafgl_framebuffer_simple_t fbo;

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{

    fbo = rafgl_framebuffer_simple_create(width, height);

    rafgl_raster_t perlin = generate_perlin_noise(9, 0.5f);
    rafgl_texture_init(&perlin_tex);
    rafgl_texture_load_from_raster(&perlin_tex, &perlin);
    rafgl_raster_cleanup(&perlin);

    rafgl_texture_load_cubemap_named(&skybox_tex, "above_the_sea", "jpg");
    skybox_shader = rafgl_program_create_from_name("v9_skybox_shader");
    skybox_shader_cell = rafgl_program_create_from_name("v9_skybox_shader_cell");

    skybox_uni_P = glGetUniformLocation(skybox_shader, "uni_P");
    skybox_uni_V = glGetUniformLocation(skybox_shader, "uni_V");

    skybox_cell_uni_P = glGetUniformLocation(skybox_shader_cell, "uni_P");
    skybox_cell_uni_V = glGetUniformLocation(skybox_shader_cell, "uni_V");

    rafgl_meshPUN_init(&skybox_mesh);
    rafgl_meshPUN_load_cube(&skybox_mesh, 1.0f);

    num_meshes = sizeof(mesh_names) / sizeof(mesh_names[0]);
    object_colour = vec3(0.8f, 0.40f, 0.0f);

    rafgl_log_fps(RAFGL_TRUE);

    int i;

    for(i = 0; i < 1; i++)
    {
        rafgl_log(RAFGL_INFO, "Loading mesh %d!\n", i + 1);
        rafgl_meshPUN_init(meshes + i);
		printf("AAAAAAAAAAAAAAa\n");
        rafgl_meshPUN_load_from_OBJ(meshes + i, mesh_names[i]);
		printf("BBBBBBBBBBBBBB\n");
    }


    char shader_name[128];

    for(i = 0; i < NUM_SHADERS; i++)
    {
        sprintf(shader_name, "v9_object_shader%d", i);
        object_shader[i] = rafgl_program_create_from_name(shader_name);
        object_uni_M[i] = glGetUniformLocation(object_shader[i], "uni_M");
        object_uni_VP[i] = glGetUniformLocation(object_shader[i], "uni_VP");
        object_uni_object_colour[i] = glGetUniformLocation(object_shader[i], "uni_object_colour");
        object_uni_light_colour[i] = glGetUniformLocation(object_shader[i], "uni_light_colour");
        object_uni_light_direction[i] = glGetUniformLocation(object_shader[i], "uni_light_direction");
        object_uni_ambient[i] = glGetUniformLocation(object_shader[i], "uni_ambient");
        object_uni_camera_position[i] = glGetUniformLocation(object_shader[i], "uni_camera_position");
    }


    uni_visibility_factor = glGetUniformLocation(object_shader[7], "uni_visibility_factor");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    light_direction = v3_norm(light_direction);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);


}


mat4_t model, view, projection, view_projection;

/* field of view */
float fov = 75.0f;

vec3_t camera_position = vec3m(0.0f, 1.0f, 6.5f);
vec3_t camera_target = vec3m(0.0f, 0.0f, 0.0f);
vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);
vec3_t aim_dir = vec3m(0.0f, 0.0f, -1.0f);

float camera_angle = -M_PIf * 0.5f;
float angle_speed = 0.2f * M_PIf;
float move_speed = 2.4f;

float hoffset = 0;

int rotate = 0;
float model_angle = 0.0f;

void v3show(vec3_t v)
{
    printf("(%.2f %.2f %.2f)\n", v.x, v.y, v.z);
}

float time = 0.0f;
int reshow_cursor_flag = 0;
int last_lmb = 0;


int selected_mesh = 3;
float sensitivity = 1.0f;

int selected_shader = 0;

float visibility_factor = -2.0f;
float turn = 1.0f;

int is_P_down = 0;

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    visibility_factor += delta_time * turn;
    if(visibility_factor > 12.0f) turn = -1;
    if(visibility_factor < -5.0f) turn = 1;

    time += delta_time;
    model_angle += delta_time * rotate;


    if(!game_data->keys_down[RAFGL_KEY_LEFT_SHIFT])
    {
        angle_speed = 0.2f * M_PIf;
        move_speed = 2.4f;
        sensitivity = 1.0f;
    }
    else
    {
        angle_speed = 5 * 0.2f * M_PIf;
        move_speed = 5 * 2.4f;
        sensitivity = 10.0f;
    }

    is_P_down = game_data->keys_down['P'];


    if(game_data->is_lmb_down)
    {

        if(reshow_cursor_flag == 0)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }

        float ydelta = game_data->mouse_pos_y - game_data->raster_height / 2;
        float xdelta = game_data->mouse_pos_x - game_data->raster_width / 2;

        if(!last_lmb)
        {
            ydelta = 0;
            xdelta = 0;
        }

        hoffset -= sensitivity * ydelta / game_data->raster_height;
        camera_angle += sensitivity * xdelta / game_data->raster_width;

        glfwSetCursorPos(window, game_data->raster_width / 2, game_data->raster_height / 2);
        reshow_cursor_flag = 1;
    }
    else if(reshow_cursor_flag)
    {
        reshow_cursor_flag = 0;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    last_lmb = game_data->is_lmb_down;

    aim_dir = v3_norm(vec3(cosf(camera_angle), hoffset, sinf(camera_angle)));

    if(game_data->keys_down['W']) camera_position = v3_add(camera_position, v3_muls(aim_dir, move_speed * delta_time));
    if(game_data->keys_down['S']) camera_position = v3_add(camera_position, v3_muls(aim_dir, -move_speed * delta_time));

    vec3_t right = v3_cross(aim_dir, vec3(0.0f, 1.0f, 0.0f));
    if(game_data->keys_down['D']) camera_position = v3_add(camera_position, v3_muls(right, move_speed * delta_time));
    if(game_data->keys_down['A']) camera_position = v3_add(camera_position, v3_muls(right, -move_speed * delta_time));

    if(game_data->keys_pressed['R']) rotate = !rotate;

    if(game_data->keys_pressed[RAFGL_KEY_UP]) selected_mesh = (selected_mesh + 1) % num_meshes;
    if(game_data->keys_pressed[RAFGL_KEY_DOWN]) selected_mesh = (selected_mesh + num_meshes - 1) % num_meshes;

    if(game_data->keys_down[RAFGL_KEY_ESCAPE]) glfwSetWindowShouldClose(window, GLFW_TRUE);

    if(game_data->keys_down[RAFGL_KEY_SPACE]) camera_position.y += 1.0f * delta_time * move_speed;
    if(game_data->keys_down[RAFGL_KEY_LEFT_CONTROL]) camera_position.y -= 1.0f * delta_time * move_speed;


    int i;
    for(i = 0; i < NUM_SHADERS; i++)
    {
        if(selected_shader != i && game_data->keys_pressed[RAFGL_KEY_0 + i])
        {
            selected_shader = i;

            if(selected_shader == 7)
            {
                visibility_factor = -2.0f;
                selected_mesh = 0;
            }

            if(i == 1)
            {
                object_colour = vec3(0.8f, 0.40f, 0.0f);
            }
            else
            {
                object_colour = RAFGL_CYAN;
            }
        }
    }

    float aspect = ((float)(game_data->raster_width)) / game_data->raster_height;    projection = m4_perspective(fov, aspect, 0.1f, 100.0f);

    if(!game_data->keys_down['T'])
    {
        view = m4_look_at(camera_position, v3_add(camera_position, aim_dir), camera_up);
    }
    else
    {
        view = m4_look_at(camera_position, vec3(0.0f, 0.0f, 0.0f), camera_up);
    }

    model = m4_identity();
    model = m4_rotation_y(model_angle);
    model = m4_mul(model, m4_translation(vec3(0.0f, sinf(model_angle) * 0.45, 0.0f)));

    view_projection = m4_mul(projection, view);


}


void main_state_render(GLFWwindow *window, void *args)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo_id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glDepthMask(GL_FALSE);

    if(selected_shader != 6)
    {
        glUseProgram(skybox_shader);
        glUniformMatrix4fv(skybox_uni_V, 1, GL_FALSE, (void*) view.m);
        glUniformMatrix4fv(skybox_uni_P, 1, GL_FALSE, (void*) projection.m);
    }
    else
    {
        glUseProgram(skybox_shader_cell);
        glUniformMatrix4fv(skybox_cell_uni_V, 1, GL_FALSE, (void*) view.m);
        glUniformMatrix4fv(skybox_cell_uni_P, 1, GL_FALSE, (void*) projection.m);
    }

    glBindVertexArray(skybox_mesh.vao_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);

    glDrawArrays(GL_TRIANGLES, 0, skybox_mesh.vertex_count);
    glDepthMask(GL_TRUE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


    glUseProgram(object_shader[selected_shader]);


    if(selected_shader == 7)
    {
        glUniform1f(uni_visibility_factor, visibility_factor / 10.0f);
    }

    glBindTexture(GL_TEXTURE_2D, perlin_tex.tex_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(meshes[selected_mesh].vao_id);

    glUniformMatrix4fv(object_uni_M[selected_shader], 1, GL_FALSE, (void*) model.m);
    glUniformMatrix4fv(object_uni_VP[selected_shader], 1, GL_FALSE, (void*) view_projection.m);

    glUniform3f(object_uni_object_colour[selected_shader], object_colour.x, object_colour.y, object_colour.z);
    glUniform3f(object_uni_light_colour[selected_shader], light_colour.x, light_colour.y, light_colour.z);
    glUniform3f(object_uni_light_direction[selected_shader], light_direction.x, light_direction.y, light_direction.z);
    glUniform3f(object_uni_ambient[selected_shader], ambient.x, ambient.y, ambient.z);
    glUniform3f(object_uni_camera_position[selected_shader], camera_position.x, camera_position.y, camera_position.z);

    glDrawArrays(GL_TRIANGLES, 0, meshes[selected_mesh].vertex_count);

    glBindVertexArray(0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    if(is_P_down)
    {
        rafgl_texture_show(&perlin_tex, 0);
    }
    else
    {
        rafgl_texture_t tmptex;
        tmptex.tex_id = fbo.tex_id;
        rafgl_texture_show(&tmptex, 1);
    }
    glEnable(GL_DEPTH_TEST);

}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteShader(object_shader);
}
