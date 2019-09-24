struct GLShader
{
    u32 id = 0;
    
    void apply();
    void create(const char* const vsrc, const char* const fsrc);
};

struct GLTilemapTexture
{
    u32 texture_id;
    i32 width, height;
    i32 rows, cols;
};


struct Animation
{
    float duration = 1.f;
    ivec2 start = {0, 0};
    u32 size = 1;
    b32 loop = true;
};

#define MAX_ENTITY_PARAMS 2

#define TILEMAP_ROWS 12
#define TILEMAP_COLS 15

// Custom Parameters
// eEmptySpace :: hidden item
// eBlockFrail :: health, hidden item

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
    ePlayer,
    eGoblin,
    
    EntityBaseType_Count,
};

struct Entity
{
    EntityBaseType type;
    CustomParameter params[MAX_ENTITY_PARAMS];
};

