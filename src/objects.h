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


struct Sprite
{
    GLTilemapTexture const * tilemap = 0;
    ivec2 size = {64, 64};
    ivec2 position = {0,0};
    float rotation = 0.f;
    AABox collision_box = {0,0,64,64};
    ivec2 mirror = {false, false};
    ivec2 velocity = {0,0};
    // TODO(miked): Maybe figure out a method of keeping custom data
    // on a per sprite basis?
    b32 is_on_air = false;
    
    i32 current_frame = 0;
    u32 current_animation = 0;
    float time_accumulator = 0.f;
    Animation* animation_set;
    
    Entity entity;
    
    AABox get_transformed_AABox() const
    {
        return this->collision_box.translate(this->position);
    }
};
