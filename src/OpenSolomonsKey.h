#ifndef OSK_HH
#define OSK_HH
#include <stdint.h>

#define OSK_ROUND_TO_POW_64

#define internal static
#define global   static
#define persist  static

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef i32      b32;

extern u32    g_wind_width;
extern u32    g_wind_height;

extern u32    g_view_width;
extern u32    g_view_height;

extern double g_tile_scale;  // base scale for everything
double        g_pixel_scale; // scale of a pixel(for non tile-aligned movement)

typedef struct
{
    b32 move_right;
    b32 move_left;
    b32 move_up;
    b32 move_down;
    b32 spacebar_pressed;
    b32 m_pressed;
} InputState;

global InputState g_input_state;


#define W_2_H 1.1428571428571428
#define HEIGHT_2_WIDTH_SCALE 0.875

#define HORIZ_SCREEN_PAD (g_tile_scale / 2.0)
#define VERTC_SCREEN_PAD (g_tile_scale)

#define IS_POW2(x) x && !(x & (x - 1))


#if   defined(OSK_PLATFORM_X11)

b32 x11_get_key_state(i32 key);
#define ISTATE_KEYDOWN_ACTION(k, a) g_input_state.a = x11_get_key_state(k)

#elif defined(OSK_PLATFORM_WIN32)

b32 win32_get_key_state(i32 key);
#define ISTATE_KEYDOWN_ACTION(k, a) g_input_state.a = win32_get_key_state(k)

#endif

extern void cb_init();
extern void cb_resize();
extern void cb_render(InputState istate, float dt);



const char* const g_2d_vs =
R"EOS(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}

)EOS";

const char* const g_2d_fs =
R"EOS(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2DArray sampler;
uniform int layer = 0;

void main()
{    
      color =  texture(sampler, vec3(TexCoords,layer));
      //color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    
)EOS";

typedef struct
{
    u32 texture_id;
    i32 width, height;
    i32 rows, cols;
}  TilemapTexture;

typedef struct
{
    const i32 rows = 15;
    const i32 cols = 12;
    TilemapTexture const* texture;
    
} Tilemap;

#endif //! OSK_HH