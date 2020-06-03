#if !defined(OSK_RES_H)
#define OSK_RES_H

struct Animation {
    float duration = 1.f;
    fvec2 start = {0, 0};
    u32 size = 1;
    b32 loop = true;
};

/* Tilemaps
Define all of the tilemaps by associating them with a name, and pass
the extra required information. All of the rendering is done through
tilemaps.
*/
#define ALL_TILEMAPS \
/*          NAME               PATH IN FOLDER                             rows cols*/\
DEF_TILEMAP(TM_logo            ,"res/logo3.png"                           ,1    ,1  )\
DEF_TILEMAP(TM_essentials      ,"res/essentials.png"                      ,5    ,5  )\
DEF_TILEMAP(TM_dana            ,"res/dana_all.png"                        ,5    ,6  )\
DEF_TILEMAP(TM_dana_fire       ,"res/dana_fireball.png"                   ,3    ,1  )\
DEF_TILEMAP(TM_fairies         ,"res/fairies.png"                         ,1    ,2  )\
DEF_TILEMAP(TM_demonhead       ,"res/demon_head.png"                      ,2    ,8  )\
DEF_TILEMAP(TM_goblin          ,"res/goblin_all.png"                      ,4    ,7  )\
DEF_TILEMAP(TM_blueflame       ,"res/blue_flame.png"                      ,2    ,7  )\
DEF_TILEMAP(TM_ghost           ,"res/ghost_all.png"                       ,2    ,3  )\
DEF_TILEMAP(TM_panelmonster    ,"res/panel_monster_all.png"               ,3    ,5  )\
DEF_TILEMAP(TM_wyvern          ,"res/wyvern_all.png"                      ,2    ,3  )\
DEF_TILEMAP(TM_dragon          ,"res/dragon_all.png"                      ,4    ,5  )\
DEF_TILEMAP(TM_effects         ,"res/effects.png"                         ,5    ,4  )\
DEF_TILEMAP(font               ,"res/font.png"                            ,6    ,16 )\
DEF_TILEMAP(misc               ,"res/misc.png"                            ,4    ,4  )\
DEF_TILEMAP(TM_pickups         ,"res/pickups.png"                         ,5    ,7  )\

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

/* Backgrounds
Lol
*/
#define ALL_BACKGROUNDS \
B("res/bg/0.png")\
B("res/bg/1.png")\
B("res/bg/2.png")\
B("res/bg/3.png")\
B("res/bg/4.png")\
B("res/bg/5.png")\
B("res/bg/6.png")\
B("res/bg/7.png")\
B("res/bg/8.png")\
B("res/bg/9.png")\
B("res/bg/10.png")\
B("res/bg/11.png")\
B("res/bg/12.png")\
B("res/bg/13.png")\
B("res/bg/14.png")\
B("res/bg/15.png")\
B("res/bg/16.png")\
B("res/bg/17.png")\
B("res/bg/18.png")\

struct RESBackground {
    const char *name;
};

RESBackground g_background_files[] {
#define B(name) name,
    ALL_BACKGROUNDS
#undef B
};

/* ANIMATIONS
You can group animations by "character", and the macro ugliness here
should prevent name collisions, if you input different character names.
*/
#define ALL_CHARACTERS                                                              \
/*            Name,        Tilemap,  Animation Count */                         \
DEF_CHARACTER(Dana,                  TM_dana,            5,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Dana,                  Idle,    .1f,      {0,0}, 1,      false)   \
DEF_ANIM(Dana,                  Run ,    .1f,      {0,1}, 5,      true )   \
DEF_ANIM(Dana,                  Cast,    .05f,     {0,2}, 3,      false)   \
DEF_ANIM(Dana,                  Crouch,  .05f,     {0,3}, 1,      true )   \
DEF_ANIM(Dana,                  Die,     .1f,      {0,4}, 4,      false)   \
)                                                                 \
DEF_CHARACTER(Goblin,                TM_goblin,          5,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Goblin,                Walk,    .2f,      {0,0}, 6,      true )   \
DEF_ANIM(Goblin,                Punch,   .18f,     {0,1}, 4,      false)   \
DEF_ANIM(Goblin,                Chase,   .15f,     {0,2}, 7,      true )   \
DEF_ANIM(Goblin,                Wait,    1.f,      {5,0}, 1,      false)   \
DEF_ANIM(Goblin,                Fall,    .1f,      {0,3}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(Ghost,                 TM_ghost,           2,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Ghost,                 Fly,     .1f,      {0,0}, 3,      true )   \
DEF_ANIM(Ghost,                 Punch,   .1f,      {0,1}, 3,      false)   \
)                                                                 \
DEF_CHARACTER(BlueFlame,             TM_blueflame,      2,                      \
DEF_ANIM(BlueFlame,             Normal,  .1f,      {0,0}, 7,      true )   \
DEF_ANIM(BlueFlame,             Tame,    .1f,      {0,1}, 4,      true )   \
)                                                                 \
DEF_CHARACTER(DemonHead,             TM_demonhead,      2,                      \
DEF_ANIM(DemonHead,             Normal,  .1f,      {0,0}, 8,      true )   \
DEF_ANIM(DemonHead,             Fade,    .1f,      {0,1}, 8,      false)   \
)                                                                 \
DEF_CHARACTER(PanelMonster,          TM_panelmonster,   2,                      \
DEF_ANIM(PanelMonster,          Wait,    0.0f,     {0,0}, 1,      true )   \
DEF_ANIM(PanelMonster,          Fire,    0.1f,     {0,0}, 5,      false)   \
)                                                                 \
DEF_CHARACTER(PanelMonsterFlame,     TM_panelmonster,   3,                      \
DEF_ANIM(PanelMonsterFlame,     Create,  0.2f,     {0,1}, 3,      false)   \
DEF_ANIM(PanelMonsterFlame,     Default, 0.2f,     {0,2}, 4,      true )   \
DEF_ANIM(PanelMonsterFlame,     Hit,     0.2f,     {3,1}, 2,      false)   \
)                                                                 \
DEF_CHARACTER(Wyvern,                TM_wyvern,         2,                      \
DEF_ANIM(Wyvern,                Default, 0.1f,     {0,0}, 3,      true )   \
DEF_ANIM(Wyvern,                Hit,     0.1f,     {0,0}, 3,      false)   \
)                                                                 \
DEF_CHARACTER(Dragon,                TM_dragon,         5,                      \
DEF_ANIM(Dragon,                Walk,    0.2f,     {0,0}, 5,      true )   \
DEF_ANIM(Dragon,                Fire,    0.2f,     {3,1}, 1,      true )   \
DEF_ANIM(Dragon,                Turn,    0.2f,     {4,3}, 1,      false)   \
DEF_ANIM(Dragon,                TurnWait,2.2f,     {4,0}, 1,      false)   \
DEF_ANIM(Dragon,                FireWait,0.34f,    {0,1}, 3,      false)   \
)                                                                 \
DEF_CHARACTER(DragonFire,            TM_dragon,         1,                      \
DEF_ANIM(DragonFire,            Default, 0.2f,     {0,2}, 4,      false)   \
)                                                                 \
DEF_CHARACTER(DFireball,             TM_dana_fire,       3,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(DFireball,             Down,    .1f,      {0,0}, 1,      true )   \
DEF_ANIM(DFireball,             Middle,  .1f,      {0,1}, 1,      true )   \
DEF_ANIM(DFireball,             Up,      .1f,      {0,2}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(Effect,                TM_effects,        5,                      \
DEF_ANIM(Effect,                Smoke,   .1f,      {0,0}, 4,      false)   \
DEF_ANIM(Effect,                Star,    .1f,      {0,1}, 3,      true )   \
DEF_ANIM(Effect,                Flash,   .1f,      {0,2}, 2,      false)   \
DEF_ANIM(Effect,                Flash2,  .1f,      {0,3}, 3,      false)   \
DEF_ANIM(Effect,                Hit,     .1f,      {0,4}, 4,      false)   \
)                                                                 \
DEF_CHARACTER(Door,                  TM_essentials,     2,                      \
DEF_ANIM(Door,                  Close,   .1f,      {0,2}, 5,      false)   \
DEF_ANIM(Door,                  Open,    .1f,      {0,3}, 5,      false)   \
)                                                                 \
DEF_CHARACTER(Key,                   TM_essentials,     1,                      \
DEF_ANIM(Key,                   Default, .1f,      {0,4}, 1,      false)   \
)                                                                 \
DEF_CHARACTER(Fairie,                TM_fairies,        1,                      \
DEF_ANIM(Fairie,                Default, .1f,      {0,0}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(KMirror,               TM_essentials,     1,                      \
DEF_ANIM(KMirror,               Default, .1f,      {3,1}, 1,      false)   \
)                                                                 \

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
    if (err != GL_NO_ERROR) {
        printf("TMerr: %d\n", err);
        exit(-1);
    }
    
}

////////////////////////////////
// AUDIO
internal RESSound Wave_load_from_file(const char* file);

#define ALL_SOUNDS \
DEF_SOUND(SND_background, "res/audio/bgm1.wav" ) \
DEF_SOUND(SND_jump,       "res/audio/bloop.wav") \
DEF_SOUND(SND_show_key,   "res/audio/key_appear.wav") \
DEF_SOUND(SND_show_player,"res/audio/player_appear.wav") \
DEF_SOUND(SND_hurry,      "res/audio/hurry.wav") \

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
