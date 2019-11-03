#include <rafgl.h>
#include <commands.h>

void command_load(int cmd) {
	char* src = args[cmd][1];
	char* dst = args[cmd][2];
	
	if(!img_id) {
		while(**images[img_total]) {
			img_total++;
		}

		strcpy(images[img_total][0], src);
		strcpy(images[img_total][1], dst);
	}

	rafgl_raster_load_from_image(&input, images[img_id][0]);
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
	int xc = strtol(args[cmd][1], NULL, 10);
	int yc = strtol(args[cmd][2], NULL, 10);
	int r = strtol(args[cmd][3], NULL, 10);
	uint32_t stroke = strtol(args[cmd][4], NULL, 16);
	uint32_t fill = strtol(args[cmd][5], NULL, 16);

	rafgl_raster_draw_circle(&input, xc, yc, r, rafgl_HEX(stroke), rafgl_HEX(fill));
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

	rafgl_raster_load_from_image(&aux, path);
    
	int x, y;
	
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            pixel_at_m(input, x0 + x, y0 + y) = rafgl_point_sample(&aux, 1.0f * x / w, 1.0f * y / h);
        }
    }
}

void command_rota(int cmd) {
	int x, y;

	// TODO Update raster dimensions for non-square images

	rafgl_raster_init(&aux, input.width, input.height);	
 
	for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            pixel_at_m(aux, x, y) = pixel_at_m(input, y, input.width - x - 1);
        }
    }

	for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            pixel_at_m(input, x, y) = pixel_at_m(aux, x, y);
        }
    }
}

void command_flpv(int cmd) {
	int x, y;

	rafgl_pixel_rgb_t left, right;
	
    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width / 2; x++) {
            left = pixel_at_m(input, x, y);
            right = pixel_at_m(input, input.width - x - 1, y);

            pixel_at_m(input, input.width - x - 1, y) = left;
            pixel_at_m(input, x, y) = right;
        }
    }
}

void command_flph(int cmd) {
	int x, y;

	rafgl_pixel_rgb_t top, bottom;
	
    for(y = 0; y < input.height / 2; y++) {
        for(x = 0; x < input.width; x++) {
            top = pixel_at_m(input, x, y);
            bottom = pixel_at_m(input, x, input.height - y - 1);

            pixel_at_m(input, x, input.height - y - 1) = top;
            pixel_at_m(input, x, y) = bottom;
        }
    }
}

void command_edge(int cmd) {
	int x, y, offset_x, offset_y, delta, delta_max;
	
	rafgl_pixel_rgb_t src, dst, tmp;
	
    for(y = 0; y < input.height - 1; y++) {
        for(x = 0; x < input.width - 1; x++) { 
			src = pixel_at_m(input, x, y);

			delta_max = 0;

			for(offset_x = -1; offset_x < 2; offset_x++) {
				for(offset_y = -1; offset_y < 2; offset_y++) {
					tmp = pixel_at_m(input, x + offset_x, y + offset_y);
					
					delta = rafgl_distance1D(rafgl_brightness(src), rafgl_brightness(tmp));
			
					if(delta > delta_max) {
						delta_max = delta;
					}
				}
			}
 
            dst.r = rafgl_saturate(delta << 2);
            dst.g = rafgl_saturate(delta << 2);
            dst.b = rafgl_saturate(delta << 2);

			pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_auto(int cmd) {
	int histogram_brightness[256];
	int histogram_red[256];
	int histogram_green[256];
	int histogram_blue[256];

	int x, y;
	float scale;
	float cutoff = 0.005f;
    int limit = input.width * input.height * cutoff;
    int left_bound = 0, right_bound = 255;
    int sum = histogram_brightness[0];
	
	rafgl_pixel_rgb_t src, dst;

	for(x = 0; x < 256; x++) {
        histogram_red[x] = 0;
        histogram_green[x] = 0;
        histogram_blue[x] = 0;
        histogram_brightness[x] = 0;
    }
    
	for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            src = pixel_at_m(input, x, y);
            histogram_brightness[rafgl_brightness(src)] += 1;
        }
    }

	for(x = 1; x < 256; x++) {
		sum += histogram_brightness[x];
		
		if(sum >= limit) {
			left_bound = x - 1;
			break;
		}
	}

	sum = histogram_brightness[255];

	for(x = 254; x >= 0; x--) {
		sum += histogram_brightness[x];
		
		if(sum >= limit) {
			right_bound = x + 1;
			break;
		}
	}

	scale = 255.0f / (right_bound - left_bound);
	
    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            src = pixel_at_m(input, x, y);

            dst.r = rafgl_saturate((src.r - left_bound) * scale);
            dst.g = rafgl_saturate((src.g - left_bound) * scale);
            dst.b = rafgl_saturate((src.b - left_bound) * scale);
 
   			pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_ngtv(int cmd) {
	int x, y;
	
	rafgl_pixel_rgb_t src, dst;
	
    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            src = pixel_at_m(input, x, y);
			
			dst.r = 255 - src.r;
			dst.g = 255 - src.g;
			dst.b = 255 - src.b;
			
			pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_gray(int cmd) {
	int x, y, brightness;
	
	rafgl_pixel_rgb_t src, dst;
	
    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            src = pixel_at_m(input, x, y);
			
			brightness = rafgl_brightness(src);

            dst.r = brightness;
            dst.g = brightness;
            dst.b = brightness;

			pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_blwh(int cmd) {
	int threshold = strtol(args[cmd][1], NULL, 10);
	
	int x, y, brightness;
	
	rafgl_pixel_rgb_t src, dst;
	
    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            src = pixel_at_m(input, x, y);
			
			brightness = rafgl_brightness(src);

			if(brightness > threshold) {
 				dst.r = 255;
            	dst.g = 255;
            	dst.b = 255;
			}else {
 				dst.r = 0;
            	dst.g = 0;
            	dst.b = 0;
			}
    
   			pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_vign(int cmd) {
	float xc = strtof(args[cmd][1], NULL);
	float yc = strtof(args[cmd][2], NULL);
	float factor = strtof(args[cmd][3], NULL);
	
	int x, y;
	float xs, ys, dist;
	float r = sqrtf(2) * 0.5f;
	
	rafgl_pixel_rgb_t src, dst;
	
    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            src = pixel_at_m(input, x, y);

            xs = 1.0f * x / input.width;
            ys = 1.0f * y / input.height;

            dist = rafgl_distance2D(xs, ys, xc, yc) / r;

            dst.r = rafgl_saturate(src.r * (1.0f - dist * factor));
            dst.g = rafgl_saturate(src.g * (1.0f - dist * factor));
            dst.b = rafgl_saturate(src.b * (1.0f - dist * factor));
 
   			pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_bblr(int cmd) {
	int radius = strtol(args[cmd][1], NULL, 10);

	int x, y, xs, ys, offset;
	int sample_count = 2 * radius + 1;
	float r, g, b;
    rafgl_pixel_rgb_t src, dst;

    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            r = g = b = 0;
            
            for(offset = -radius; offset <= radius; offset++) {
				xs = rafgl_clampi(x + offset, 0, input.width - 1);
				ys = y;

				src = pixel_at_m(input, xs, ys);
                
				r += src.r;
                g += src.g;
                b += src.b;
            }

            dst.r = r / sample_count;
            dst.g = g / sample_count;
            dst.b = b / sample_count;

            pixel_at_m(input, x, y) = dst;
        }
    }

    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            r = g = b = 0;
            
            for(offset = -radius; offset <= radius; offset++) {
				xs = x;
				ys = rafgl_clampi(y + offset, 0, input.height - 1);

				src = pixel_at_m(input, xs, ys);
                
				r += src.r;
                g += src.g;
                b += src.b;
            }

            dst.r = r / sample_count;
            dst.g = g / sample_count;
            dst.b = b / sample_count;

            pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_rblr(int cmd) { 
	float intensity = strtof(args[cmd][1], NULL);

	int i, x, y, xs, ys, xc = input.width / 2, yc = input.height / 2;
	int sample_count = input.width * 0.1;
	float factor, angle, distance, r, g, b;
    rafgl_pixel_rgb_t src, dst;

    for(y = 0; y < input.height; y++) {
        for(x = 0; x < input.width; x++) {
            r = g = b = 0;
            
			factor = 0.0f;
            angle = atan2(y - yc, x - xc);
            distance = rafgl_distance2D(xc, yc, x, y);

            for(i = 0; i < sample_count; i++) {
                xs = rafgl_clampi(xc + cosf(angle + factor * intensity * M_2_PI) * distance, 0, input.width - 1);
                ys = rafgl_clampi(yc + sinf(angle + factor * intensity * M_2_PI) * distance, 0, input.height - 1);
                
				src = pixel_at_m(input, xs, ys);
                
				r += src.r;
                g += src.g;
                b += src.b;
				
				factor += 1.0f / sample_count;
            }

            dst.r = r / sample_count;
            dst.g = g / sample_count;
            dst.b = b / sample_count;

            pixel_at_m(input, x, y) = dst;
        }
    }
}

void command_zblr(int cmd) {
	float intensity = strtof(args[cmd][1], NULL);

	int i, x, y, xs, ys, xd, yd, xc = input.width / 2, yc = input.height / 2;
	int sample_count = input.width * 0.1;
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
