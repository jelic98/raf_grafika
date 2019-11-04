#include <math.h>
#include <ctype.h>
#include <glad/glad.h>
#include <rafgl.h>
#include <main_state.h>
#include <commands.h>

#define BUTTON_HEIGHT 100
#define BUTTON_WIDTH (raster_width >> 1)
#define COLOR_REJECT rafgl_RGB(255, 0, 0)
#define COLOR_ACCEPT rafgl_RGB(0, 255, 0)
#define SS_BUTTONS_PATH "res/buttons.png"
#define SS_BUTTON_WIDTH 2
#define SS_BUTTON_HEIGHT 1
#define ERROR_ARGS "Error occured while parsing arguments\n"
#define ERROR_COMMAND "Error occured while parsing command file\n"
#define FLAG_INTERACTIVE "-i"
#define COMMENT_CHAR '#'

static rafgl_texture_t texture;
static rafgl_button_t btn_reject, btn_accept;
static rafgl_spritesheet_t ss_buttons;
static int interactive, reject_pressed, accept_pressed, reject_released, accept_released;
static char command_file[PATH_LENGTH];

static command_t commands[] = {
	{"LOAD", &command_load},
	{"LINE", &command_line},
	{"CIRC", &command_circ},
	{"RECT", &command_rect},
	{"INST", &command_inst},
	{"ROTA", &command_rota},
	{"FLPV", &command_flpv},
	{"FLPH", &command_flph},
	{"EDGE", &command_edge},
	{"AUTO", &command_auto},
	{"NGTV", &command_ngtv},
	{"GRAY", &command_gray},
	{"BLWH", &command_blwh},
	{"VIGN", &command_vign},
	{"BBLR", &command_bblr},
	{"RBLR", &command_rblr},
	{"ZBLR", &command_zblr}
};

void args_parse(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(stderr, ERROR_ARGS);
		return;
	}

	strcpy(command_file, argv[1]);
	
	int i;

	for(i = 2; i < argc; i++) {
		if(!strcmp(argv[i], FLAG_INTERACTIVE)) {
			interactive = 1;
		}
	}
}

void command_parse()  {
	static int initialized = 0;

	if(!initialized) {
		FILE* fin = fopen(command_file, "r");

		if(!fin) {
			fprintf(stderr, ERROR_COMMAND);
			return;
		}

		int i = 0, j = 0;
		char c, arg[ARG_LENGTH] = {0};
		char* pa = arg;

		while((c = fgetc(fin)) != EOF) {
			if(isspace(c)) {
				strcpy(args[i][j++], arg);
				memset(pa = arg, 0, sizeof(arg));

				if(c == '\n') {
					i++, j = 0;
				}
			}else {
				*pa++ = c;
			}
		}

		if(fclose(fin) == EOF) {
			fprintf(stderr, ERROR_COMMAND);
		}
	
		initialized = 1;
	}

	int i = -1, j;
	
	while(j = -1, **args[++i]) {
		while(*commands[++j].key && **args[i] != COMMENT_CHAR) {
			if(!strcmp(*args[i], commands[j].key)) {
				commands[j].fun(i);
				break;
			}
		}
	}
}

void image_init() {	
	accept_pressed = 0;
	reject_pressed = 0;	
	
	accept_released = 0;
	reject_released = 0;	
	
	command_parse();
}

void image_update() {
	int x, y;
    float xn, yn;
	
    for(y = 0; y < raster_height; y++) {
        yn = 1.0f * y / raster_height;

        for(x = 0; x < raster_width; x++) {
            xn = 1.0f * x / raster_width;
            
            pixel_at_m(output, x, y) = rafgl_point_sample(&input, xn, yn);
		}
    }

	if(!interactive) {
		rafgl_raster_save_to_png(&input, images[img_id][1]);
		image_reload();
	}
}

void image_reload() {
	if(img_id++ < img_total) {
		image_init();
	}else {
		rafgl_game_destroy();
		exit(EXIT_SUCCESS);
	}
}

void buttons_init() {
	if(!interactive) {
		return;
	}
	
	rafgl_button_init(&btn_reject, 0, raster_height - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_REJECT);
	rafgl_button_init(&btn_accept, BUTTON_WIDTH, raster_height - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_ACCEPT);

	rafgl_spritesheet_init(&ss_buttons, SS_BUTTONS_PATH, SS_BUTTON_WIDTH, SS_BUTTON_HEIGHT);
}

void buttons_update(rafgl_game_data_t* game_data) {
	if(!interactive) {
		return;
	}
	
	btn_reject.pressed = rafgl_button_check(&btn_reject, game_data);
	btn_accept.pressed = rafgl_button_check(&btn_accept, game_data);

	accept_pressed |= btn_accept.pressed;
	reject_pressed |= btn_reject.pressed;

	accept_released |= !btn_accept.pressed && accept_pressed;
	reject_released |= !btn_reject.pressed && reject_pressed;

	if(accept_released) {
		rafgl_raster_save_to_png(&input, images[img_id][1]);
		image_reload();
	}

	if(reject_released) {
		image_reload();
	}

	rafgl_button_show(&output, &btn_reject);
	rafgl_button_show(&output, &btn_accept);

	rafgl_raster_draw_spritesheet(&output, &ss_buttons, 0, 0, (BUTTON_WIDTH) + (BUTTON_WIDTH / 2) - ss_buttons.frame_width / 2, raster_height - BUTTON_HEIGHT / 2 - ss_buttons.frame_height / 2, 0.8f);
	rafgl_raster_draw_spritesheet(&output, &ss_buttons, 1, 0, (BUTTON_WIDTH / 2) - ss_buttons.frame_width / 2, raster_height - BUTTON_HEIGHT / 2 - ss_buttons.frame_height / 2, 1.2f);
}

void main_state_init(GLFWwindow *window, void* args) {
	image_init();
	buttons_init(); 
	rafgl_texture_init(&texture);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void* args) {
	image_update();
	buttons_update(game_data);
	rafgl_texture_load_from_raster(&texture, &output);
}

void main_state_render(GLFWwindow *window, void* args) {
    rafgl_texture_show(&texture);
}

void main_state_cleanup(GLFWwindow *window, void* args) {
    rafgl_raster_cleanup(&output);
    rafgl_texture_cleanup(&texture);
}
