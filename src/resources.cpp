#if !defined(OSK_RES_H)
#define OSK_RES_H

#include "resources.h"

struct Animation {
    float duration = 1.f;
    fvec2 start = {0, 0};
    u32 size = 1;
    b32 loop = true;
};

struct RESTilemap {
    const char* const name;
    i32 rows, cols;
};
#define DEF_TILEMAP(name, path, rows, cols) TILEMAP_##name,

enum E_TILEMAPS { ALL_TILEMAPS TILEMAP_COUNT };

#undef DEF_TILEMAP
// #define DEF_TILEMAP(name, path, rows, cols) [TILEMAP_##name] = {path, rows, cols},
#define DEF_TILEMAP(name, path, rows, cols) {path, rows, cols},

#define GET_TILEMAP_TEXTURE(name) g_tilemap_textures[TILEMAP_##name]
global GLTilemapTexture g_tilemap_textures[TILEMAP_COUNT];

struct RESBackground {
    const char *name;
};

RESBackground g_background_files[] {
#define B(name) name,
    ALL_BACKGROUNDS
#undef B
};

#define DEF_ANIM(character, name, ...) CHARACTER_##character##_anim_##name,
#define DEF_CHARACTER(name, tilemap, anim_count, ...) enum E_##name##_anims{   \
__VA_ARGS__ CHARACTER_##name##_animcount\
}; \


ALL_CHARACTERS


#undef DEF_ANIM
#undef DEF_CHARACTER

#define DEF_ANIM(character, name, ...)
#define DEF_CHARACTER(name, tilemap, anim_count, ...) CHARACTER_##name ,

enum E_CHARACTERS
{
    ALL_CHARACTERS CHARACTER_COUNT
};


#undef DEF_ANIM
#undef DEF_CHARACTER

#define DEF_ANIM(character, name, ...)
#define DEF_CHARACTER(name, tilemap, anim_count, ...) TILEMAP_##tilemap ,

global u64 CHARACTER_TO_TILEMAP[CHARACTER_COUNT] =
{
    ALL_CHARACTERS
};

#undef DEF_ANIM
#undef DEF_CHARACTER

#define DEF_ANIM(character, name, ...) {__VA_ARGS__},
#define DEF_CHARACTER(name, tilemap, anim_count, ...) \
global Animation CHARACTER_##name##_anims[anim_count] = { \
__VA_ARGS__ \
};

ALL_CHARACTERS

#undef DEF_ANIM
#undef DEF_CHARACTER

// Use this with a variable of type ux to hold current animation
#define GET_ANIM_BY_HANDLE(character, hnd) CHARACTER_##character##_anims[hnd]
#define GET_CHAR_ANIM_COUNT(character) CHARACTER_##character##_animcount
#define GET_CHAR_ANIM_HANDLE(character, name) CHARACTER_##character##_anim_##name
#define GET_CHAR_ANIM(character, name) CHARACTER_##character##_anims[CHARACTER_##character##_anim_##name]
#define GET_CHAR_TILEMAP(character) g_tilemap_textures[CHARACTER_TO_TILEMAP[ CHARACTER_##character ]]

#define GET_CHAR_ANIMSET(character) CHARACTER_##character##_anims

// Get the identifier of the animation
#define GET_CHAR_ANIMENUM(character, name) CHARACTER_##character##_anim_##name

internal u8*
load_image_as_rgba_pixels(const char* const name, i32* out_width, i32* out_height, i32* out_n) {
    int i_w, i_h, i_n;
    unsigned char* data = stbi_load(name, out_width, out_height, out_n, 4);
    
    return data;
}

internal void gl_load_background_texture(long bgn) {
    i32 w,h,n;
    u8 *data = load_image_as_rgba_pixels(g_background_files[bgn].name, &w, &h, &n);
    assert(data);
    gl_update_rgba_texture(data, w, h, g_background_texture_id);
    free(data);
}

internal void load_tilemap_textures() {
    i32 width, height, bpp;
    u8* data;
    
    const RESTilemap RES_TILEMAPS[TILEMAP_COUNT] = {
        ALL_TILEMAPS
    };
#undef DEF_TILEMAP
    
    assert(TILEMAP_COUNT > 0);
    
    for (u32 i = 0 ;i < TILEMAP_COUNT; ++i) {
        inform("loading tilemap: %s...",RES_TILEMAPS[i].name);
        data = load_image_as_rgba_pixels(RES_TILEMAPS[i].name,
                                         &width, &height, &bpp);
        assert(data);
        
        g_tilemap_textures[i] = gl_load_rgba_tilemap(data, width, height, RES_TILEMAPS[i].rows, RES_TILEMAPS[i].cols);
        
    }
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        fprintf(stderr, "TMerr: %d\n", err);
        exit(4);
    }
    
}

////////////////////////////////
// AUDIO
internal RESSound Wave_load_from_file(const char* file);

#define DEF_SOUND(name, path) name,
enum E_ALL_SOUNDS {
    ALL_SOUNDS
        ALL_SOUNDS_COUNT
};
#undef DEF_SOUND

#define DEF_SOUND(name, path) path,
const char *ALL_SOUND_PATHS[ALL_SOUNDS_COUNT] = {
    ALL_SOUNDS
};
#undef DEF_SOUND

RESSound ALL_SOUND_RES[ALL_SOUNDS_COUNT];
#define GET_SOUND(name) ((RESSound *)&ALL_SOUND_RES[name])

internal void load_sound_resources() {
    for (u32 i = 0; i < ALL_SOUNDS_COUNT; ++i) {
        ALL_SOUND_RES[i] = Wave_load_from_file(ALL_SOUND_PATHS[i]);
    }
}

#endif //!OSK_RES_H
