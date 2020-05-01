#ifndef OSK_HH
#define OSK_HH
#include <stdint.h>

/* Compile Time options:
OSK_ROUND_TO_POW_2
 :: Rounds the viewport scale to the power of two closest to the
:: current scale; Use this if tilemap has seams, although it
:: should be fixed by using texture arrays.
OSK_EXCLUDE_MSG_EXPR
:: excludes the stringified expression from messages
*/
//#define OSK_ROUND_TO_POW_2
//#define OSK_EXCLUDE_MSG_EXPR
inline void _exit_with_message(char* message)
{
    puts(message);
    fflush(stdout); // Will now print everything in the stdout buffer
    exit(-1);
}

#ifdef OSK_EXCLUDE_MSG_EXPR
#define warn_unless(expr, msg) if (!(expr)) puts("[WARNING] " #msg)
#define fail_unless(expr, msg) if (!(expr)) _exit_with_message("[ERROR] " #msg)
#else
#define warn_unless(expr, msg) if (!(expr)) puts("[WARNING] " #msg "\n\t" #expr)
#define fail_unless(expr, msg) if (!(expr)) _exit_with_message("[ERROR] " #msg "\n\t" #expr)
#endif

void exit_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[ERROR]: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    exit(128);
}

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

#if defined(OSK_PLATFORM_WIN32)

#define inform(fmt, ...) printf("[INFO] " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) printf("[WARNING] " fmt "\n", __VA_ARGS__)
#define error(fmt, ...) printf("[ERROR] " fmt "\n", __VA_ARGS__)

#elif defined(OSK_PLATFORM_X11)

#define inform(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define warn(fmt, ...) printf("[WARNING] " fmt "\n", ##__VA_ARGS__)
#endif


#define assert(expr) do{if (!(expr)) { fprintf(stderr, "ASSERTION FAILED: %s:%d\n", __FILE__, __LINE__); exit(-1);}  }while(0)


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
typedef int32_t   b32;

u32    g_wind_width;
u32    g_wind_height;
u32    g_view_width;
u32    g_view_height;
double g_tile_scale;  // base scale for everything
double g_pixel_scale; // scale of a pixel(for non tile-aligned movement)

#define W_2_H 1.1428571428571428
#define HEIGHT_2_WIDTH_SCALE 0.875

#define TILEMAP_ROWS (12)
#define TILEMAP_COLS (15)

#include "input.h"

extern void cb_init();
extern void cb_resize();
extern void cb_render(InputState istate, u64 audio_sample_count, float dt);

internal char*
platform_load_entire_file(const char* path)
{
    u64 size;
    char* data;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    data = (char*)malloc(size + 1);
    assert(data);
    fread(data, size, 1, f);
    data[size] = 0;
    fclose(f);
    
    
    return data;
}

// AUDIO
#define AUDIO_SAMPLERATE 48000
#define AUDIO_CHANNELS 2
#define AUDIO_BPS 16
#define AUDIO_BYTESPERSAMPLE ((AUDIO_BPS / 8) * AUDIO_CHANNELS)
#define AUDIO_FRAMES 1024
#define AUDIO_BUFFER_SIZE (AUDIO_SAMPLERATE * AUDIO_BYTESPERSAMPLE)
//#define AUDIO_BUFFER_SIZE 1000
#define AUDIO_MAX_SOUNDS 32

struct RESSound
{
    u32 samplerate;
    u32 num_channels;
    u32 num_samples;
    void* data;
};

enum SoundType
{
    SoundEffect,
    Music,
};

struct Sound
{
    u64 counter = 0;
    u64 max_counter = ~(0u);
    b32 looping = false;
    b32 playing = true;
    const RESSound* resource = 0;
    
    SoundType type;
};

global struct
{
    
    Sound all_sounds[AUDIO_MAX_SOUNDS];
    i32 all_sounds_size = 0;
    //float volume = 0.5f;
    float volume = 0.0f;
    
    u8 buffer[AUDIO_BUFFER_SIZE] = {};
} g_audio;
#define M_PI 3.14159265359

#endif //! OSK_HH