#include <math.h>
#include <ctype.h>
#include <glad/glad.h>
#include <rafgl.h>
#include <game_constants.h>
#include <main_state.h>
#include <commands.h>

#define BUTTON_HEIGHT 100
#define BUTTON_WIDTH RASTER_WIDTH >> 1
#define COLOR_REJECT rafgl_RGB(255, 0, 0)
#define COLOR_ACCEPT rafgl_RGB(0, 255, 0)
#define COMMAND_PATH "/Users/Lazar/Desktop/in.cmd"
#define OUT_PATH "/Users/Lazar/Desktop/out-"
#define OUT_TYPE ".png"
#define SS_BUTTONS_PATH "res/buttons.png"
#define SS_BUTTON_WIDTH 2
#define SS_BUTTON_HEIGHT 1
#define ERROR_PARSE "Error occured while parsing command file"
#define COMMENT_CHAR '#'

static rafgl_texture_t texture;
static rafgl_button_t btn_reject, btn_accept;
static rafgl_spritesheet_t ss_buttons;
static int rejected, accepted;
static char out_file[PATH_LENGTH];

// TODO Document command arguments
static command_t commands[] = {
	{"LOAD", &command_load}, // Load image from file
	{"LINE", &command_line}, // Draw line
	{"CIRC", &command_circ}, // Draw circle
	{"RECT", &command_rect}, // Draw rectangle
	{"INST", &command_inst}, // Insert image from file
	{"ZBLR", &command_zblr} // Apply zoom blur on image
};

void command_parse()  {
	static int initialized = 0;

	if(!initialized) {
		FILE* fin = fopen(COMMAND_PATH, "r");

		if(!fin) {
			fprintf(stderr, ERROR_PARSE);
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
			fprintf(stderr, ERROR_PARSE);
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
	accepted = 0;
	rejected = 0;	
	
	command_parse();
}

void image_update() {
	int x, y;
    float xn, yn;
	
    for(y = 0; y < RASTER_HEIGHT; y++) {
        yn = 1.0f * y / RASTER_HEIGHT;

        for(x = 0; x < RASTER_WIDTH; x++) {
            xn = 1.0f * x / RASTER_WIDTH;
            
            pixel_at_m(output, x, y) = rafgl_bilinear_sample(&input, xn, yn);
        }
    }
}

void image_reload() {
	if(img_id++ < img_total) {
		image_init();
	}
}

void buttons_init() {
	rafgl_button_init(&btn_reject, 0, RASTER_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_REJECT);
	rafgl_button_init(&btn_accept, BUTTON_WIDTH, RASTER_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_ACCEPT);

	rafgl_spritesheet_init(&ss_buttons, SS_BUTTONS_PATH, SS_BUTTON_WIDTH, SS_BUTTON_HEIGHT);
}

void buttons_update(rafgl_game_data_t* game_data) {
	btn_reject.pressed = rafgl_button_check(&btn_reject, game_data);
	btn_accept.pressed = rafgl_button_check(&btn_accept, game_data);

	if(btn_accept.pressed && !accepted) {
        sprintf(out_file, OUT_PATH "%d" OUT_TYPE, img_id);
        rafgl_raster_save_to_png(&input, out_file);
		accepted = 1;
		
		image_reload();
	}

	if(btn_reject.pressed && !rejected) {
		image_reload();
	}

	rafgl_button_show(&output, &btn_reject);
	rafgl_button_show(&output, &btn_accept);

	rafgl_raster_draw_spritesheet(&output, &ss_buttons, 1, 0, (BUTTON_WIDTH >> 1) - ss_buttons.frame_width / 2, RASTER_HEIGHT - BUTTON_HEIGHT / 2 - ss_buttons.frame_height / 2);
	rafgl_raster_draw_spritesheet(&output, &ss_buttons, 0, 0, (BUTTON_WIDTH) + (BUTTON_WIDTH >> 1) - ss_buttons.frame_width / 2, RASTER_HEIGHT - BUTTON_HEIGHT / 2 - ss_buttons.frame_height / 2);
}

void main_state_init(GLFWwindow *window, void* args) {
	image_init();
	buttons_init();
    rafgl_raster_init(&output, RASTER_WIDTH, RASTER_HEIGHT);
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
