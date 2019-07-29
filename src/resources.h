#if !defined(OSK_RES_H)
#define OSK_RES_H

struct RESTilemap
{
    const char* const name;
    i32 rows, cols;
};

enum E_TILEMAPS
{
    TM_TEST,
    TM_COUNT
};

#define GET_TILEMAP_TEXTURE(i) g_tilemap_textures[i]
global GLTilemapTexture g_tilemap_textures[TM_COUNT];
global const RESTilemap RES_TILEMAPS[TM_COUNT] = 
{
    [TM_TEST] = {"test_tilemap.png", .rows = 6, .cols = 5}
};


#endif //!OSK_RES_H
