#include <game_constants.h>
#include <glad/glad.h>
#include <main_state.h>
#include <math.h>
#include <rafgl.h>

#define TOTAL_SHADERS 1
#define TOTAL_STAGES 5
#define TOTAL_MESHES 1
#define LENGTH_NAME 128
#define FIELD_OF_VIEW 75.0f

static struct pipeline_stage {
	GLuint shader;
	GLuint uni_m;
	GLuint uni_vp;
	rafgl_framebuffer_simple_t fbo;
} stages[TOTAL_STAGES];

static GLuint skybox_shader;
static GLuint skybox_uni_p, skybox_uni_v;

static rafgl_texture_t skybox_tex;
static rafgl_meshPUN_t skybox_mesh;

static mat4_t model, view, projection, view_projection;

static vec3_t camera_dir = vec3m(0.0f, 1.0f, 6.5f);
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

void init_preprocess(int width, int height) {
	stages[0].shader = rafgl_program_create_from_name("pipeline/preprocess");

	stages[0].uni_m = glGetUniformLocation(stages[0].shader, "uni_m");
	stages[0].uni_vp = glGetUniformLocation(stages[0].shader, "uni_vp");

	stages[0].fbo = rafgl_framebuffer_simple_create(width, height);
}

void init_ssao(int width, int height) {
	stages[1].shader = rafgl_program_create_from_name("pipeline/ssao");

	stages[1].uni_m = glGetUniformLocation(stages[1].shader, "uni_m");
	stages[1].uni_vp = glGetUniformLocation(stages[1].shader, "uni_vp");

	stages[1].fbo = rafgl_framebuffer_simple_create(width, height);
}

void init_blur(int width, int height) {
	stages[2].shader = rafgl_program_create_from_name("pipeline/blur");

	stages[2].uni_m = glGetUniformLocation(stages[2].shader, "uni_m");
	stages[2].uni_vp = glGetUniformLocation(stages[2].shader, "uni_vp");

	stages[2].fbo = rafgl_framebuffer_simple_create(width, height);
}

void init_light(int width, int height) {
	stages[3].shader = rafgl_program_create_from_name("pipeline/light");

	stages[3].uni_m = glGetUniformLocation(stages[3].shader, "uni_m");
	stages[3].uni_vp = glGetUniformLocation(stages[3].shader, "uni_vp");

	stages[3].fbo = rafgl_framebuffer_simple_create(width, height);
}

void init_postprocess(int width, int height) {
	//stages[4].shader = rafgl_program_create_from_name("pipeline/postprocess");

	stages[4].uni_m = glGetUniformLocation(stages[4].shader, "uni_m");
	stages[4].uni_vp = glGetUniformLocation(stages[4].shader, "uni_vp");

	stages[4].fbo = rafgl_framebuffer_simple_create(width, height);
}

void main_state_init(GLFWwindow* window, void* args, int width, int height) {
	rafgl_log_fps(RAFGL_TRUE);

	rafgl_texture_load_cubemap_named(&skybox_tex, "above_the_sea", "jpg");

	skybox_shader = rafgl_program_create_from_name("skybox");

	skybox_uni_p = glGetUniformLocation(skybox_shader, "uni_p");
	skybox_uni_v = glGetUniformLocation(skybox_shader, "uni_v");

	rafgl_meshPUN_init(&skybox_mesh);
	rafgl_meshPUN_load_cube(&skybox_mesh, 1.0f);

	int i;

	for(i = 0; i < 1; i++) {
		rafgl_meshPUN_init(meshes + i);
		rafgl_meshPUN_load_from_OBJ(meshes + i, mesh_names[i]);
	}

	init_preprocess(width, height);
	init_ssao(width, height);
	//init_blur(width, height);
	//init_light(width, height);
	init_postprocess(width, height);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

void main_state_update(GLFWwindow* window, float delta_time, rafgl_game_data_t* game_data, void* args) {
	if(game_data->keys_down[RAFGL_KEY_UP]) {
		angle_v -= angle_speed;
	} else if(game_data->keys_down[RAFGL_KEY_DOWN]) {
		angle_v += angle_speed;
	}

	if(game_data->keys_down[RAFGL_KEY_LEFT]) {
		angle_h -= angle_speed;
	} else if(game_data->keys_down[RAFGL_KEY_RIGHT]) {
		angle_h += angle_speed;
	}

	aim_dir = v3_norm(vec3(cosf(angle_h), angle_v, sinf(angle_h)));
	vec3_t right = v3_cross(aim_dir, vec3(0.0f, 1.0f, 0.0f));

	if(game_data->keys_down['W']) {
		camera_dir = v3_add(camera_dir, v3_muls(aim_dir, speed_move * delta_time));
	} else if(game_data->keys_down['S']) {
		camera_dir = v3_add(camera_dir, v3_muls(aim_dir, -speed_move * delta_time));
	}

	if(game_data->keys_down['D']) {
		camera_dir = v3_add(camera_dir, v3_muls(right, speed_move * delta_time));
	} else if(game_data->keys_down['A']) {
		camera_dir = v3_add(camera_dir, v3_muls(right, -speed_move * delta_time));
	}

	if(game_data->keys_down[RAFGL_KEY_SPACE]) {
		camera_dir.y += 1.0f * delta_time * speed_move;
	} else if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) {
		camera_dir.y -= 1.0f * delta_time * speed_move;
	}

	if(game_data->keys_pressed['R']) {
		flag_rotate = !flag_rotate;
	}

	if(game_data->keys_down[RAFGL_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	int i;

	for(i = 0; i < TOTAL_SHADERS; i++) {
		if(current_shader != i && game_data->keys_pressed[RAFGL_KEY_0 + i]) {
			current_shader = i;
		}
	}

	model = m4_identity();
	model = m4_rotation_y(angle_model += delta_time * flag_rotate);
	model = m4_mul(model, m4_translation(vec3(0.0f, sinf(angle_model) * 0.45, 0.0f)));

	view = m4_look_at(camera_dir, v3_add(camera_dir, aim_dir), camera_up);

	float raster_aspect = ((float) (game_data->raster_width)) / game_data->raster_height;
	projection = m4_perspective(FIELD_OF_VIEW, raster_aspect, 0.1f, 100.0f);
	view_projection = m4_mul(projection, view);
}

void stage_preprocess() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[4].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	glUseProgram(skybox_shader);
	glUniformMatrix4fv(skybox_uni_v, 1, GL_FALSE, (void*) view.m);
	glUniformMatrix4fv(skybox_uni_p, 1, GL_FALSE, (void*) projection.m);

	glBindVertexArray(skybox_mesh.vao_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);

	glDrawArrays(GL_TRIANGLES, 0, skybox_mesh.vertex_count);
	glDepthMask(GL_TRUE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glEnable(GL_DEPTH_TEST);
	glUseProgram(stages[0].shader);
}

void stage_ssao() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[1].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(stages[1].shader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void stage_blur() {

}

void stage_light() {

}

void stage_postprocess() {
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(meshes[current_mesh].vao_id);

	glUniformMatrix4fv(stages[4].uni_m, 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(stages[4].uni_vp, 1, GL_FALSE, (void*) view_projection.m);

	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);

	glBindVertexArray(0);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	rafgl_texture_t tex;
	tex.tex_id = stages[4].fbo.tex_id;
	rafgl_texture_show(&tex, 1);

	glEnable(GL_DEPTH_TEST);
}

void main_state_render(GLFWwindow* window, void* args) {
	stage_preprocess();
	stage_ssao();
	//stage_blur();
	//stage_light();
	stage_postprocess();
}

void main_state_cleanup(GLFWwindow* window, void* args) {
	int i;

	for(i = 0; i < TOTAL_SHADERS; i++) {
		glDeleteShader((GLuint) stages[i].shader);
	}
}
