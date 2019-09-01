/* NOTE(miked): 
Attempting to switch to a non palleted level format.
*/

#define MAX_ENTITY_PARAMS 2
union CustomParameter
{
    u64    as_u64;
    double as_f64;
};

enum EntityBaseType
{
    eEmptySpace,
    eBlockFrail,
    eBlockSolid,
    ePlayerSpawnPoint,
    eGoblin,
    
    EntityBaseType_Count,
};

struct Entity
{
    EntityBaseType type;
    CustomParameter params[MAX_ENTITY_PARAMS];
};

#define TILEMAP_ROWS 12
#define TILEMAP_COLS 15
struct
{
    u64 tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    u64 hidden_tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    Sprite* spritelist = 0;
    i32 spritelist_size = 0;
    i32 spritelist_cap = 0;
    
} g_scene;

#define IS_DIGIT(x) (x >= '0' && x <= '9')

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
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
}

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

internal void level_load(char* data)
{
    const char* c = data;
#define OSK_LEVEL_FMT_VERSION "V0.2"
    fail_unless(string_parse(c, OSK_LEVEL_FMT_VERSION), "Version string does not match!");
    
    c += 5;
    puts("LOADING LEVEL...");
    
    u32 counter_x = 0;
    u32 counter_y = 0;
    
    while (*c)
    {
        switch(*c)
        {
            case '#':
            {
                puts("comment");
                c = string_nextline(c);
                printf("%c\n", *c);
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
                // TODO(miked): add custom parameters with commas!
                
                fail_unless(res < EntityBaseType_Count,
                            "Entity index does not exist in version" 
                            OSK_LEVEL_FMT_VERSION);
                printf("%ld", res);
                if (*c == ' ' ) printf("x ");
                else if (*c == '\n') printf("n ");
                
                if (counter_x >= TILEMAP_COLS)
                {
                    counter_x = 0;
                    printf("\t%d\n", counter_y);
                    counter_y++;
                }
                
                g_scene.tilemap[counter_x][counter_y] = res;
                
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
    //load_test_level(g_scene.palette, &g_scene.palette_size);
    char* lvl = platform_load_entire_file("lvl1.osk");
    level_load(lvl);
    free(lvl);
    
    //exit(0);
}

internal void 
scene_sprite_add(Sprite* sprite)
{
    fail_unless(sprite, "Passing null sprite to scene_add");
    if (!g_scene.spritelist)
    {
        g_scene.spritelist = (Sprite*)malloc(10 * sizeof(Sprite));
        g_scene.spritelist_size = 0;
        g_scene.spritelist_cap = 10;
        fail_unless(g_scene.spritelist, "");
    }
    fail_unless((g_scene.spritelist_size - 1) < g_scene.spritelist_cap, "");
    
    g_scene.spritelist[g_scene.spritelist_size++] = *sprite;
}
