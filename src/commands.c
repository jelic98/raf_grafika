#include <rafgl.h>
#include <commands.h>

void command_load(int cmd) {
	char* path = args[cmd][1];
	
	if(!img_id) {
		while(images[img_total][0]) {
			img_total++;
		}

		strcpy(images[img_total], path);
	}

	rafgl_raster_load_from_image(&input, images[img_id]);
	// TODO rafgl_raster_init(&output, raster_width = input.width, raster_height = input.height);
	rafgl_raster_init(&output, raster_width, raster_height);
}

void command_line(int cmd) {
	int x0 = strtol(args[cmd][1], NULL, 10);
	int y0 = strtol(args[cmd][2], NULL, 10);
	int x1 = strtol(args[cmd][3], NULL, 10);
	int y1 = strtol(args[cmd][4], NULL, 10);
	uint32_t stroke = strtol(args[cmd][5], NULL, 16);

	rafgl_raster_draw_line(&input, x0, y0, x1, y1, rafgl_HEX(stroke));
}

void command_circ(int cmd) {
	int x0 = strtol(args[cmd][1], NULL, 10);
	int y0 = strtol(args[cmd][2], NULL, 10);
	int r = strtol(args[cmd][3], NULL, 10);
	uint32_t stroke = strtol(args[cmd][4], NULL, 16);
	uint32_t fill = strtol(args[cmd][5], NULL, 16);

	rafgl_raster_draw_circle(&input, x0, y0, r, rafgl_HEX(stroke), rafgl_HEX(fill));
}

void command_rect(int cmd) {
	int x0 = strtol(args[cmd][1], NULL, 10);
	int y0 = strtol(args[cmd][2], NULL, 10);
	int w = strtol(args[cmd][3], NULL, 10);
	int h = strtol(args[cmd][4], NULL, 10);
	uint32_t stroke = strtol(args[cmd][5], NULL, 16);
	uint32_t fill = strtol(args[cmd][6], NULL, 16);

	rafgl_raster_draw_rectangle(&input, x0, y0, w, h, rafgl_HEX(stroke), rafgl_HEX(fill));
}

void command_inst(int cmd) {
	char* path = args[cmd][1];
	int x0 = strtol(args[cmd][2], NULL, 10);
	int y0 = strtol(args[cmd][3], NULL, 10);
	int w = strtol(args[cmd][4], NULL, 10);
	int h = strtol(args[cmd][5], NULL, 10);

	rafgl_raster_t item;
	rafgl_raster_load_from_image(&item, path);
    
	int x, y;
	
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            pixel_at_m(input, x0 + x, y0 + y) = rafgl_point_sample(&item, 1.0f * x / w, 1.0f * y / h);
        }
    }
}

void command_zblr(int cmd) {
	float intensity = strtof(args[cmd][1], NULL);
	int sample_count = strtol(args[cmd][2], NULL, 10);

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
				
				factor += 2.0f / sample_count;
            }

            dst.r = r / sample_count;
            dst.g = g / sample_count;
            dst.b = b / sample_count;

            pixel_at_m(input, x, y) = dst;
        }
    }
}
