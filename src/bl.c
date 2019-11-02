#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include <rafgl.h>

static rafgl_raster_t doge, blured_doge, blured_upscaled_doge;
static rafgl_raster_t raster, raster2;
static rafgl_texture_t texture;

static int raster_width = 800, raster_height = 800;


/* primer 9 - zoom blur efekat */


void main_state_init(GLFWwindow *window, void *args)
{
    /* inicijalizacija */
    /* raster init nam nije potreban ako radimo load from image */
    rafgl_raster_load_from_image(&doge, "res/images/doge.png");

    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);
    rafgl_raster_init(&blured_upscaled_doge, raster_width, raster_height);
    rafgl_raster_init(&blured_doge, doge.width, doge.height);

    int x, y, xs, ys, i;
    float xn, yn;

    int sample_count = 50;
    /* vektor pravca po kojem blurujemo (intenzited takodje zavisi od intenziteta vektora) */
    int dx, dy;
    float factor, intensity = 0.2f;

    int cx = doge.width / 2, cy = doge.height / 2;



    float r, g, b;

    rafgl_pixel_rgb_t sampled, resulting;

    for(y = 0; y < doge.height; y++)
    {
        for(x = 0; x < doge.width; x++)
        {
            dx = x - cx;
            dy = y - cy;
            r = g = b = 0;
            factor = -1.0f;
            for(i = 0; i < sample_count; i++)
            {
                /* ramdpmujemo u okolini */
                xs = rafgl_clampi(cx + (1.0f + factor * intensity) * dx, 0, doge.width - 1);
                ys = rafgl_clampi(cy + (1.0f + factor * intensity) * dy, 0, doge.height - 1);
                sampled = pixel_at_m(doge, xs, ys);
                r += sampled.r;
                g += sampled.g;
                b += sampled.b;
                factor += 2.0f / sample_count;

            }

            resulting.r = r / sample_count;
            resulting.g = g / sample_count;
            resulting.b = b / sample_count;

            pixel_at_m(blured_doge, x, y) = resulting;
        }
    }


    /* upscale blurovane slike */
    for(y = 0; y < raster_height; y++)
    {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++)
        {
            xn = 1.0f * x / raster_width;
            pixel_at_m(blured_upscaled_doge, x, y) = rafgl_bilinear_sample(&blured_doge, xn, yn);
        }
    }

    rafgl_texture_init(&texture);
}


int pressed, location = 0;
float selector = 0.5f;


void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    /* hendluj input */
    if(game_data->is_lmb_down && game_data->is_rmb_down)
    {
        pressed = 1;
        location = rafgl_clampi(game_data->mouse_pos_y, 0, raster_height - 1);
        selector = 1.0f * location / raster_height;
    }
    else
    {
        pressed = 0;
    }


    /* izmeni raster */

    int x, y, cx = raster_width / 2, cy = raster_height / 2;


    float xn, yn, magn = 2.0f * selector, radius = rafgl_distance2D(0, 0, cx, cy);

    float angle, distance, distance_max = 0, distance_n;

    rafgl_pixel_rgb_t sampled, sampled2, resulting, resulting2;


    int xnew, ynew;


    for(y = 0; y < raster_height; y++)
    {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++)
        {
            xn = 1.0f * x / raster_width;


            sampled = pixel_at_m(blured_upscaled_doge, x, y);
            sampled2 = rafgl_bilinear_sample(&doge, xn, yn);

            resulting = sampled;
            resulting2 = sampled2;

            pixel_at_m(raster, x, y) = resulting;
            pixel_at_m(raster2, x, y) = resulting2;


            if(pressed && rafgl_distance1D(location, y) < 3 && x > raster_width - 15)
            {
                pixel_at_m(raster, x, y).rgba = rafgl_RGB(255, 0, 0);
            }

        }
    }

    /* update-uj teksturu*/
    if(!game_data->keys_down[RAFGL_KEY_SPACE])
        rafgl_texture_load_from_raster(&texture, &raster);
    else
        rafgl_texture_load_from_raster(&texture, &raster2);
}


void main_state_render(GLFWwindow *window, void *args)
{
    /* prikazi teksturu */
    rafgl_texture_show(&texture);
}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_raster_cleanup(&raster2);
    rafgl_texture_cleanup(&texture);

}
