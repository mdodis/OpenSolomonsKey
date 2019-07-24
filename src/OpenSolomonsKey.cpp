#include <stdio.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl_funcs.h"
#include "gl_graphics.h"

#define IS_DIGIT(x) (x >= '0' && x <= '9')
#define ENUM_TO_STR(x) #x


u32 g_quad_vao;

global GLShader g_shd_2d;
GLTilemapTexture g_tilemap_texture;

//Tilemap        g_tilemap;

glm::mat4 g_projection;

inline i32 ftrunc(float n) { return (i32)(n); }

inline i32 iclamp(i32 a, i32 b, i32 x)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline ivec2 iclamp(ivec2 a, ivec2 b, ivec2 x)
{
    return ivec2
    {
        iclamp(a.x, b.x, x.x),
        iclamp(a.y, b.y, x.y)
    };
}

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

void
cb_init()
{
    
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    g_shd_2d.create(g_2d_vs, g_2d_fs);
    
    
    int w, h, n;
    u8* data = load_image_as_rgba_pixels("Phoebus.png", &w, &h, &n);
    assert(data);
    
    glActiveTexture(GL_TEXTURE0);
    g_tilemap_texture = gl_load_rgba_tilemap(data, w, h, 16, 16);
    
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
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("Glerr: %d\n", err);
        exit(-10);
    }
    
    assert(glGetError() == GL_NO_ERROR);
    return;
}

#include <math.h>
int highestPowerof2(int n)
{
    int p = (int)log2(n);
    return (int)pow(2, p);
}

void
cb_resize()
{
    g_tile_scale = get_tilescale_and_dimensions(
        g_wind_width, g_wind_height,
        &g_view_width, &g_view_height);
    
#ifdef OSK_ROUND_TO_POW_2
    g_tile_scale = highestPowerof2((u64)g_tile_scale);
    
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
#include <float.h>

internal b32
intersect(
const AABox* const a,
const AABox* const b,
ivec2* const opt_pen = 0)
{
    AABox result;
    result.min_y = a->min_y - b->max_y;
    result.max_y = a->max_y - b->min_y;
    
    result.min_x = a->min_x - b->max_x;
    result.max_x = a->max_x - b->min_x;
    
    
    if (result.min_x <= 0 &&
        result.max_x >= 0 &&
        result.min_y <= 0 &&
        result.max_y >= 0)
    {
        ivec2 penetration = {0,0};
        float min = FLT_MAX;
        
        if (glm::abs(result.min_x) < min)
        {
            min = glm::abs(result.min_x);
            penetration = {result.min_x, 0};
        }
        
        if (glm::abs(result.max_x) < min)
        {
            min = glm::abs(result.max_x);
            penetration = {result.max_x, 0};
        }
        
        if (glm::abs(result.min_y) < min)
        {
            min = glm::abs(result.min_y);
            penetration = {0, result.min_y};
        }
        
        if (glm::abs(result.max_y) < min)
        {
            min = glm::abs(result.max_y);
            penetration = {0, result.max_y};
        }
        
        if (opt_pen) *opt_pen = penetration;
        
        return true;
    }
    
    if (opt_pen) *opt_pen = {0,0};
    
    return false;
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

inline ivec2 
map_position_to_tile(ivec2 position)
{
    return ivec2{position.x / 64, position.y / 64};
}

glm::vec2 position(0,0);
glm::vec2 size(64,64);
float rotate = -90.f;

void
cb_render(InputState istate, float dt)
{
    AABox player_box = {
        5,0,
        45,64};
    
    
    glClearColor( 0.156, 0.156,  0.156, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (istate.move_right)
    {
        position.x += 200.f * dt;
    }
    if (istate.move_left)
    {
        position.x -= 200.f * dt;
    }
    if (istate.move_up)
    {
        position.y -= 200.f * dt;
    }
    if (istate.move_down)
    {
        position.y += 200.f * dt;
    }
    
    persist b32 loaded = false;
    if (istate.spacebar_pressed && !loaded)
    {
        load_test_level(g_palette, &g_palette_size);
        loaded = true;
    }
    
    
    ivec2 ipos = {(i32)position.x + 32, (i32)position.y + 32};
    ivec2 player_tile = map_position_to_tile(ipos);
    
    for(int i = 0; i < 15; ++i )
    {
        for(int j = 0; j < 12; ++j )
        {
            u32 id;
            PaletteEntryType type = g_palette[g_tilemap[i][j]].type;
            
            if (type == PENTRY_EMPTY_SPACE) id = -1;
            
            if (type == PENTRY_BLOCK_UNBREAKABLE) id = 3;
            if (type == PENTRY_BLOCK_BREAKABLE) id = 4;
            
            
            gl_slow_tilemap_draw(
                &g_tilemap_texture,
                {i * 64, j * 64},
                {64, 64},
                0.f,
                id);
            
            if (i == player_tile.x && j == player_tile.y)
            {
                
                gl_slow_tilemap_draw(
                    &g_tilemap_texture,
                    {i * 64, j * 64},
                    {64, 64},
                    0.f,
                    5);
                
            }
        }
    }
    
    ivec2 start_tile = player_tile + ivec2{-1,-1};
    start_tile = iclamp({0,0}, {14,11}, start_tile);
    
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
            
            AABox collision, player_trans;
            collision = collision.translate(tile_coords);
            
            player_trans = player_box.translate(
            {ftrunc(position.x), ftrunc(position.y)});
            
            ivec2 diff;
            b32 collided = intersect(&player_trans, &collision, &diff);
            if (collided)
            {
                position -= glm::vec2{diff.x, diff.y};
                position.x = ftrunc(position.x);
                position.y = ftrunc(position.y);
            }
            
            gl_slow_tilemap_draw(
                &g_tilemap_texture,
                {tile_coords.x, tile_coords.y},
                {64, 64},
                0.f,
                5);
        }
    }
    
    
    player_box = player_box.translate({(i32)position.x, (i32)position.y});
    
    gl_slow_tilemap_draw(
        &g_tilemap_texture,
        position, size,rotate, 1, true);
    
    gl_slow_tilemap_draw(
        &g_tilemap_texture,
        {player_box.min_x, player_box.min_y},
        {player_box.max_x - player_box.min_x, player_box.max_y - player_box.min_y},
        0,0 + 1 * 16 );
    
}

#include "gl_graphics.cpp"