#if !defined(OSK_RES_H)
#define OSK_RES_H

////////////////////////////////
////////////////////////////////
/* Tilemaps
Define all of the tilemaps by associating them with a name, and pass
the extra required information. All of the rendering is done through
tilemaps.
*/
#define ALL_TILEMAPS \
/*          NAME     PATH IN FOLDER                                rows cols*/\
DEF_TILEMAP(test    ,"test_tilemap.png"                           ,6    ,5   )\
DEF_TILEMAP(dana    ,"res/joined/dana_all.png"                    ,3    ,6   )\


struct RESTilemap
{
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
////////////////////////////////
////////////////////////////////


////////////////////////////////
////////////////////////////////
/* ANIMATIONS
You can group animations by "character", and the macro ugliness here
should prevent name collisions, if you input different character names.
*/
#define ALL_CHARACTERS                                                         \
/*            Name,        Tilemap, Animation Count */                         \
DEF_CHARACTER(test_player, dana,    3,                                         \
/*       Character,   Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(test_player, Idle,    .1f,      {0,0}, 1,      false)   \
DEF_ANIM(test_player, Run ,    .1f,      {0,1}, 5,      true )   \
DEF_ANIM(test_player, Cast,    .5f,      {0,2}, 3,      false)   \
)                                                                \
DEF_CHARACTER(test_enemy, test,     1,                                         \
/*       Character,   Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(test_enemy,  Idle,    .5f,      {0,0}, 4,      true)    \
)                                                                \


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
#define GET_CHAR_ANIMENUM(character, name) CHARACTER_##character##_anim_##name

////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////
////////////////////////////////

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

internal void load_tilemap_textures()
{
    i32 width, height, bpp;
    u8* data;
    
    const RESTilemap RES_TILEMAPS[TILEMAP_COUNT] = 
    {
        ALL_TILEMAPS
    };
#undef DEF_TILEMAP
    
    assert(TILEMAP_COUNT > 0);
    
    for (u32 i = 0 ;i < TILEMAP_COUNT; ++i)
    {
        data = load_image_as_rgba_pixels(
            RES_TILEMAPS[i].name,
            &width, &height, &bpp);
        assert(data);
        
        g_tilemap_textures[i] = gl_load_rgba_tilemap(
            data,
            width, height,
            RES_TILEMAPS[i].rows, RES_TILEMAPS[i].cols);
        
    }
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("err: %d\n", err);
        exit(-1);
    }
    
}


#endif //!OSK_RES_H
