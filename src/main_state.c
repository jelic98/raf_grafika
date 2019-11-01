#include <math.h>
#include <glad/glad.h>
#include <rafgl.h>
#include <game_constants.h>
#include <main_state.h>

#define BUTTON_HEIGHT 100
#define BUTTON_WIDTH RASTER_WIDTH >> 1
#define COLOR_REJECT rafgl_RGB(255, 0, 0)
#define COLOR_ACCEPT rafgl_RGB(0, 255, 0)
#define COMMAND_PATH "/Users/Lazar/Desktop/in.cmd"
#define IN_PATH "res/doge.jpg"
#define OUT_PATH "/Users/Lazar/Desktop/out-"
#define OUT_TYPE ".png"
#define OUT_PATH_LENGTH 256
#define ARGS_MAX 5
#define ARG_LENGTH 16

static rafgl_raster_t input, scaled, raster;
static rafgl_texture_t texture;
static rafgl_button_t btn_reject, btn_accept;
static int image_id, rejected, accepted;
static char file[OUT_PATH_LENGTH];

void command_parse(char* args[]) {
	args[0] = "ZOOMBLUR";
	args[1] = "0.2";
	args[2] = "10";
}

void effect_zoomblur(char* args[]) {
	float intensity = atof(args[1]);
	int sample_count = atoi(args[2]);

	int i, x, y, xs, ys, xd, yd, xc = input.width / 2, yc = input.height / 2;

	float factor, r, g, b;

    rafgl_pixel_rgb_t src, dst;

    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            xd = x - xc;
            yd = y - yc;

            r = g = b = 0;
            
			factor = -1.0f;

            for(i = 0; i < sample_count; i++) {
                xs = rafgl_clampi(xc + (1.0f + factor * intensity) * xd, 0, input.width - 1);
                ys = rafgl_clampi(yc + (1.0f + factor * intensity) * yd, 0, input.height - 1);
				
				src = pixel_at_m(input, xs, ys);
                
				r += src.r;
                g += src.g;
                b += src.b;
				
				factor += 1.2f / sample_count;
            }

            dst.r = r / sample_count;
            dst.g = g / sample_count;
            dst.b = b / sample_count;

            pixel_at_m(input, x, y) = dst;
        }
    }
}

void image_init() {
	image_id++;
	
	accepted = 0;
	rejected = 0;	
	
	char args[ARGS_MAX][ARG_LENGTH];
	command_parse(args);

	effect_zoomblur(args);
}

void image_update() {
	int x, y;
    float xn, yn;

    for(y = 0; y < RASTER_HEIGHT; y++) {
        yn = 1.0f * y / RASTER_HEIGHT;

        for(x = 0; x < RASTER_WIDTH; x++) {
            xn = 1.0f * x / RASTER_WIDTH;
            
            pixel_at_m(raster, x, y) = rafgl_bilinear_sample(&input, xn, yn);
        }
    }
}

void buttons_show(rafgl_game_data_t* game_data) {
	btn_reject.pressed = rafgl_button_check(&btn_reject, game_data);
	btn_accept.pressed = rafgl_button_check(&btn_accept, game_data);

	if(btn_accept.pressed && !accepted) {
        sprintf(file, OUT_PATH "%d" OUT_TYPE, image_id);
        rafgl_raster_save_to_png(&raster, file);
		accepted = 1;
	}

	rafgl_button_show(&raster, &btn_reject);
	rafgl_button_show(&raster, &btn_accept);
}

void main_state_init(GLFWwindow *window, void* args) {
    rafgl_raster_load_from_image(&input, IN_PATH);

    rafgl_raster_init(&scaled, RASTER_WIDTH, RASTER_HEIGHT);
    rafgl_raster_init(&raster, RASTER_WIDTH, RASTER_HEIGHT);

	image_init();

	rafgl_button_init(&btn_reject, 0, RASTER_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_REJECT);
	rafgl_button_init(&btn_accept, BUTTON_WIDTH, RASTER_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_ACCEPT);

    rafgl_texture_init(&texture);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void* args) {
	image_update();
	buttons_show(game_data);
	rafgl_texture_load_from_raster(&texture, &raster);
}

void main_state_render(GLFWwindow *window, void* args) {
    rafgl_texture_show(&texture);
}

void main_state_cleanup(GLFWwindow *window, void* args) {
    rafgl_raster_cleanup(&raster);
    rafgl_texture_cleanup(&texture);
}
