#if !defined(OSK_RES_H)
#define OSK_RES_H

struct RESTilemap
{
    const char* const name;
    i32 rows, cols;
};

#define DEF_TILEMAP(name, path, rows, cols) name,

#define ALL_TILEMAPS \
/*          NAME     PATH IN FOLDER     rows cols*/ \
DEF_TILEMAP(TM_TEST, "test_tilemap.png",6   ,5   )

////////////////////////////////
enum E_TILEMAPS { ALL_TILEMAPS TM_COUNT };

#undef DEF_TILEMAP
#define DEF_TILEMAP(name, path, rows, cols) [name] = {path, rows, cols},

#define GET_TILEMAP_TEXTURE(i) g_tilemap_textures[i]
global GLTilemapTexture g_tilemap_textures[TM_COUNT];
#endif //!OSK_RES_H
