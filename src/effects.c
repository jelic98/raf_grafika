#include <rafgl.h>
#include <effects.h>

void effect_zoomblur(int cmd) {
	float intensity = atof(args[cmd][1]);
	int sample_count = atoi(args[cmd][2]);

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

