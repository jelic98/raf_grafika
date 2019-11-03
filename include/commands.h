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

rafgl_raster_t input, output, aux;
char images[IMGS_MAX][2][PATH_LENGTH];
char args[CMDS_MAX][ARGS_MAX][ARG_LENGTH];
int img_total, img_id;
int raster_width;
int raster_height;

void command_load(int);
void command_line(int);
void command_circ(int);
void command_rect(int);
void command_inst(int);
void command_rota(int);
void command_flpv(int);
void command_flph(int);
void command_edge(int);
void command_auto(int);
void command_ngtv(int);
void command_gray(int);
void command_blwh(int);
void command_vign(int);
void command_bblr(int);
void command_rblr(int);
void command_zblr(int);

#endif // COMMANDS_H_INCLUDED
