#ifndef OSK_HH
#define OSK_HH
#include <stdint.h>

//#define OSK_ROUND_TO_POW_2

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

typedef size_t   b32;

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

enum PaletteEntryType
{
    PENTRY_EMPTY_SPACE,        /*params: -*/
    PENTRY_BLOCK_BREAKABLE,    /*params: tilemapidx, tileidx*/
    PENTRY_BLOCK_UNBREAKABLE,  /*params: tilemapidx, tileidx*/
    PENTRY_UNKNOWN
};

struct PaletteEntry
{
    PaletteEntryType type;
    i32 params[2];
};

#endif //! OSK_HH