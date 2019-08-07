#include <stdio.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define OSK_MATH_IMPL
#include "osk_math.h"

#include "gl_funcs.h"
#include "gl_graphics.h"

#include "drawing.cpp"

#define IS_DIGIT(x) (x >= '0' && x <= '9')
#define ENUM_TO_STR(x) #x

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


internal u8*
load_image_as_rgba_pixels(
const char* const name,
i32* out_width,
i32* out_height,
i32* out_n)
{
    int i_w, i_h, i_n;
    unsigned char* data = stbi_load(name, out_width, out_height, out_n, 4);
    
    return data;
}


#include "resources.cpp"

void
cb_init()
{
    
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

internal b32
string_cmp_indentifier(
const char* c,
const char* str,
u64* out_size)
{
    u64 size = 0;
    
    while (*c != ' ' && *c != '\n')
    {
        if (*c != *str)
        {
            if (*str == 0) break;
            return false;
        }
        
        size++;
        c++; str++;
    }
    *out_size = size;
    return true;
}

internal const char*
string_trim(const char* c)
{
    while(
        *c == ' ' ||
        *c == '\n')
        c++;
    return c;
}

internal const char*
string_parse_uint(const char* c, u64* out_i)
{
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
}

global PaletteEntry g_palette[64];
global u64 g_tilemap[15][12];
global u64 g_palette_size;

internal void load_test_level(
PaletteEntry palette[64],
u64* out_size)
{ 
    char* level;
    u64 size;
    FILE* f = fopen("test_level.oskmap", "rb");
    if (!f) exit(-1);
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    level = (char*)malloc(size + 1);
    assert(level);
    fread(level, size, 1, f);
    level[size] = 0;
    fclose(f);
    
    const char* c = level;
    
    u64 palette_size, counter = 0;
    
    while(*c != 0)
    {
        switch(*c)
        {
            case '#':
            {
                c++;
                u64 stride = 0;
                if (string_cmp_indentifier(c, "PALETTE", &stride))
                {
                    puts("PALETTE");
                    c += stride;
                    
                    c = string_trim(c);
                    c = string_parse_uint(c, &palette_size);
                    
                    *out_size = palette_size;
                    
                    printf("%ld\n", (int)palette_size);
                    
                }
                else if (string_cmp_indentifier(c, ENUM_TO_STR(PENTRY_EMPTY_SPACE), &stride))
                {
                    puts("PENTRY_EMPTY_SPACE");
                    c += stride;
                    if (counter != 0)
                    {
                        puts("ERR:PENTRY_EMPTY_SPACE must exist as first element");
                        exit(-1);
                    }
                    
                    palette[counter].type = PENTRY_EMPTY_SPACE;
                    
                    counter++;
                }
                else if (string_cmp_indentifier(c, ENUM_TO_STR(PENTRY_BLOCK_BREAKABLE), &stride))
                {
                    puts("PENTRY_BLOCK_BREAKABLE");
                    c += stride;
                    
                    palette[counter].type = PENTRY_BLOCK_BREAKABLE;
                    
                    counter++;
                }
                else if (string_cmp_indentifier(c, ENUM_TO_STR(PENTRY_BLOCK_UNBREAKABLE), &stride))
                {
                    puts("PENTRY_BLOCK_UNBREAKABLE");
                    c += stride;
                    
                    palette[counter].type = PENTRY_BLOCK_UNBREAKABLE;
                    
                    counter++;
                }
                else if (string_cmp_indentifier(c, "MAP", &stride))
                {
                    puts("MAP");
                    c += stride;
                    u64 layer_num;
                    
                    c = string_trim(c);
                    c = string_parse_uint(c, &layer_num);
                    printf("%d\n",(int) layer_num);
                    c = string_trim(c);
                    
                    for(u32 j = 0; j < 12; ++j)
                    {
                        for(u32 i = 0; i < 15; ++i)
                        {
                            u64 idx;
                            c = string_trim(c);
                            c = string_parse_uint(c, &idx);
                            
                            g_tilemap[i][j] = idx;
                            
                            printf("%d ", (int)idx);
                        }
                        puts("");
                    }
                    
                }
                else
                {
                    puts("UNKNOWN directive!");
                    exit(6);
                }
                
                if (counter > palette_size)
                {
                    puts("ERR: palette_size smaller than the counter");
                    exit(5);
                }
                
            } break;
            
            default:
            {
                c++;
            } break;
        }
        
    }
}

u32 current_frame = 0;
Timer test_anim_timer;

#define GRAVITY 900
#define MAX_YSPEED 450
#define JUMP_STRENGTH 350

global AnimatedSprite player = {
    .collision_box = {5,0,45,64}
};

global AnimatedSprite enemy = {
    .position = {100, 100},
    .collision_box = {5,5,40,40},
    .velocity = {0, 0}
};

global u32 current_animation = GET_CHAR_ANIM_HANDLE(test_player, Idle);
global b32 is_on_air = true;

void
cb_render(InputState istate, float dt)
{
    player.tilemap = &GET_CHAR_TILEMAP(test_player);
    enemy.tilemap = &GET_TILEMAP_TEXTURE(test);
    
    glClearColor( 0.156, 0.156,  0.156, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ivec2 before = player.position;
    
    if (istate.move_right)
    {
        player.velocity.x = 250.f;
    }
    else if (istate.move_left)
    {
        player.velocity.x = -250.f;
    }
    else
        player.velocity.x = 0;
    
    if (istate.move_up && !is_on_air)
    {
        player.velocity.y = -JUMP_STRENGTH;
        is_on_air = true;
    }
    
    
    player.velocity.y += GRAVITY * dt;
    
    player.velocity.y = iclamp(-JUMP_STRENGTH, MAX_YSPEED, player.velocity.y);
    player.position.x += player.velocity.x * dt;
    player.position.y += player.velocity.y * dt;
    
    persist b32 loaded = false;
    if (!loaded)
    {
        load_test_level(g_palette, &g_palette_size);
        loaded = true;
        player.velocity.y = 0;
        //current_animation = GET_CHAR_ANIM_HANDLE(test_player, Idle2);
        current_frame = 0;
        test_anim_timer.reset();
    }
    
    
    ivec2 ipos = {(i32)player.position.x + 32, (i32)player.position.y + 32};
    ivec2 player_tile = map_position_to_tile(ipos);
    
    
    for(int i = 0; i < 15; ++i )
    {
        for(int j = 0; j < 12; ++j )
        {
            u32 id;
            PaletteEntryType type = g_palette[g_tilemap[i][j]].type;
            
            if (type == PENTRY_EMPTY_SPACE) continue;
            if (type == PENTRY_BLOCK_UNBREAKABLE) id = 2;
            if (type == PENTRY_BLOCK_BREAKABLE) id = 1;
            
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
            
            if (g_tilemap[start_tile.x + i][start_tile.y + j] == PENTRY_EMPTY_SPACE) continue;
            
            ivec2 tile_coords =
            {
                (start_tile.x + i) * 64,
                (start_tile.y + j) * 64
            };
            
            AABox collision = {0,0,64,64};
            collision = collision.translate(tile_coords);
            AABox player_trans = player.get_transformed_AABox();
            
            ivec2 diff;
            b32 collided = aabb_intersect(&player_trans, &collision, &diff);
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
                    puts("GRND");
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
    
    float elapsed = test_anim_timer.get_elapsed_secs();
    Animation* anim_ref = &GET_ANIM_BY_HANDLE(test_player, current_animation); 
    
    if (elapsed > anim_ref->duration)
    {
        if (current_frame < anim_ref->size)
            current_frame++;
        else if (current_frame >= anim_ref->size && anim_ref->loop)
            current_frame = 0;
        
        test_anim_timer.reset();
    }
    
    AABox box = player.get_transformed_AABox();
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {box.min_x, box.min_y},
        {box.max_x - box.min_x, box.max_y - box.min_y},
        0,5 * 5 + 1 );
    
    box = enemy.get_transformed_AABox();
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
    
    i32 frame_to_render = anim_ref->start.y * player.tilemap->cols
        + anim_ref->start.x + current_frame;
    AnimatedSprite_draw(&player, frame_to_render);
}

#include "gl_graphics.cpp"