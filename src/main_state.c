#include <math.h>
#include <ctype.h>
#include <glad/glad.h>
#include <rafgl.h>
#include <game_constants.h>
#include <main_state.h>
#include <effects.h>

#define BUTTON_HEIGHT 100
#define BUTTON_WIDTH RASTER_WIDTH >> 1
#define COLOR_REJECT rafgl_RGB(255, 0, 0)
#define COLOR_ACCEPT rafgl_RGB(0, 255, 0)
#define COMMAND_PATH "/Users/Lazar/Desktop/in.cmd"
#define IN_PATH "res/doge.png"
#define OUT_PATH "/Users/Lazar/Desktop/out-"
#define OUT_TYPE ".png"
#define OUT_PATH_LENGTH 256
#define SS_BUTTONS_PATH "res/buttons.png"
#define SS_BUTTONS_WIDTH 128
#define SS_BUTTONS_HEIGHT 64
#define ERROR_PARSE "Error occured while parsing command file"

static rafgl_texture_t texture;
static rafgl_button_t btn_reject, btn_accept;
// TODO static rafgl_spritesheet_t ss_buttons;
static int image_id, rejected, accepted;
static char out_file[OUT_PATH_LENGTH];

typedef struct effect_t {
	char* key;
	void (*fun)(int);
} effect_t;

static void command_parse()  {
	static int initialized = 0;

	if(!initialized) {
		FILE* fin = fopen(COMMAND_PATH, "r");

		if(!fin) {
			fprintf(stderr, ERROR_PARSE);
			return;
		}

		int i = 0, j = 0, k = 0;
		char c, arg[ARG_LENGTH] = {0};

		while((c = fgetc(fin)) != EOF) {
			if(isspace(c)) {
				strcpy(args[i][j++], arg);
				memset(arg, k = 0, sizeof(arg));

				if(c == '\n') {
					i++, j = 0;
				}
			}else {
				arg[k++] = c;
			}
		}

		if(fclose(fin) == EOF) {
			fprintf(stderr, ERROR_PARSE);
		}
	
		initialized = 1;
	}
}

static void command_run() {
	static effect_t effects[] = {
		{"ZOOMBLUR", &effect_zoomblur}
	};
	
	int i = -1, j;
	
	while(j = -1, **args[++i]) {
		while(*effects[++j].key) {
			if(!strcmp(*args[i], effects[j].key)) {
				effects[j].fun(i);
				break;
			}
		}
	}
}

static void image_init() {
	image_id++;
	
	accepted = 0;
	rejected = 0;	

	command_parse();
	command_run();
}

static void image_update() {
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

static void buttons_init() {
	rafgl_button_init(&btn_reject, 0, RASTER_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_REJECT);
	rafgl_button_init(&btn_accept, BUTTON_WIDTH, RASTER_HEIGHT - BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, COLOR_ACCEPT);

	// TODO rafgl_spritesheet_init(&ss_buttons, SS_BUTTONS_PATH, SS_BUTTONS_WIDTH, SS_BUTTONS_HEIGHT);
}

static void buttons_update(rafgl_game_data_t* game_data) {
	btn_reject.pressed = rafgl_button_check(&btn_reject, game_data);
	btn_accept.pressed = rafgl_button_check(&btn_accept, game_data);

	if(btn_accept.pressed && !accepted) {
        sprintf(out_file, OUT_PATH "%d" OUT_TYPE, image_id);
        rafgl_raster_save_to_png(&raster, out_file);
		accepted = 1;
	}

	rafgl_button_show(&raster, &btn_reject);
	rafgl_button_show(&raster, &btn_accept);

	// TODO rafgl_raster_draw_spritesheet(&raster, &ss_buttons, 0, 0, 11, 12);
}

void main_state_init(GLFWwindow *window, void* args) {
    rafgl_raster_load_from_image(&input, IN_PATH);

    rafgl_raster_init(&scaled, RASTER_WIDTH, RASTER_HEIGHT);
    rafgl_raster_init(&raster, RASTER_WIDTH, RASTER_HEIGHT);

	image_init();
	buttons_init();

    rafgl_texture_init(&texture);
}

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void* args) {
	image_update();
	buttons_update(game_data);
	rafgl_texture_load_from_raster(&texture, &raster);
}

void main_state_render(GLFWwindow *window, void* args) {
    rafgl_texture_show(&texture);
}

void main_state_cleanup(GLFWwindow *window, void* args) {
    rafgl_raster_cleanup(&raster);
    rafgl_texture_cleanup(&texture);
}
