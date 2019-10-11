#ifndef RAFGL_H_INCLUDED
#define RAFGL_H_INCLUDED

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include "rafgl_keys.h"

#define SYSTEM_SEPARATOR "\\"


#define pixel_at_m(r, x, y) (*(r.data + (y) * r.width + (x)))
#define pixel_at_pm(r, x, y) (*(r->data + (y) * r->width + (x)))


#define rafgl_abs_m(x) ((x) >= 0 ? (x) : -(x))
#define rafgl_min_m(x, y) ((x) < (y) ? (x) : (y))
#define rafgl_max_m(x, y) ((x) > (y) ? (x) : (y))
#define rafgl_game_add_named_game_state(game_p, state_name) rafgl_game_add_game_state(game_p, state_name##_init, state_name##_update, state_name##_render, state_name##_cleanup)


#define rafgl_RGBA(r, g, b, a) (((r) << 0) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define rafgl_RGB(r, g, b) rafgl_RGBA(r, g, b, 0xff)


typedef union _rafgl_pixel_rgb_t
{

    struct
    {
        uint8_t r, g, b, a;
    };
    uint32_t rgba;
    uint8_t components[4];

} rafgl_pixel_rgb_t;

typedef struct _rafgl_raster
{
    int width, height;
    rafgl_pixel_rgb_t *data;
} rafgl_raster_t;

typedef struct _rafgl_texture_t
{
    GLuint tex_id;
    int width, height, channels;
} rafgl_texture_t;

typedef struct _rafgl_list_t
{
    void *head;
    void *tail;
    int element_size;
    int count;
} rafgl_list_t;


typedef struct _rafgl_game_t
{
    rafgl_list_t game_states;
    int current_game_state;
    int next_game_state;

    GLFWwindow *window;
} rafgl_game_t;

typedef struct _rafgl_game_data_t
{
    int raster_width, raster_height;
    double mouse_pos_x, mouse_pos_y;
    int is_lmb_down, is_rmb_down, is_mmb_down;

    uint8_t *keys_down, *keys_pressed;

} rafgl_game_data_t;



typedef struct _rafgl_game_state_t
{
    int id;
    void *args;
    void (*init)(GLFWwindow *window, void *args);
    void (*update)(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args);
    void (*render)(GLFWwindow *window, void *args);
    void (*cleanup)(GLFWwindow *window, void *args);
} rafgl_game_state_t;


int rafgl_game_init(rafgl_game_t *game, const char *title, int window_width, int window_height, int fullscreen);
void rafgl_game_add_game_state(rafgl_game_t *game, void (*init)(GLFWwindow *window, void *args), void (*update)(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args), void (*render)(GLFWwindow *window, void *args), void (*cleanup)(GLFWwindow *window, void *args));

int rafgl_raster_init(rafgl_raster_t *raster, int width, int height);
int rafgl_raster_copy(rafgl_raster_t *raster_to, rafgl_raster_t *raster_from);
int rafgl_raster_load_from_image(rafgl_raster_t *raster, const char *image_path);
int rafgl_raster_cleanup(rafgl_raster_t *raster);


/* helpers function declarations start */


void rafgl_texture_init(rafgl_texture_t *tex);
int rafgl_texture_load_basic(const char *texture_path, rafgl_texture_t *res);
void rafgl_texture_load_from_raster(rafgl_texture_t *texture, rafgl_raster_t *raster);
void rafgl_texture_show(const rafgl_texture_t *texture);
void rafgl_texture_cleanup(rafgl_texture_t *texture);

char* read_file_content(const char *filepath);
int file_size(const char *filepath);

GLuint rafgl_program_create(const char *vertex_source_filepath, const char *fragment_source_filepath);
GLuint rafgl_program_create_from_source(const char *vertex_source, const char *fragment_source);
GLuint rafgl_program_create_from_name(const char *program_name);

int rafgl_list_init(rafgl_list_t *list, int element_size);
int rafgl_list_append(rafgl_list_t *list, void *data);
int rafgl_list_append_sized(rafgl_list_t *list, int size, void *data);
int rafgl_list_remove(rafgl_list_t *list, int index);
void* rafgl_list_get(rafgl_list_t *list, int index);
int rafgl_list_free(rafgl_list_t *list);
int rafgl_list_show(rafgl_list_t *list, void (*fun)(void *data, int last));
int rafgl_list_test(void);

float rafgl_distance1D(float x1, float x2);
float rafgl_distance2D(float x1, float y1, float x2, float y2);
float rafgl_distance3D(float x1, float y1, float z1, float x2, float y2, float z2);

int rafgl_clampi(int value, int lower, int upper);

/* helpers function declarations end*/


//#define RAFGL_IMPLEMENTATION

#ifdef RAFGL_IMPLEMENTATION

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
/* rafgl core implementation */

static GLFWwindow *__window;
static int __done = 0;
static int __window_width = 0, __window_height = 0;

static uint8_t __keys_down[400];
static uint8_t __keys_pressed[400];


static const char *__2D_raster_vertex_shader_source = "\
#version 330 core\n\
\n\
layout(location = 0) in vec2 pos;\n\
\n\
out vec2 uv;\n\
\n\
void main()\n\
{\n\
    gl_Position = vec4(pos, 0.0, 1.0);\n\
    uv = (pos + vec2(1.0)) * 0.5;\n\
    uv.y = 1.0 - uv.y;\n\
}\
";

static const char *__2D_raster_fragment_shader_source = "\
#version 330 core\n\
\n\
in vec2 uv;\n\
out vec4 frag_colour;\n\
uniform sampler2D raster;\n\
\n\
void main()\n\
{\n\
    frag_colour = texture(raster, uv);\n\
    //frag_colour = vec4(uv.x);\n\
}\
";

static unsigned int __raster_program = 0;
static unsigned int __raster_vao = 0;

static float __raster_corners[] = {
    -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f
};


void __key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    /* printf("%c %d\n", key, action); */
    if(__keys_down[key] == 0 && action != 0) __keys_pressed[key] = 1;
        else __keys_pressed[key] = 1;
    __keys_down[key] = action;

}

void __error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int rafgl_game_init(rafgl_game_t *game, const char *title, int window_width, int window_height, int fullscreen)
{
    if(__done) return -1;
    __done = 1;



    __window_width = window_width;
    __window_height = window_height;

    if(!glfwInit())
    {
        fprintf(stderr, "GLFWInit() failed\n");
    }

    glfwSetErrorCallback(__error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWmonitor *mnt = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(mnt);


    if(fullscreen)
        __window = glfwCreateWindow(mode->width, mode->height, title, mnt, NULL);
    else
        __window = glfwCreateWindow(window_width, window_height, title, NULL, NULL);


    if(__window == NULL)
    {
        fprintf(stderr, "Failed to create GLFW window!\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(__window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initiate GLAD!\n");
        glfwTerminate();
        return -1;
    }

    game -> window = __window;
    game -> current_game_state = -1;
    game -> next_game_state = -1;
    rafgl_list_init(&(game -> game_states), sizeof(rafgl_game_state_t));

    if(!__raster_vao)
    {
        glGenVertexArrays(1, &__raster_vao);
        GLuint raster_vbo;
        glGenBuffers(1, &raster_vbo);
        glBindVertexArray(__raster_vao);
        glBindBuffer(GL_ARRAY_BUFFER, raster_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(__raster_corners), __raster_corners, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }

    if(!__raster_program)
    {
        __raster_program = rafgl_program_create_from_source(__2D_raster_vertex_shader_source, __2D_raster_fragment_shader_source);
        glUniform1i(glGetUniformLocation(__raster_program, "raster"), 0);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glfwSetKeyCallback(__window, __key_callback);

    return 0;
}


int rafgl_raster_init(rafgl_raster_t *raster, int width, int height)
{
    raster->data = calloc(width * height, sizeof(rafgl_pixel_rgb_t));
    raster->width = width;
    raster->height = height;
    return 0;
}

int rafgl_raster_cleanup(rafgl_raster_t *raster)
{
    free(raster->data);
    raster->height = 0;
    raster->width = 0;
    return 0;
}

int rafgl_raster_copy(rafgl_raster_t *raster_to, rafgl_raster_t *raster_from)
{

    if(raster_to->data == NULL)
    {
        /* new raster */
        rafgl_raster_init(raster_to, raster_from->width, raster_from->height);
    }
    else if(raster_to -> width != raster_from -> width || raster_to -> height != raster_from -> height)
    {
        /* resize */
        rafgl_raster_cleanup(raster_to);
        rafgl_raster_init(raster_to, raster_from->width, raster_from->height);
    }

    /* just copy */
    memcpy(raster_to->data, raster_from->data, raster_from->width * raster_from->height * sizeof(rafgl_pixel_rgb_t));
    return 0;
}

int rafgl_raster_load_from_image(rafgl_raster_t *raster, const char *image_path)
{
    int width, height, channels;
    raster->data = (rafgl_pixel_rgb_t *) stbi_load(image_path, &width, &height, &channels, 4);
    raster->width = width;
    raster->height = height;
    return 0;
}


void rafgl_game_add_game_state(rafgl_game_t *game, void (*init)(GLFWwindow *window, void *args), void (*update)(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args), void (*render)(GLFWwindow *window, void *args), void (*cleanup)(GLFWwindow *window, void *args))
{
    rafgl_game_state_t state;
    state.init = init;
    state.update = update;
    state.render = render;
    state.cleanup = cleanup;
    state.id = 0;

    rafgl_list_append(&game->game_states, &state);
}


static int __game_state_change_request = -1;
static void *__game_state_change_request_args = NULL;

void rafgl_game_request_state_change(int state_index, void *args)
{
    __game_state_change_request = state_index;
    __game_state_change_request_args = args;
}

void rafgl_game_start(rafgl_game_t *game, void *_args)
{
    void *args = _args;
    rafgl_game_state_t *current_state = rafgl_list_get(&game->game_states, 0);
    int current_game_state_index = 0;

    rafgl_game_data_t game_data;
    game_data.keys_down = __keys_down;
    game_data.keys_pressed = __keys_pressed;

    current_state->init(game->window, args);


    double current_frame, last_frame;
    float elapsed;

    last_frame = glfwGetTime();

    int fbwidth, fbheight, fbwlast = 0, fbhlast = 0;

    while(!glfwWindowShouldClose(game->window))
    {
        glfwPollEvents();

        current_frame = glfwGetTime();
        elapsed = current_frame - last_frame;
        last_frame = current_frame;


        glfwGetFramebufferSize(game->window, &fbwidth, &fbheight);
        if(fbwlast != fbwidth || fbhlast != fbheight)
        {
            glViewport(0, 0, fbwidth, fbheight);
        }
        fbwlast = fbwidth;
        fbhlast = fbheight;

        game_data.raster_width = fbwidth;
        game_data.raster_height = fbheight;

        glfwGetCursorPos(game->window, &game_data.mouse_pos_x, &game_data.mouse_pos_y);

        game_data.is_lmb_down = glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_LEFT);
        game_data.is_rmb_down = glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_RIGHT);
        game_data.is_mmb_down = glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_MIDDLE);

        current_state->update(game->window, elapsed, &game_data, args);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        current_state->render(game->window, args);

        glfwSwapBuffers(game->window);

        if(__game_state_change_request == current_game_state_index)
        {
            printf("Already in that state!\n");
            __game_state_change_request = -1;
        }

        if(__game_state_change_request >= 0)
        {
            printf("Changigng state!\n");
            current_state->cleanup(game->window, args);

            args = __game_state_change_request_args;
            __game_state_change_request_args = NULL;

            current_state = rafgl_list_get(&game->game_states, __game_state_change_request);

            current_game_state_index = __game_state_change_request;
            __game_state_change_request = -1;

            current_state->init(game->window, args);
            last_frame = glfwGetTime();

        }

    }


}



/* Helpers implementation*/



inline float rafgl_distance1D(float x1, float x2)
{
    if(x1 > x2)
        return x1 - x2;
    else
        return x2 - x1;
}

inline float rafgl_distance2D(float x1, float y1, float x2, float y2)
{
    float delta_x = x2 - x1;
    float delta_y = y2 - y1;

    return sqrtf(delta_x * delta_x + delta_y * delta_y);
}

inline float rafgl_distance3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float delta_x = x2 - x1;
    float delta_y = y2 - y1;
    float delta_z = z2 - z1;

    return sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
}

int rafgl_clampi(int value, int lower, int upper)
{
    if(value < lower)
        return lower;
    else if(value > upper)
        return upper;
    else return value;
}


void rafgl_texture_init(rafgl_texture_t *tex)
{
    GLuint tx;
    glGenTextures(1, &tx);
    tex->channels = 0;
    tex->width = 0;
    tex->height = 0;
    tex->tex_id = tx;
}

int rafgl_texture_load_basic(const char *texture_path, rafgl_texture_t *res)
{
    return -1;
}



void rafgl_texture_load_from_raster(rafgl_texture_t *texture, rafgl_raster_t *raster)
{
    GLuint tex_slot = texture->tex_id;
    glBindTexture(GL_TEXTURE_2D, tex_slot);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raster->width, raster->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raster->data);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture->tex_id = tex_slot;
    texture->width = raster->width;
    texture->height = raster->height;
    texture->channels = 3;
}


void rafgl_texture_show(const rafgl_texture_t *texture)
{

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->tex_id);

    glBindVertexArray(__raster_vao);
    glUseProgram(__raster_program);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void rafgl_texture_cleanup(rafgl_texture_t *texture)
{
    glDeleteTextures(1, &(texture->tex_id));
    texture->channels = 0;
    texture->height = 0;
    texture->width = 0;
    texture->tex_id = 0;
    return;
}


int rafgl_list_init(rafgl_list_t *list, int element_size)
{
    list -> count = 0;
    list -> element_size = element_size;
    list -> head = NULL;
    list -> tail = NULL;
    return 0;
}

int rafgl_list_append(rafgl_list_t *list, void *data)
{
    return rafgl_list_append_sized(list, list -> element_size, data);
}

int rafgl_list_append_sized(rafgl_list_t *list, int size, void *data)
{
    if(list -> head == NULL && list -> tail == NULL)
    {
        list -> head = list -> tail = malloc(sizeof(void*) + size);
        memcpy(list -> tail + sizeof(void*), data, size);
        *((void**)list -> tail) = NULL;
        list -> count++;
    }
    else
    {
        *((void**)list -> tail) = malloc(sizeof(void*) + size);
        list -> tail = *((void**)list -> tail);
        memcpy(list -> tail + sizeof(void*), data, size);
        *((void**)list -> tail) = NULL;
        list -> count++;
    }
    return 0;
}

int rafgl_list_remove(rafgl_list_t *list, int index)
{
    void *target, *i = list -> head;
    if(index >= list -> count) return -1;
    if(index < 0) index = list -> count + index;
    if(index < 0) return -1;

    if(index == 0)
    {
        target = list -> head;
        list -> head = *((void**) list -> head);
    }
    else
    {
        index--;
        while(index--)
        {
            i = *((void**)i);
        }
        target = *((void**)i);
        *((void**)i) = *((void**)(*((void**)i)));

    }
    list -> count--;
    free(target);
    return 0;
}

void* rafgl_list_get(rafgl_list_t *list, int index)
{
    void *i = list -> head;

    if(index >= list -> count) return NULL;
    if(index < 0) index = list -> count + index;
    if(index < 0) return NULL;

    while(index--)
    {
        i = *((void**)i);
    }

    return i + sizeof(void*);
}

int rafgl_list_free(rafgl_list_t *list)
{
    void **i = list -> head;
    void *curr;
    while(i)
    {
        curr = i;
        i = *i;
        free(curr);
    }
    return 0;
}

int rafgl_list_show(rafgl_list_t *list, void (*fun)(void*, int))
{
    void **i = list -> head;
    void *curr_data;
    while(i)
    {
        curr_data = i + 1;
        i = *i;
        fun(curr_data, i == NULL);
    }
    return 0;
}

int rafgl_file_size(const char *filepath)
{
    int size = 0;
    FILE *f = fopen(filepath, "rt");

    fseek(f, 0L, SEEK_END);

    size = ftell(f);

    fclose(f);

    return size;
}

char* rafgl_file_read_content(const char *filepath)
{
    int fsize = rafgl_file_size(filepath);
    FILE *f = fopen(filepath, "rt");

    fseek(f, 0, SEEK_SET);

    char *content = calloc(sizeof(char), fsize + 10);          /* This must later be freed */

    fread(content, 1, fsize, f);

    fclose(f);

    *(content + fsize) = 0;
    return content;
}

GLuint rafgl_program_create_from_source(const char *vertex_source, const char *fragment_source)
{
    GLuint vert, frag, program;
    int success;
    char info_log[512];

    vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertex_source, NULL);
    glCompileShader(vert);

    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(vert, 512, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILE_FAILED\n%s\n", info_log);
    }

    frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragment_source, NULL);
    glCompileShader(frag);

    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(frag, 512, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILE_FAILED\n%s\n", info_log);
    }


    program = glCreateProgram();

    glAttachShader(program, vert);
    glAttachShader(program, frag);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
    }


    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

GLuint rafgl_program_create(const char *vertex_source_filepath, const char *fragment_source_filepath)
{
    GLuint program;

    char *vert_source = rafgl_file_read_content(vertex_source_filepath);
    char *frag_source = rafgl_file_read_content(fragment_source_filepath);


    program = rafgl_program_create_from_source(vert_source, frag_source);

    free(vert_source);
    free(frag_source);

    return program;
}

GLuint rafgl_program_create_from_name(const char *program_name)
{
    char v[255], f[255];
    v[0] = 0;
    f[0] = 0;

    strcat(v, "res" SYSTEM_SEPARATOR "shaders" SYSTEM_SEPARATOR);
    strcat(v, program_name);
    strcat(v, SYSTEM_SEPARATOR "vert.glsl");

    strcat(f, "res" SYSTEM_SEPARATOR "shaders" SYSTEM_SEPARATOR);
    strcat(f, program_name);
    strcat(f, SYSTEM_SEPARATOR "frag.glsl");

    return rafgl_program_create(v, f);
}

/*
void test_show(void *element, int last)
{
    vec3f data = *((vec3f*)element);
    vec3f_show(data);
    if(!last) printf(" -> ");
}

int rafgl_list_test(void)
{
    rafgl_list_t list;
    rafgl_list_init(&list, sizeof(vec3f));
    vec3f tmp = vec3f_create(0.0f, 0.0f, 0.0f);

    int i;
    for(i = 0; i < 10; i++)
    {
        tmp.x = i;
        tmp.z = 2.0f * i;
        rafgl_list_append(&list, &tmp);
    }

    rafgl_list_show(&list, test_show);
    rafgl_list_remove(&list, 0);
    rafgl_list_remove(&list, 1);
    rafgl_list_remove(&list, -1);
    rafgl_list_show(&list, test_show);

    rafgl_list_free(&list);

}
*/

#endif // RAFGL_IMPLEMENTATION
#endif // RAFGL_H_INCLUDED
