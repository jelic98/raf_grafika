#include <math.h>
#include <glad/glad.h>
#include <rafgl.h>
#include <game_constants.h>
#include <main_state.h>

#define TOTAL_MESHES 1
#define MAX_SHADERS 1
#define LENGTH_NAME 128
#define FIELD_OF_VIEW 75.0f

static vec3_t object_color = RAFGL_GREEN;
static vec3_t light_color = RAFGL_WHITE;
static vec3_t light_direction = vec3m(-0.55f, -0.55f, -0.63f);
static vec3_t ambient = RAFGL_GRAYX(0.16f);

static GLuint object_shader[MAX_SHADERS],
			  object_uni_M[MAX_SHADERS],
			  object_uni_VP[MAX_SHADERS],
			  object_uni_object_color[MAX_SHADERS],
			  object_uni_light_color[MAX_SHADERS],
			  object_uni_light_direction[MAX_SHADERS],
			  object_uni_ambient[MAX_SHADERS],
			  object_uni_camera_position[MAX_SHADERS];

static GLuint skybox_shader;
static GLuint skybox_uni_P, skybox_uni_V;

static rafgl_texture_t skybox_tex;
static rafgl_meshPUN_t skybox_mesh;

static rafgl_framebuffer_simple_t fbo;

static mat4_t model, view, projection, view_projection;

static vec3_t camera_position = vec3m(0.0f, 1.0f, 6.5f);
static vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);
static vec3_t aim_dir = vec3m(0.0f, 0.0f, -1.0f);

static rafgl_meshPUN_t meshes[TOTAL_MESHES];
static const char* mesh_names[TOTAL_MESHES] = { "res/models/monkey.obj" };

static int current_mesh = 0;
static int current_shader = 0;

static float angle_v = 0.0f;
static float angle_h = -M_PIf * 0.5f;
static float angle_speed = M_PIf * 0.003f;
static float angle_model = -M_PIf;
static float speed_move = 5.0f;

static int flag_rotate = 0;

void main_state_init(GLFWwindow* window, void* args, int width, int height) {	
	rafgl_log_fps(RAFGL_TRUE);
	
	fbo = rafgl_framebuffer_simple_create(width, height);

	rafgl_texture_load_cubemap_named(&skybox_tex, "above_the_sea", "jpg");
	
	skybox_shader = rafgl_program_create_from_name("skybox");
	skybox_uni_P = glGetUniformLocation(skybox_shader, "uni_P");
	skybox_uni_V = glGetUniformLocation(skybox_shader, "uni_V");
	
	rafgl_meshPUN_init(&skybox_mesh);
	rafgl_meshPUN_load_cube(&skybox_mesh, 1.0f);

	int i;

	for(i = 0; i < 1; i++) {
		rafgl_meshPUN_init(meshes + i);
		rafgl_meshPUN_load_from_OBJ(meshes + i, mesh_names[i]);
	}

	char shader_name[LENGTH_NAME];

	for(i = 0; i < MAX_SHADERS; i++) {
		sprintf(shader_name, "object_%d", i + 1);
		
		object_shader[i] = rafgl_program_create_from_name(shader_name);
		
		object_uni_M[i] = glGetUniformLocation(object_shader[i], "uni_M");
		object_uni_VP[i] = glGetUniformLocation(object_shader[i], "uni_VP");
		object_uni_object_color[i] = glGetUniformLocation(object_shader[i], "uni_object_color");
		object_uni_light_color[i] = glGetUniformLocation(object_shader[i], "uni_light_color");
		object_uni_light_direction[i] = glGetUniformLocation(object_shader[i], "uni_light_direction");
		object_uni_ambient[i] = glGetUniformLocation(object_shader[i], "uni_ambient");
		object_uni_camera_position[i] = glGetUniformLocation(object_shader[i], "uni_camera_position");
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	
	light_direction = v3_norm(light_direction);
}

void main_state_update(GLFWwindow* window, float delta_time, rafgl_game_data_t* game_data, void* args) {
	if(game_data->keys_down[RAFGL_KEY_UP]) {
		angle_v -= angle_speed;
	}else if(game_data->keys_down[RAFGL_KEY_DOWN]) {
		angle_v += angle_speed;
	}

	if(game_data->keys_down[RAFGL_KEY_LEFT]) {
		angle_h -= angle_speed;
	}else if(game_data->keys_down[RAFGL_KEY_RIGHT]) {
		angle_h += angle_speed;
	}

	aim_dir = v3_norm(vec3(cosf(angle_h), angle_v, sinf(angle_h)));
	vec3_t right = v3_cross(aim_dir, vec3(0.0f, 1.0f, 0.0f));

	if(game_data->keys_down['W']) {
		camera_position = v3_add(camera_position, v3_muls(aim_dir, speed_move * delta_time));
	}else if(game_data->keys_down['S']) {
		camera_position = v3_add(camera_position, v3_muls(aim_dir, -speed_move * delta_time));
	}

	if(game_data->keys_down['D']) {
		camera_position = v3_add(camera_position, v3_muls(right, speed_move * delta_time));
	}else if(game_data->keys_down['A']) {
		camera_position = v3_add(camera_position, v3_muls(right, -speed_move * delta_time));
	}

	if(game_data->keys_down[RAFGL_KEY_SPACE]) {
		camera_position.y += 1.0f * delta_time * speed_move;
	}else if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) {
		camera_position.y -= 1.0f * delta_time * speed_move;
	}

	if(game_data->keys_pressed['R']) {
		flag_rotate = !flag_rotate;
	}

	if(game_data->keys_down[RAFGL_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	model = m4_identity();
	model = m4_rotation_y(angle_model += delta_time * flag_rotate);
	model = m4_mul(model, m4_translation(vec3(0.0f, sinf(angle_model) * 0.45, 0.0f)));

	view = m4_look_at(camera_position, v3_add(camera_position, aim_dir), camera_up);

	float raster_aspect = ((float) (game_data->raster_width)) / game_data->raster_height;
	projection = m4_perspective(FIELD_OF_VIEW, raster_aspect, 0.1f, 100.0f);
	view_projection = m4_mul(projection, view);
}

void main_state_render(GLFWwindow* window, void* args) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	glUseProgram(skybox_shader);
	glUniformMatrix4fv(skybox_uni_V, 1, GL_FALSE, (void*) view.m);
	glUniformMatrix4fv(skybox_uni_P, 1, GL_FALSE, (void*) projection.m);
	
	glBindVertexArray(skybox_mesh.vao_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);

	glDrawArrays(GL_TRIANGLES, 0, skybox_mesh.vertex_count);
	glDepthMask(GL_TRUE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glUseProgram(object_shader[current_shader]);

	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(meshes[current_mesh].vao_id);

	glUniformMatrix4fv(object_uni_M[current_shader], 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(object_uni_VP[current_shader], 1, GL_FALSE, (void*) view_projection.m);

	glUniform3f(object_uni_object_color[current_shader], object_color.x, object_color.y, object_color.z);
	glUniform3f(object_uni_light_color[current_shader], light_color.x, light_color.y, light_color.z);
	glUniform3f(object_uni_light_direction[current_shader], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(object_uni_ambient[current_shader], ambient.x, ambient.y, ambient.z);
	glUniform3f(object_uni_camera_position[current_shader], camera_position.x, camera_position.y, camera_position.z);

	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);

	glBindVertexArray(0);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	rafgl_texture_t tmptex;
	tmptex.tex_id = fbo.tex_id;
	rafgl_texture_show(&tmptex, 1);
	glEnable(GL_DEPTH_TEST);
}

void main_state_cleanup(GLFWwindow* window, void* args) {
	glDeleteShader((GLuint) object_shader);
}
