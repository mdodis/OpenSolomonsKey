#ifndef OSK_HH
#define OSK_HH
#include <stdint.h>

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

extern u32    g_wind_width;
extern u32    g_wind_height;

extern u32    g_view_width;
extern u32    g_view_height;

extern double g_tile_scale;

#define W_2_H 1.1428571428571428
#define HEIGHT_2_WIDTH_SCALE 0.875

#define HORIZ_SCREEN_PAD (g_tile_scale / 2.0)
#define VERTC_SCREEN_PAD (g_tile_scale)

#define IS_POW2(x) x && !(x & (x - 1))

extern void cb_init();
extern void cb_resize();
extern void cb_render();

global u32 g_text_id;

#endif //! OSK_HH