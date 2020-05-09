/*
TODO:
- level timer sound change when close to stop
- Background selection from .osk format
- level 2

- TODO: Blueflame should probably go to monster group!

- Goblin can live from a fall if it's falling and a block appears near it; SEE: https:youtu.be/jNi6DQEX3xQ?t=12
- Goblin can only fall _IF_ its currently in the walking, chasing, or waiting state. If it were in a punch state, it would have to finish that first, and then proceed to die by gravity
- Pickup secrets (add+destroy block in empty space)
- Sound resource system
- Make effects into single tilemap!
- WIN32: don't jump when maximizing through shortcut
- Better pixel shader: see casey's video (handmade char on octopath traveller)
  NOTE:
  use sox to convert audio into desired format:
sox [input] -r 48k -c 2 -b 16 [output]
-r 48k  :: 48000 sample rate
-c 2    :: stereo (2 channels)
-b 16   :: 16b / sample (can't explicitly say if it's signed or unsigned though...)
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <GL/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "osk_math.h"
#include "gl_funcs.h"
#include "gl_graphics.h"
#include "gl_graphics.cpp"
#include "resources.cpp"
#include "text.cpp"
#include "objects.h"
#include "audio.cpp"
#include "levels.cpp"
#include "score.cpp"
#include "sprites.cpp"

/* Calculate aspect ratio from current window dimensions.
 Returns the size of a tile in pixels. For example, a window
 of dimensions (1024x896) will get a tilescale of 64 pixels
*/
internal double
get_tilescale_and_dimensions(u32 current_w, u32 current_h, u32* out_w, u32* out_h) {
    i32 vw, vh;
    int leftover;
    if (((double)current_w / (double)current_h) == W_2_H) {
        vw = current_w;
        vh = current_h;
        glViewport(0,0,current_w, current_h);
        
    } else {
        
        if (current_w > current_h) {
            vh = current_h;
            vw = (i32)((double)(vh) * W_2_H);
            vw = ftrunc(vw);
            
            leftover = (i32)current_w - (i32)vw;
            if (leftover < 0) {
                // if the new one is bigger than what we show
                vh += leftover;
                vw = (i32)((double)(vh) * W_2_H);
                vw = ftrunc(vw);
            }
            leftover = (i32)current_w - (i32)vw;
            assert(leftover >= 0);
            
            glViewport(leftover / 2, 0, vw, vh);
        } else {
            vw = current_w;
            vh = (i32)((double)(vw) * HEIGHT_2_WIDTH_SCALE);
            vh = ftrunc(vh);
            
            leftover = (i32)current_h - (i32)vh;
            assert(leftover >= 0);
            
            glViewport(0, leftover / 2, vw, vh);
        }
        
    }
    
    *out_w = vw;
    *out_h = vh;
    
    return (int)vw * (0.0625);
}


internal void draw_extra_stuff() {
    GLTilemapTexture* texture = &GET_TILEMAP_TEXTURE(misc);
    
    // Draw the horizontal bars
    for (i32 idx = 0; idx < 9; ++idx) {
        float i = float(idx);
        // top bars
        gl_slow_tilemap_draw(texture,
                             fvec2{i * 128, 24},
                             fvec2{64, 64},
                             0.f, 2 * 4 + 0,
                             true, true,
                             NRGBA{1,1,1,1},
                             false);
        gl_slow_tilemap_draw(texture,
                             fvec2{i * 128 + 64, 24},
                             fvec2{64, 64},
                             0.f, 2 * 4 + 1,
                             true, true,
                             NRGBA{1,1,1,1},
                             false);
        // bottom bars
        gl_slow_tilemap_draw(texture,
                             fvec2{i * 128, 12 * 64 + 32},
                             fvec2{64, 64},
                             0.f, 2 * 4 + 2,
                             false, false,
                             NRGBA{1,1,1,1},
                             false);
        gl_slow_tilemap_draw(texture,
                             fvec2{i * 128 + 64, 12 * 64 + 32},
                             fvec2{64, 64},
                             0.f, 2 * 4 + 3,
                             false, false,
                             NRGBA{1,1,1,1},
                             false);
        
    }
    
    
    // draw the extra stuff:
    for (i32 i = 0; i < 6; ++i) {
        // Draw the vertical bars
        gl_slow_tilemap_draw(texture,
                             fvec2{-32.f, i * 128.f + 64.f},
                             fvec2{64.f, 64.f},
                             0.f, 1 * 4 + 2,
                             false, false,
                             NRGBA{1,1,1,1},
                             false);
        gl_slow_tilemap_draw(texture,
                             fvec2{-32.f, i * 128.f + 64.f + 64.f},
                             fvec2{64.f, 64.f},
                             0.f, 1 * 4 + 3,
                             false, false,
                             NRGBA{1,1,1,1},
                             false);
        gl_slow_tilemap_draw(texture,
                             fvec2{15.f * 64.f + 32.f, i * 128.f + 64.f},
                             fvec2{64, 64},
                             0.f, 1 * 4 + 0,
                             false, false,
                             NRGBA{1,1,1,1},
                             false);
        gl_slow_tilemap_draw(texture,
                             fvec2{15.f * 64.f + 32.f, i * 128.f + 64.f + 64.f},
                             fvec2{64, 64},
                             0.f, 1 * 4 + 1,
                             false, false,
                             NRGBA{1,1,1,1},
                             false);
        
    }
    
}

void cb_init() {
    srand(time(0));
    srand(time(0));
    gl_init();
    load_tilemap_textures();
    load_sound_resources();
    
    load_map(&g_scene.loaded_map, "test1.txt");
    
    audio_play_sound(GET_SOUND(SND_background), true, SoundType::Music, false);
    return;
}

void
cb_resize()
{
    g_tile_scale = get_tilescale_and_dimensions(g_wind_width, g_wind_height, &g_view_width, &g_view_height);
    
#ifdef OSK_ROUND_TO_POW_2
    g_tile_scale = highest_pow2((u64)g_tile_scale);
    
    g_view_width  = g_tile_scale * 16;
    g_view_height = (i32)((double)(g_view_width) * HEIGHT_2_WIDTH_SCALE);
    
    int lx = (i32)g_wind_width - g_view_width;
    int ly = (i32)g_wind_height - g_view_height;
    glViewport(lx / 2, ly / 2, g_view_width, g_view_height);
#endif
    
    g_projection = glm::ortho(0.0f, (float)g_view_width, (float)g_view_height, 0.0f);
    
    g_shd_2d.apply();
    GLuint loc = glGetUniformLocation(g_shd_2d.id, "projection");
    glUniformMatrix4fv(
                       loc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(g_projection));
    
    
    g_pixel_scale = (float)g_tile_scale / 64.0f;
}

void draw_ui(float dt) {
    
    // 1p
    persist float text_1p_t = 0.f;
    text_1p_t += dt;
    
    draw_text("1p", 0, 0, false, 32,NRGBA{
                  sinf(text_1p_t) + 1,
                  cosf(text_1p_t) + 1,
                  sinf(text_1p_t) * 0.5f + cosf(text_1p_t) * 0.5f + 1,1});
    
    draw_text("Bonus", 0, 12, false, 32, NRGBA{1,1,0.5,1});
    draw_num(long(g_scene.player_time * 100), 1, 13 , false, 40, true);
    
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(dana), {0, 13 * 64 + 8}, {1024, 64}, 0, 2,
                         false, false, NRGBA{0,0,0,1}, false);
    for (int i = 0; i < g_scene.player_lives; ++i) {
        
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(dana), {i * 32.f, 13 * 64}, {64, 64}, 0, 5,
                             false, false, NRGBA{1,1,1,1}, false);
        
    }
    
    draw_text("ROUND..", 27, 24, false, 32.f);
    
    draw_score(dt);
    
    draw_text("MS per frame", 0);
    draw_num(dt * 1000.f, 1);
}

void cb_render(InputState istate, u64 audio_sample_count, float dt)
{
    glClearColor( 0.0, 0.0,  0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    draw_extra_stuff();
    
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(background),
                         {0,0}, {15 * 64, 768},
                         0);
    
    if (dt > 0.13f) dt = 0.13f;
    
    if (g_scene.playing)
        scene_update(&istate, dt);
    else
        scene_startup_animation(dt);
    
    draw_ui(dt);
    
}
