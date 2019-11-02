#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#define IMGS_MAX 10
#define CMDS_MAX 100
#define ARGS_MAX 5
#define ARG_LENGTH 64
#define PATH_LENGTH 256

typedef struct command_t {
	char* key;
	void (*fun)(int);
} command_t;

rafgl_raster_t input, scaled, raster;
char images[IMGS_MAX][PATH_LENGTH];
char args[CMDS_MAX][ARGS_MAX][ARG_LENGTH];

void command_in(int);
void command_zoomblur(int);

#endif // COMMANDS_H_INCLUDED
