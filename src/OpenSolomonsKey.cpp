/*
TODO:
- eBlockFrail needs to have a default health parameter
- eGoblin
- continue level loading
- Sound resource system (one of us...)
- background music play/stop
=========================================================
  X Block creation - destruction
   X Play on event (keyboard press)
 X scene graph to query enemies for collisions
   X player animation logic
   
  NOTE:
  use sox to convert audio into desired format:
sox [input] -r 48k -c 2 -b 16 [output]
-r 48k  :: 48000 sample rate
-c 2    :: stereo (2 channels)
-b 16   :: 16b / sample (can't explicitly say if it's signed or unsigned though...)
*/

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define OSK_MATH_IMPL
#include "osk_math.h"
#include "gl_funcs.h"
#include "objects.h"
#include "gl_graphics.h"
#include "resources.cpp"
#include "audio.cpp"
#include "sprites.cpp"
#include "levels.cpp"

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


RESSound bg_sound;

void
cb_init()
{
    player_jump_sound = Wave_load_from_file("bloop.wav");
    bg_sound = Wave_load_from_file("bgm1.wav");
    
    
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
    
    
    scene_init("Hello ");
    
    Sprite player_sprite = {
        .tilemap = &GET_CHAR_TILEMAP(test_player),
        .collision_box = {5,0,45,64},
        .current_frame = 0,
        .current_animation = GET_CHAR_ANIMENUM(test_player, Run),
        .animation_set = GET_CHAR_ANIMSET(test_player),
        .entity =
        {
            ePlayer,
            {0,0}
        }
    };
    scene_sprite_add(&player_sprite);
    
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
    
    glClearColor( 0.156, 0.156,  0.156, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (GET_KEYPRESS(m_pressed))
    {
        audio_play_sound(&player_jump_sound);
    }
    
    persist b32 initial = true;
    if (initial)
    {
        initial = false;
        audio_play_sound(&bg_sound, true);
    }
    
    scene_update(&istate, dt);
    audio_update(&istate, audio_sample_count);
    
}

#include "gl_graphics.cpp"