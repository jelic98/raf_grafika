#ifndef EFFECTS_H_INCLUDED
#define EFFECTS_H_INCLUDED

#define CMDS_MAX 100
#define ARGS_MAX 5
#define ARG_LENGTH 16

rafgl_raster_t input, scaled, raster;
char args[CMDS_MAX][ARGS_MAX][ARG_LENGTH];

void effect_zoomblur(int);

#endif // EFFECTS_H_INCLUDED
