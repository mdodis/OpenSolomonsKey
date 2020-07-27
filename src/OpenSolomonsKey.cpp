/*
TODO:
- Goblin can live from a fall if it's falling and a block appears near it; SEE: https:youtu.be/jNi6DQEX3xQ?t=12
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
#include <algorithm>
#include <GL/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "osk_math.h"
//#include "gl_funcs.h"
#include "gl_graphics.h"
#include "gl_graphics.cpp"
#include "resources.cpp"
#include "text.cpp"
#include "map/entity.h"
#include "map/map.h"
#include "pickups.h"
#include "objects.h"
#include "score.cpp"
#include "audio.cpp"
#include "levels.cpp"
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
        gl_slow_tilemap_draw(texture, fvec2{i * 128, 24}, fvec2{64, 64}, 0.f, 2 * 4 + 0, true, true, NRGBA{1,1,1,1}, false);
        gl_slow_tilemap_draw(texture, fvec2{i * 128 + 64, 24}, fvec2{64, 64}, 0.f, 2 * 4 + 1, true, true, NRGBA{1,1,1,1}, false);
        // bottom bars
        gl_slow_tilemap_draw(texture, fvec2{i * 128, 12 * 64 + 32}, fvec2{64, 64}, 0.f, 2 * 4 + 2, false, false, NRGBA{1,1,1,1}, false);
        gl_slow_tilemap_draw(texture, fvec2{i * 128 + 64, 12 * 64 + 32}, fvec2{64, 64}, 0.f, 2 * 4 + 3, false, false, NRGBA{1,1,1,1}, false);
        
    }
    
    
    // draw the extra stuff:
    for (i32 i = 0; i < 6; ++i) {
        // Draw the vertical bars
        gl_slow_tilemap_draw(texture, fvec2{-32.f, i * 128.f + 64.f}, fvec2{64.f, 64.f}, 0.f, 1 * 4 + 2, false, false, NRGBA{1,1,1,1}, false);
        gl_slow_tilemap_draw(texture, fvec2{-32.f, i * 128.f + 64.f + 64.f}, fvec2{64.f, 64.f}, 0.f, 1 * 4 + 3, false, false, NRGBA{1,1,1,1}, false);
        gl_slow_tilemap_draw(texture, fvec2{15.f * 64.f + 32.f, i * 128.f + 64.f}, fvec2{64,64}, 0.f, 1 * 4 + 0, false, false, NRGBA{1,1,1,1}, false);
        gl_slow_tilemap_draw(texture, fvec2{15.f * 64.f + 32.f, i * 128.f + 64.f + 64.f}, fvec2{64, 64}, 0.f, 1 * 4 + 1, false, false, NRGBA{1,1,1,1}, false);
        
    }
    
}


int extract_file_level_number(char *str) {
    // level_<num>
    char *c;
    
    c = str + 6;
    int result = 0;
    while (isdigit(*c)) {
        result *= 10;
        result += (*c) - '0';
        c++;
    }
    
    return result;
}

bool level_string_cmp(char *s1, char *s2) {
    int n1 = extract_file_level_number(s1);
    int n2 = extract_file_level_number(s2);
    return n1 < n2;
}

std::vector<char *> g_map_list;

void cb_init() {
    srand(time(0));
    gl_init();
    load_tilemap_textures();
    load_sound_resources();
    gl_load_background_texture(17);
    g_map_list = list_maps();
    std::sort(g_map_list.begin(), g_map_list.end(), level_string_cmp);
    return;
}

void
cb_resize() {
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
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(g_projection));
    
    g_pixel_scale = (float)g_tile_scale / 64.0f;
}

internal void draw_ribbon(float dt) {
    const int num_ribbons = g_scene.player_num_slots;
    const int num_fireballs = g_scene.player_num_fireballs;
    int i;
    NRGBA color;
    static float t = 0.f;
    t += dt * 4;
    
    color.r = (sinf(t) + 2.f) * 0.5f;
    color.g = (sinf(t) + 2.f) * 0.5f;
    color.b = (cosf(t) + 2.f) * 0.5f;
    color.a = 1.f;
    
    fvec2 ribbon_start_pos = fvec2{64 * 9 + 47, 64 * 12};
    fvec2 ribbon_fire_start_pos = fvec2{64 * 10 + 4, 32 * 23 + 25};
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_ribbon), {64 * 10, 64 * 12}, {64,64}, 0, 2);
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_ribbon), ribbon_start_pos, {64,64}, 0, 1);
    
    for (i = 0; i < num_ribbons; i += 1) {
        // actual ribbon slot width is 46x46
        fvec2 pos = ribbon_start_pos - fvec2{float(i) * 45, 0};
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_ribbon), pos, {64,64}, 0, 1);
    }
    
    fvec2 pos = ribbon_start_pos - fvec2{float(i) * 45, 0};
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_ribbon), pos, {64,64}, 0, 0);
    
    for (i = 0; i < num_fireballs; i += 1){
        fvec2 pos = ribbon_fire_start_pos - fvec2{46 * float(i), 0};
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_ribbon_fire), pos, {32,32}, 0, 4, false, false, color);
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_ribbon_fire), pos + fvec2{0,32}, {32,32}, 0, 5, false, false, color);
        
    }
    
    
}

internal void draw_ui(float dt) {
    
    // 1p
    persist float text_1p_t = 0.f;
    text_1p_t += dt;
    
    draw_text("1p", 0, 0, false, 32,NRGBA{
                  sinf(text_1p_t) + 1,
                  cosf(text_1p_t) + 1,
                  sinf(text_1p_t) * 0.5f + cosf(text_1p_t) * 0.5f + 1,1});
    
    draw_text("Bonus", 0, 12, false, 32, NRGBA{1,1,0.5,1});
    draw_num(long(g_scene.player_time * 100), 1, 13 , false, 40, true);
    
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_dana), {0, 13 * 64 + 8}, {1024, 64}, 0, 2,
                         false, false, NRGBA{0,0,0,1}, false);
    for (int i = 0; i < g_scene.player_lives; ++i) {
        
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_dana), {i * 32.f, 13 * 64}, {64, 64}, 0, 5,
                             false, false, NRGBA{1,1,1,1}, false);
        
    }
    
    draw_ribbon(dt);
    
    draw_text("ROUND..", 27, 24, false, 32.f);
    
    draw_score(dt);
    
#ifndef NDEBUG
    draw_text("MS per frame", 0);
    draw_num(dt * 1000.f, 1);
#endif
}

void scene_menu(float dt);

void cb_render(InputState istate, u64 audio_sample_count, float dt) {
    
    if (g_scene.current_state == SS_MENU) {
        scene_menu(dt);
    } else {
        glClearColor( 0.0, 0.0,  0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        draw_extra_stuff();
        
        gl_background_draw();
        if (dt > 0.13f) dt = 0.13f;
        
        switch(g_scene.current_state) {
            
            case SS_STARTUP: {
                scene_startup_animation(dt);
            }break;
            case SS_PLAYING: {
                scene_update(&istate, dt);
            }break;
            case SS_WIN: {
                scene_win_animation(dt);
            }break;
            case SS_LOSE: {
                //g_scene.current_state = SS_PLAYING;
                //reload_map();
                //g_scene.player_is_dead = false;
                scene_lose_animation(dt);
            }break;
            
            default: {
                assert(false);
            }break;
        }
        
        draw_ui(dt);
    }
}

#include "menu.cpp"
