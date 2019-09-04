/*
TODO:
   - "scene graph" to query enemies for collisions
- player animation logic
- continue level loading
- background music play/stop
- continue the level format after doing the goblin enemy
=========================================================
 X Play on event (keyboard press)
 
 NOTE:
  use sox to convert audio into desired format:
sox [input] -r 48k -c 2 -b 16 [output]
-r 48k  :: 48000 sample rate
-c 2    :: stereo (2 channels)
-b 16   :: 16b / sample (can't explicitly say if it's signed or unsigned though...)
*/

#include <stdio.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <portaudio.h>

#define OSK_MATH_IMPL
#include "osk_math.h"
#include "gl_funcs.h"
#include "gl_graphics.h"
#include "sprites.cpp"
#include "levels.cpp"
#include "audio.cpp"

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

#include "resources.cpp"

RESSound test_sound;
RESSound bg_sound;

void
cb_init()
{
    audio_init();
    
    test_sound = Wave_load_from_file("bloop.wav");
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(g_quad_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    assert(glGetError() == GL_NO_ERROR);
    
    
    scene_init("Hello ");
    
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

#define GRAVITY 900
#define MAX_YSPEED 450
#define JUMP_STRENGTH 350

global Sprite player = {
    .collision_box = {5,0,45,64},
    .current_frame = 0,
    .current_animation = GET_CHAR_ANIMENUM(test_player, Run),
    .animation_set = GET_CHAR_ANIMSET(test_player),
};

global b32 is_on_air = true;

void
cb_render(InputState istate, float dt)
{
    
    player.tilemap = &GET_CHAR_TILEMAP(test_player);
    Sprite_update_animation(&player, dt);
    
    glClearColor( 0.156, 0.156,  0.156, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ivec2 before = player.position;
    
    if (GET_KEYDOWN(move_right))
    {
        player.velocity.x = 250.f;
    }
    else if (GET_KEYDOWN(move_left))
    {
        player.velocity.x = -250.f;
    }
    else
        player.velocity.x = 0;
    
    if (GET_KEYPRESS(move_up) && !is_on_air)
    {
        player.velocity.y = -JUMP_STRENGTH;
        audio_play_sound(&test_sound);
        is_on_air = true;
    }
    
    if (GET_KEYPRESS(m_pressed))
    {
        Sprite spr;
        spr.collision_box = {0,0,64,64};
        spr.current_frame = 0;
        spr.position = {10,10};
        spr.current_animation = GET_CHAR_ANIMENUM(test_enemy, Idle);
        spr.animation_set = GET_CHAR_ANIMSET(test_enemy);
        spr.tilemap = &GET_CHAR_TILEMAP(test_enemy);
        scene_sprite_add(&spr);
    }
    
    persist b32 initial = true;
    if (initial)
    {
        initial = false;
        audio_play_sound(&bg_sound);
    }
    
    player.velocity.y += GRAVITY * dt;
    
    player.velocity.y = iclamp(-JUMP_STRENGTH, MAX_YSPEED, player.velocity.y);
    player.position.x += player.velocity.x * dt;
    player.position.y += player.velocity.y * dt;
    
    
    ivec2 ipos = {(i32)player.position.x + 32, (i32)player.position.y + 32};
    ivec2 player_tile = map_position_to_tile(ipos);
    
    
    for(int i = 0; i < 15; ++i )
    {
        for(int j = 0; j < 12; ++j )
        {
            u32 id;
            EntityBaseType type = (EntityBaseType)g_scene.tilemap[i][j];
            
            if (type == eEmptySpace) continue;
            if (type == eBlockSolid) id = 2;
            if (type == eBlockFrail) id = 1;
            
            gl_slow_tilemap_draw(
                &GET_TILEMAP_TEXTURE(test),
                {i * 64, j * 64},
                {64, 64},
                0.f,
                id);
            
        }
    }
    
    //
    // Collision detection around 3x3 grid
    //
    ivec2 start_tile = player_tile - 1;
    start_tile = iclamp({0,0}, {14,11}, start_tile);
    b32 collided_on_bottom = false;
    for (i32 j = 0; j < 3; ++j)
    {
        for (i32 i = 0; i < 3; ++i)
        {
            if (i == 1 && j == 1) continue;
            
            if (g_scene.tilemap[start_tile.x + i][start_tile.y + j] == eEmptySpace) continue;
            
            ivec2 tile_coords =
            {
                (start_tile.x + i) * 64,
                (start_tile.y + j) * 64
            };
            
            AABox collision = {0,0,64,64};
            collision = collision.translate(tile_coords);
            AABox player_trans = player.get_transformed_AABox();
            
            ivec2 diff;
            b32 collided = aabb_minkowski(&player_trans, &collision, &diff);
            if (collided)
            {
                player.position = player.position - (diff);
                
                if (player.velocity.y < 0 &&
                    iabs(diff.y) < 5)
                {
                    player.position.y += diff.y;
                    continue;
                }
                
                if (j == 2) collided_on_bottom = true;
                
                if (j == 2 &&
                    iabs(diff.y) > 0)
                {
                    is_on_air = false;
                    player.velocity.y = 0;
                    //puts("GRND");
                }
                
                if (player.velocity.y < 0 && 
                    is_on_air &&
                    (player_trans.min_x < collision.max_x && 
                     diff.x >= 0))
                {
                    player.velocity.y = -player.velocity.y;
                    printf("%d %d %d %d %d\n",
                           player_trans.min_x,
                           collision.max_x,
                           collision.max_y,
                           player_trans.min_y, diff.y);
                    
                    is_on_air = true;
                }
                
                if (i == 0 && j == 2 &&
                    iabs(diff.x) > 0)
                {
                    player.position.x += diff.x;
                }
                
            }
            
            gl_slow_tilemap_draw(
                &GET_TILEMAP_TEXTURE(test),
                {tile_coords.x, tile_coords.y},
                {64, 64},
                0.f,
                5 * 5);
        }
    }
    
    if (!collided_on_bottom && player.velocity.y != 0)
    {
        is_on_air = true;
    }
    
    AABox box = player.get_transformed_AABox();
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {box.min_x, box.min_y},
        {box.max_x - box.min_x, box.max_y - box.min_y},
        0,5 * 5 + 1 );
    
    if (!is_on_air)
    {
        gl_slow_tilemap_draw(
            &GET_TILEMAP_TEXTURE(test),
            {0, 0},
            {30, 30},
            0,5 * 2 + 4 );
    }
    
    Sprite_draw_anim(&player);
    
    scene_update(&istate, dt);
    audio_update(&istate);
    
}

#include "gl_graphics.cpp"