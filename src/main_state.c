#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include "../rafgl.h"

static rafgl_raster_t doge;
static rafgl_raster_t raster, raster2;
static rafgl_texture_t texture;

static int raster_width = 0, raster_height = 0;


void main_state_init(GLFWwindow *window, void *args)
{
    /* inicijalizacija */
    /* raster init nam nije potreban ako radimo load from image */
    rafgl_raster_load_from_image(&doge, "res/images/doge.png");
    raster_width = doge.width;
    raster_height = doge.height;

    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);

    rafgl_texture_init(&texture);
}


int pressed, location = 0;
float selector = 0;

int histogram[256];

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

    int x, y;

    float xn, yn;

    rafgl_pixel_rgb_t sampled, resulting, resulting2;

    for(x = 0; x < 256; x++)
    {
        histogram[x] = 0;
    }

     for(y = 0; y < raster_height; y++)
    {
        for(x = 0; x < raster_width; x++)
        {
            sampled = pixel_at_m(doge, x, y);
            histogram[rafgl_calculate_pixel_brightness(sampled)] += 1;
        }
    }

    for(y = 0; y < raster_height; y++)
    {
        for(x = 0; x < raster_width; x++)
        {

            resulting.r = 0;
            resulting.g = 0;
            resulting.b = 0;

            resulting2.r = 255;
            resulting2.g = 255;
            resulting2.b = 255;

            pixel_at_m(raster, x, y) = resulting;
            pixel_at_m(raster2, x, y) = pixel_at_m(doge, x, y);


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
