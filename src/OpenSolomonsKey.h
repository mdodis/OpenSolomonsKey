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
typedef int8_t   i18;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef i32 b32;

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
    b32 spacebar_pressed;
} InputState;

global InputState g_input_state;


#define W_2_H 1.1428571428571428
#define HEIGHT_2_WIDTH_SCALE 0.875

#define HORIZ_SCREEN_PAD (g_tile_scale / 2.0)
#define VERTC_SCREEN_PAD (g_tile_scale)

#define IS_POW2(x) x && !(x & (x - 1))


#ifdef OSK_PLATFORM_X11

b32 x11_get_key_state(i32 key);
#define ISTATE_KEYDOWN_ACTION(k, a) g_input_state.a = x11_get_key_state(k)

#elif OSK_PLATFORM_WIN32

b32 win32_get_key_state(i32 key);
#define ISTATE_KEYDOWN_ACTION(k, a) g_input_state.a = win32_get_key_state(k)

#endif

extern void cb_init();
extern void cb_resize();
extern void cb_render(InputState istate, float dt);

#endif //! OSK_HH