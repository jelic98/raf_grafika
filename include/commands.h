#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#define IMGS_MAX 10
#define CMDS_MAX 100
#define ARGS_MAX 10
#define ARG_LENGTH 64
#define PATH_LENGTH 256

typedef struct command_t {
	char* key;
	void (*fun)(int);
} command_t;

rafgl_raster_t input, output;
char images[IMGS_MAX][PATH_LENGTH];
char args[CMDS_MAX][ARGS_MAX][ARG_LENGTH];
int img_total, img_id;
int raster_width;
int raster_height;

void command_load(int);
void command_line(int);
void command_circ(int);
void command_rect(int);
void command_inst(int);
void command_zblr(int);

#endif // COMMANDS_H_INCLUDED
