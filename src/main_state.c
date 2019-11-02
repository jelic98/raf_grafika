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
#define OUT_PATH "/Users/Lazar/Desktop/out-"
#define OUT_TYPE ".png"
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
static int interactive, rejected, accepted;
static char command_file[PATH_LENGTH], out_file[PATH_LENGTH];

// TODO Document command arguments
static command_t commands[] = {
	{"LOAD", &command_load}, // Load image from file
	{"LINE", &command_line}, // Draw line
	{"CIRC", &command_circ}, // Draw circle
	{"RECT", &command_rect}, // Draw rectangle
	{"INST", &command_inst}, // Insert image from file
	{"ZBLR", &command_zblr} // Apply zoom blur on image
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
	accepted = 0;
	rejected = 0;	
	
	command_parse();
}

void image_update() {
	int x, y;
    float xn, yn;
	
    for(y = 0; y < raster_height; y++) {
        yn = 1.0f * y / raster_height;

        for(x = 0; x < raster_width; x++) {
            xn = 1.0f * x / raster_width;
            
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
	rafgl_button_init(&btn_reject, 0, raster_height - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_REJECT);
	rafgl_button_init(&btn_accept, BUTTON_WIDTH, raster_height - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_ACCEPT);

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

	rafgl_raster_draw_spritesheet(&output, &ss_buttons, 1, 0, (BUTTON_WIDTH / 2) - ss_buttons.frame_width / 2, raster_height - BUTTON_HEIGHT / 2 - ss_buttons.frame_height / 2);
	rafgl_raster_draw_spritesheet(&output, &ss_buttons, 0, 0, (BUTTON_WIDTH) + (BUTTON_WIDTH / 2) - ss_buttons.frame_width / 2, raster_height - BUTTON_HEIGHT / 2 - ss_buttons.frame_height / 2);
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
