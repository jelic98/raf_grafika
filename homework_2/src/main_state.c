#include <game_constants.h>
#include <glad/glad.h>
#include <main_state.h>
#include <math.h>
#include <rafgl.h>
#include <stdlib.h>
#include <time.h>

#define TOTAL_STAGES 5
#define TOTAL_UNIFORMS 10
#define TOTAL_MESHES 1
#define SSAO_RADIUS 0.6f
#define SSAO_BIAS 0.003f
#define SSAO_POWER 0.8f
#define LENGTH_NAME 128
#define FIELD_OF_VIEW 75.0f

static struct pipeline_stage {
	GLuint shader;
	GLuint uni[TOTAL_UNIFORMS];
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

static int current_shader = 0;
static int current_mesh = 0;

static float angle_v = 0.0f;
static float angle_h = -M_PIf * 0.5f;
static float angle_speed = M_PIf * 0.003f;
static float angle_model = -M_PIf;
static float speed_move = 5.0f;

static int flag_rotate = 0;

static GLuint smp_position, smp_normal, smp_noise, smp_kernel;
static GLuint tex_position, tex_normal, tex_noise, tex_kernel;

void init_preprocess(int width, int height) {
	stages[0].shader = rafgl_program_create_from_name("pipeline/preprocess");
	stages[0].uni[0] = glGetUniformLocation(stages[0].shader, "uni_m");
	stages[0].uni[1] = glGetUniformLocation(stages[0].shader, "uni_vp");
	stages[0].fbo = rafgl_framebuffer_simple_create(width, height);    
	
	glBindFramebuffer(GL_FRAMEBUFFER, stages[0].fbo.fbo_id);
    
	glGenTextures(1, &tex_position);
    glBindTexture(GL_TEXTURE_2D, tex_position);
   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_position, 0);

	glGenTextures(1, &tex_normal);
    glBindTexture(GL_TEXTURE_2D, tex_normal);
   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_normal, 0);
}

void init_ssao(int width, int height) {
	stages[1].shader = rafgl_program_create_from_name("pipeline/ssao");
	stages[1].uni[0] = glGetUniformLocation(stages[1].shader, "uni_m");
	stages[1].uni[1] = glGetUniformLocation(stages[1].shader, "uni_vp");
	stages[1].uni[2] = glGetUniformLocation(stages[1].shader, "uni_p");
	stages[1].uni[3] = glGetUniformLocation(stages[1].shader, "uni_radius");
	stages[1].uni[4] = glGetUniformLocation(stages[1].shader, "uni_bias");
	stages[1].uni[5] = glGetUniformLocation(stages[1].shader, "uni_power");
	stages[1].fbo = rafgl_framebuffer_simple_create(width, height);	

	glBindFramebuffer(GL_FRAMEBUFFER, stages[1].fbo.fbo_id);

	smp_position = glGetUniformLocation(stages[1].shader, "smp_position");
	smp_normal = glGetUniformLocation(stages[1].shader, "smp_normal");
	smp_noise = glGetUniformLocation(stages[1].shader, "smp_noise");
	smp_kernel = glGetUniformLocation(stages[1].shader, "smp_kernel");

	glUniform1i(smp_position, 0);
	glUniform1i(smp_normal, 1);
	glUniform1i(smp_noise, 2);
	glUniform1i(smp_kernel, 3);

	int i;

	float noise[16][3];
	float kernel[16][3];
	
	for(i = 0; i < 16; i++) {
    	noise[i][0] = (256 * ((float)rand()) / RAND_MAX);
    	noise[i][1] = (256 * ((float)rand()) / RAND_MAX);
    	noise[i][2] = 0.0f;
    	
		kernel[i][0] = -1 + 2 * ((float)rand()) / RAND_MAX;
    	kernel[i][1] = -1 + 2 * ((float)rand()) / RAND_MAX;
    	kernel[i][2] = ((float) rand()) / RAND_MAX;
	}
	
	glGenTextures(1, &smp_noise);
    glBindTexture(GL_TEXTURE_2D, smp_noise);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_BYTE, noise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenTextures(1, &smp_kernel);
    glBindTexture(GL_TEXTURE_2D, smp_kernel);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, kernel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void init_blur(int width, int height) {
	stages[2].shader = rafgl_program_create_from_name("pipeline/blur");
	stages[2].uni[0] = glGetUniformLocation(stages[2].shader, "uni_m");
	stages[2].uni[1] = glGetUniformLocation(stages[2].shader, "uni_vp");
	stages[2].fbo = rafgl_framebuffer_simple_create(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, stages[2].fbo.fbo_id);
}

void init_light(int width, int height) {
	stages[3].shader = rafgl_program_create_from_name("pipeline/light");
	stages[3].uni[0] = glGetUniformLocation(stages[3].shader, "uni_m");
	stages[3].uni[1] = glGetUniformLocation(stages[3].shader, "uni_vp");
	stages[3].fbo = rafgl_framebuffer_simple_create(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, stages[3].fbo.fbo_id);
}

void init_postprocess(int width, int height) {
	stages[4].shader = rafgl_program_create_from_name("pipeline/postprocess");
	stages[4].uni[0] = glGetUniformLocation(stages[4].shader, "uni_m");
	stages[4].uni[1] = glGetUniformLocation(stages[4].shader, "uni_vp");
	stages[4].fbo = rafgl_framebuffer_simple_create(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, stages[4].fbo.fbo_id);
}

void render_preprocess() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[0].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(stages[0].shader);
	glUniformMatrix4fv(stages[0].uni[0], 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(stages[0].uni[1], 1, GL_FALSE, (void*) view_projection.m);

	glBindVertexArray(meshes[current_mesh].vao_id);
	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);
}

void render_ssao() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[1].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(stages[1].shader);
	glUniformMatrix4fv(stages[1].uni[0], 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(stages[1].uni[1], 1, GL_FALSE, (void*) view_projection.m);
	glUniformMatrix4fv(stages[1].uni[2], 1, GL_FALSE, (void*) projection.m);
	glUniform1f(stages[1].uni[3], SSAO_RADIUS);
	glUniform1f(stages[1].uni[4], SSAO_BIAS);
	glUniform1f(stages[1].uni[5], SSAO_POWER);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_position);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_normal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_noise);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_kernel);

	glBindVertexArray(meshes[current_mesh].vao_id);
	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);
}

void render_blur() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[2].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(stages[2].shader);
	glUniformMatrix4fv(stages[2].uni[0], 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(stages[2].uni[1], 1, GL_FALSE, (void*) view_projection.m);

	glBindVertexArray(meshes[current_mesh].vao_id);
	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);
}

void render_light() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[3].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(stages[3].shader);
	glUniformMatrix4fv(stages[3].uni[0], 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(stages[3].uni[1], 1, GL_FALSE, (void*) view_projection.m);

	glBindVertexArray(meshes[current_mesh].vao_id);
	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);
}

void render_postprocess() {
	glBindFramebuffer(GL_FRAMEBUFFER, stages[4].fbo.fbo_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(stages[4].shader);
	glUniformMatrix4fv(stages[4].uni[0], 1, GL_FALSE, (void*) model.m);
	glUniformMatrix4fv(stages[4].uni[1], 1, GL_FALSE, (void*) view_projection.m);

	// blend on

	glBindVertexArray(meshes[current_mesh].vao_id);
	glDrawArrays(GL_TRIANGLES, 0, meshes[current_mesh].vertex_count);
	
	// blend off
}

void main_state_init(GLFWwindow* window, void* args, int width, int height) {
	srand(time(NULL));

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
	init_blur(width, height);
	init_light(width, height);
	init_postprocess(width, height);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
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

	for(i = 0; i < TOTAL_STAGES; i++) {
		if(current_shader != i && game_data->keys_pressed[RAFGL_KEY_0 + i]) {
			current_shader = i;
		}
	}

	model = m4_identity();
	model = m4_rotation_y(angle_model += delta_time * flag_rotate);
	model = m4_mul(model, m4_translation(vec3(0.0f, sinf(angle_model) * 0.5f, 0.0f)));

	view = m4_look_at(camera_dir, v3_add(camera_dir, aim_dir), camera_up);

	float raster_aspect = ((float) (game_data->raster_width)) / game_data->raster_height;
	projection = m4_perspective(FIELD_OF_VIEW, raster_aspect, 0.1f, 100.0f);
	view_projection = m4_mul(projection, view);
}

void main_state_render(GLFWwindow* window, void* args) {
	glDepthMask(GL_FALSE);
	glUseProgram(skybox_shader);
	glUniformMatrix4fv(skybox_uni_v, 1, GL_FALSE, (void*) view.m);
	glUniformMatrix4fv(skybox_uni_p, 1, GL_FALSE, (void*) projection.m);
	glBindVertexArray(skybox_mesh.vao_id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex.tex_id);
	glDrawArrays(GL_TRIANGLES, 0, skybox_mesh.vertex_count);
	glDepthMask(GL_TRUE);

	render_preprocess();
	render_ssao();
	render_blur();
	render_light();
	render_postprocess();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	rafgl_texture_t tex;
	tex.tex_id = stages[current_shader].fbo.tex_id;
	rafgl_texture_show(&tex, 1);
}

void main_state_cleanup(GLFWwindow* window, void* args) {
	int i;

	for(i = 0; i < TOTAL_STAGES; i++) {
		glDeleteShader((GLuint) stages[i].shader);
	}
}
