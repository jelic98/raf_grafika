#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include <rafgl.h>

#include <game_constants.h>

static GLfloat vertices[] =
{
    -1, -1,  0,     1, 0, 0,
     1, -1,  0,     0, 1, 0,
     0,  1,  0,     0, 0, 1,
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

    glBufferData(GL_ARRAY_BUFFER, 3 * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 3 * sizeof(GLfloat));


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);



}


/*
    Do sada smo radili u clip prostoru prozora, navodeci direktno konacne
    koordinate iz vertex buffera u gl_Position. Da bismo imali 3D grafiku
    kakva se inace koristi kroz OpenGL ili D3D, uspostavicemo sistem u kome
    koordinate verteksa navodimo u prostoru modela (model-space), a zatim ih
    transformisemo koristeci 4x4 transformacione matrice kako bismo ih
    u clip-space pozicije koje daju 3D izgled, definisan polozajem kamere
    (posmatraca) u odnosu na polozaj svakog objekta.

    Iako se na kraju sve svodi na jednu matricu koja kombinuje sve potrebne
    transformacije (transformacije su kumulativne, rezultat mnozenja ovih
    matrica kao rezultat daje matricu koja sadrzi u sebi i sve prethodne
    transformacije), cesto se prvo uspostave tri odvojene vrste:

    Model matrix - matrica transformacije modela, definise polozaj,
                   velicinu i orijentaciju modela u prostoru scene
                   ako ne zelimo transformacije
                   ovog tipa (koordinate verteksa ostaju kako su navedene),
                   za model matrix koristimo jedinicnu/identity matricu;
                   svaki pojedinacan objekat u sceni ima svoju model matricu

    View matrix  - matrica pogleda/kamere, koja definise polozaj i
                   orijentaciju posmatraca; zapravo sadrzi transformaciju
                   koja sve ostalo pomera tako da kamera ostane u nuli
                   koordinatnog sistema - dajuci relativnu iluziju da se
                   kamera poerila

    Projection	- matrica koja projektuje konacne 3D vertekse u 2D clip
    matrix         prostor prozora; ona odredjuje vrstu i parametre
                   projekcije (da li zelimo ortografski ili perspektivnu,
                   koji je odnos stranica slike (aspect-ratio), koji je
                   vidljivi ugao (field-of-view, FoV), itd.)
*/

/* view_projection je pomocna matrica koja ce biti unapred umnozene view i projection matrice */

mat4_t model, view, projection, view_projection;

/* field of view */
float fov = 90.0f;

vec3_t camera_position = vec3m(0.0f, 0.0f, -4.0f);
vec3_t camera_target = vec3m(0.0f, 0.0f, 0.0f);
vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);

void v3show(vec3_t v)
{
    printf("(%.2f %.2f %.2f)\n", v.x, v.y, v.z);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{


    if(game_data->keys_down['W']) camera_position.z += 1.0f * delta_time;
    if(game_data->keys_down['S']) camera_position.z -= 1.0f * delta_time;

    if(game_data->keys_down['A']) camera_position.x += 1.0f * delta_time;
    if(game_data->keys_down['D']) camera_position.x -= 1.0f * delta_time;

    if(game_data->keys_down[RAFGL_KEY_SPACE]) camera_position.y += 1.0f * delta_time;
    if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) camera_position.y -= 1.0f * delta_time;


    /**
    Matricu za projekciju mozemo konstruisati koristeci funkciju iz math_3d
    biblioteke na osnovu cetiri parametra :
    fov		- horizontalni ugao pogleda u stepenima, manji broj/uzi pogled
               daje "zumiran" izgled, velik broj daje fish-eye efekat
    aspect   - odnos (sirina / visina) prozora, npr. 1280 / 720 ~= 1.78,
    near     - daljina "near" ravni, sve blize od ovoga ce biti odbaceno
    far      - daljina "far" ravni, sve dalje od ovoga ce biti odbaceno
    */
    float aspect = ((float)(game_data->raster_width)) / game_data->raster_height;    projection = m4_perspective(fov, aspect, 0.1f, 100.0f);

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

    glDrawArrays(GL_TRIANGLES, 0, 3);

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
