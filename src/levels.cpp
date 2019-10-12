/* NOTE(miked): 
Attempting to switch to a non palleted level format.
*/

// This is a simple array list I wrote for C99. I'll probably regret
// using it, but life is all about living on the edge, and that means
// having potential creeping bugs sneaking up you when you least want it.
#define CA_TYPE Sprite
#include "calist.h"
#undef CA_TYPE

global struct
{
    
    EntityBaseType tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    // TODO(miked): Hidden items
    u64 hidden_tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    List_Sprite spritelist = {};
    
} g_scene;


internal void 
scene_sprite_add(Sprite* sprite)
{
    fail_unless(sprite, "Passing null sprite to scene_add");
    ca_push_array(Sprite, &g_scene.spritelist, sprite, 1);
}



internal const char*
string_trim(const char* c)
{
    while(*c == ' ' || *c == '\n') c++;
    return c;
}

internal const char* string_nextline(const char* c)
{
    while (*c != '\n') c++;
    return c + 1;
}

internal const char* string_parse(const char* c, const char* str)
{
    while (*str && *c && *c == *str)
    {
        c++;
        str++;
    }
    
    if (!(*str))
        return c;
    
    return 0;
}

internal const char*
string_parse_uint(const char* c, u64* out_i)
{
#define IS_DIGIT(x) (x >= '0' && x <= '9')
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
#undef IS_DIGIT
}


internal b32 is_valid_tilemap_object(EntityBaseType type)
{
    return ((u64)type <= (u64)eBlockSolid);
}

internal void level_load(char* data)
{
    const char* c = data;
#define OSK_LEVEL_FMT_VERSION "V0.2"
    fail_unless(string_parse(c, OSK_LEVEL_FMT_VERSION), "Version string does not match!");
    
    c += 5;
    inform("%s", "LOADING LEVEL...");
    
    u32 counter_x = 0;
    u32 counter_y = 0;
    
    while (*c)
    {
        switch(*c)
        {
            case '#':
            {
                c = string_nextline(c);
            }break;
            
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                u64 res;
                c = string_parse_uint(c, &res);
                
                fail_unless(res < EntityBaseType_Count,
                            "Entity index does not exist in version" 
                            OSK_LEVEL_FMT_VERSION);
                
                if (counter_x >= TILEMAP_COLS)
                {
                    counter_x = 0;
                    counter_y++;
                }
                
                if (is_valid_tilemap_object((EntityBaseType) res))
                    g_scene.tilemap[counter_x][counter_y] = (EntityBaseType)res;
                else
                {
                    Sprite sprite_to_make;
                    fvec2 sprite_initial_pos = fvec2{ (float)counter_x * 64, (float)counter_y * 64};
                    
                    
                    switch((EntityBaseType)res)
                    {
                        case eGoblin:
                        {
                            sprite_to_make = make_goblin(sprite_initial_pos);
                            c = goblin_parse_custom(&sprite_to_make, c);
                        }break;
                        
                        case ePlayerSpawnPoint:
                        {
                            sprite_to_make = make_player(sprite_initial_pos);
                        }break;
                        
                        default:
                        {
                            warn("sprite type not available for make_");
                        }break;
                    }
                    
                    scene_sprite_add(&sprite_to_make);
                    
                    g_scene.tilemap[counter_x][counter_y] = eEmptySpace;
                }
                
                counter_x++;
                
            } break;
            
            default:
            {
                c++;
            } break;
        }
    }
    
}

internal void scene_init(const char* level_path)
{
    //assert(level_path);
    char* lvl = platform_load_entire_file(level_path);
    level_load(lvl);
    free(lvl);
    
}

internal void
scene_draw_tilemap()
{
    for(int i = 0; i < 15; ++i )
    {
        for(int j = 0; j < 12; ++j )
        {
            u32 id;
            EntityBaseType type = (EntityBaseType)g_scene.tilemap[i][j];
            
            if (type == eEmptySpace) continue;
            if (type == eBlockSolid) id = 2;
            if (type == eBlockFrail) id = 1;
            
            gl_slow_tilemap_draw(
                &GET_TILEMAP_TEXTURE(test),
                {i * 64, j * 64},
                {64, 64},
                0.f,
                id);
        }
    }
}


inline u64 scene_get_tile(ivec2 p) { return g_scene.tilemap[p.x][p.y];}
inline void scene_set_tile(ivec2 p, EntityBaseType t) { g_scene.tilemap[p.x][p.y] = t; }


internal void ePlayer_update(Sprite* spref, InputState* istate, float dt);
internal void eGoblin_update(Sprite* spref, InputState* istate, float dt);

internal void 
scene_update(InputState* istate, float dt)
{
    scene_draw_tilemap();
    
    List_Sprite l = g_scene.spritelist;
    
    for (int i = 0; i < l.sz; ++i)
    {
        Sprite* spref = l.data + i;
        spref->update_animation(dt);
        
        switch(spref->entity.type)
        {
            case ePlayer:
            {
                ePlayer_update(spref, istate, dt);
            }break;
            
            case eGoblin:
            {
                eGoblin_update(spref, istate, dt);
            } break;
            
            default:
            break;
        }
        
        draw(l.data + i);
    }
}

// Finds first sprite of specific type
// if not found; return 0
internal const Sprite* const scene_get_first_sprite(EntityBaseType type)
{
    for (i32 i = 0; i < g_scene.spritelist.sz; i += 1)
    {
        Sprite* spref = &g_scene.spritelist.data[i];
        
        if (spref->entity.type == type) return spref;
    }
    return 0;
    
}

// Returns first view-blocking tile in search direction specified
// otherwise, returns {-1, -1};
internal ivec2 scene_get_first_nonempty_tile(ivec2 start_tile, ivec2 end_tile)
{
    i32 xdiff = sgn(end_tile.x - start_tile.x);
    while (start_tile != end_tile)
    {
        if (scene_get_tile(start_tile) != eEmptySpace)
        {
            return start_tile;
        }
        
        start_tile.x += xdiff;
        
        if (start_tile.x < 0 || start_tile.x > 14) return {-1, -1};
    }
    
    
    return ivec2{-1 ,-1};
}