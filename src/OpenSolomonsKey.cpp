/*
TODO:
  - Logging style text system for easier debugging (rather than switching to
    a console every time...)
    
- Background drawing and selection from .osk format

- eBlockFrail needs to have a default health parameter
- Sound resource system (one of us...)

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
#include "resources.cpp"
#include "text.cpp"
#include "objects.h"
#include "audio.cpp"
#include "levels.cpp"
#include "sprites.cpp"


global u32 g_quad_vao;
global GLShader g_shd_2d;
global glm::mat4 g_projection;

/* Calculate aspect ratio from current window dimensions.
 Returns the size of a tile in pixels. For example, a window
 of dimensions (1024x896) will get a tilescale of 64 pixels
*/
internal double
get_tilescale_and_dimensions(
u32 current_w, u32 current_h,
u32* out_w, u32* out_h)
{
    i32 vw, vh;
    int leftover;
    if (((double)current_w / (double)current_h) == W_2_H)
    {
        vw = current_w;
        vh = current_h;
        glViewport(0,0,current_w, current_h);
        
    } else {
        
        if (current_w > current_h)
        {
            vh = current_h;
            vw = (i32)((double)(vh) * W_2_H);
            vw = ftrunc(vw);
            
            leftover = (i32)current_w - (i32)vw;
            if (leftover < 0)
            {
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


internal void draw_extra_stuff()
{
    GLTilemapTexture* texture = &GET_TILEMAP_TEXTURE(misc);
    
    // Draw the horizontal bars
    for (i32 i = 0; i < 9; ++i)
    {
        // top bars
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{i * 128, 24},
            glm::vec2{64, 64},
            0.f, 2 * 4 + 0,
            true, true,
            NRGBA{1,1,1,1},
            false);
        
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{i * 128 + 64, 24},
            glm::vec2{64, 64},
            0.f, 2 * 4 + 1,
            true, true,
            NRGBA{1,1,1,1},
            false);
        
        // bottom bars
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{i * 128, 12 * 64 + 32},
            glm::vec2{64, 64},
            0.f, 2 * 4 + 2,
            false, false,
            NRGBA{1,1,1,1},
            false);
        
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{i * 128 + 64, 12 * 64 + 32},
            glm::vec2{64, 64},
            0.f, 2 * 4 + 3,
            false, false,
            NRGBA{1,1,1,1},
            false);
        
    }
    
    
    // draw the extra stuff:
    for (i32 i = 0; i < 6; ++i)
    {
        // Draw the vertical bars
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{-32, i * 128 + 64},
            glm::vec2{64, 64},
            0.f, 1 * 4 + 2,
            false, false,
            NRGBA{1,1,1,1},
            false);
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{-32, i * 128 + 64 + 64},
            glm::vec2{64, 64},
            0.f, 1 * 4 + 3,
            false, false,
            NRGBA{1,1,1,1},
            false);
        
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{15 * 64 + 32, i * 128 + 64},
            glm::vec2{64, 64},
            0.f, 1 * 4 + 0,
            false, false,
            NRGBA{1,1,1,1},
            false);
        gl_slow_tilemap_draw(
            texture,
            glm::vec2{15 * 64 + 32, i * 128 + 64 + 64},
            glm::vec2{64, 64},
            0.f, 1 * 4 + 1,
            false, false,
            NRGBA{1,1,1,1},
            false);
        
    }
    
}

RESSound bg_sound;

void
cb_init()
{
    player_jump_sound = Wave_load_from_file("res/bloop.wav");
    bg_sound = Wave_load_from_file("res/bgm1.wav");
    
    srand(time(0));
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    g_shd_2d.create(g_2d_vs, g_2d_fs);
    
    //glActiveTexture(GL_TEXTURE0);
    load_tilemap_textures();
    
    GLuint sampler_loc = glGetUniformLocation(g_shd_2d.id, "sampler");
    glUniform1i(sampler_loc, 0);
    
    GLuint VBO;
    GLfloat vertices[] = {
        // Pos      // Tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &g_quad_vao);
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(g_quad_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    assert(glGetError() == GL_NO_ERROR);
    
    
    scene_init("lvl1.osk");
    
    return;
}

void
cb_resize()
{
    g_tile_scale = get_tilescale_and_dimensions(
        g_wind_width, g_wind_height,
        &g_view_width, &g_view_height);
    
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

void
cb_render(InputState istate, u64 audio_sample_count, float dt)
{
    glClearColor( 0.0, 0.0,  0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    draw_extra_stuff();
    
    // TODO(miked): move this into resource "manager"
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(background),
        {0,0}, {15 * 64, 768},
        0);
    
    draw_text("MS per frame", 0);
    draw_num(dt * 1000.f, 1);
    
    persist b32 initial = true;
    if (initial)
    {
        initial = false;
        audio_play_sound(&bg_sound, true, Music);
    }
    
    // audio control
    if (GET_KEYPRESS(space_pressed))    audio_toggle_playing(Music);
    else if (GET_KEYPRESS(sound_up))    g_audio.volume = fclamp(0.f, 1.f, g_audio.volume + 0.1f);
    else if (GET_KEYPRESS(sound_down))  g_audio.volume = fclamp(0.f, 1.f, g_audio.volume - 0.1f);
    
    if (dt > 0.13f) dt = 0.13f;
    
    scene_update(&istate, dt);
    audio_update(&istate, audio_sample_count);
    
    // NOTE(miked): testing an effect
    if (GET_KEYPRESS(m_pressed))
    {
        Sprite effect = make_effect({64 * 5,64 * 3});
        SET_ANIMATION(((Sprite*)&effect), Effect, Test);
        
        scene_sprite_add(&effect);
        
    }
    
    // 1p
    persist float text_1p_t = 0.f;
    text_1p_t += dt;
    
    draw_text("1p", 0, 0, false, 32,NRGBA{
              sinf(text_1p_t) + 1,
              cosf(text_1p_t) + 1,
              sinf(text_1p_t) * 0.5f + cosf(text_1p_t) * 0.5f + 1,1});
    
    draw_text("Bonus", 0, 12, false, 32, NRGBA{1,1,0.5,1});
    draw_num(1000, 1, 15 , false, 32, true);
}

#include "gl_graphics.cpp"