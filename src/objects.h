#define MAX_ENTITY_PARAMS (3 + 4)
// Custom Parameters
// eEmptySpace :: hidden item
// eBlockFrail :: health, hidden item

/* NOTE: eEffect - scene_update _automatically_ marks
 * any eEffect entity for removal once it's animation is finished
*/

enum EntityBaseType {
    eEmptySpace = 0,
    eBlockFrail = 1,
    eBlockSolid = 2,
    eBlockFrailHalf = 3,
    ePlayerSpawnPoint = 4,
    ePlayer = 5,
    eEnemy = 6,
    eDoor = 7,
    eKey = 8,
    ePickup = 9,
    
    eFairie,
    eEffect,
    eDFireball,
    EntityBaseType_Count,
};

enum class EnemyType {
    Chimera,
    Demonhead,
    Dragon,
    Gargoyle,
    Ghost,
    Goblin,
    Nuel,
    Salamander,
    Wyvern,
    PanelMonster,
    EarthMage,
    SparkBall,
    BlueFlame,
    KMirror,
    Count
};

union CustomParameter {
    u64    as_u64;
    double as_f64;
    i64    as_i64;
    EnemyType as_etype;
    void *as_ptr;
};

inline bool tile_is_empty(EntityBaseType type) {
    if (type == eDoor || type == eEmptySpace) return true;
    
    return false;
}

struct Entity {
    EntityBaseType type;
    CustomParameter params[MAX_ENTITY_PARAMS];
};

#include "sprites.h"

#include <vector>
/* Thanks c++ stl! Now this'll speed up my productivity */
typedef std::vector<Sprite> List_Sprite;

struct Map {
    const char *name; // path!!!
    EntityBaseType tiles[TILEMAP_COLS][TILEMAP_ROWS];
    int hidden_pickups[TILEMAP_COLS][TILEMAP_ROWS];
    
    List_Sprite sprites;
    List_Sprite pickups;
    
    ivec2 exit_location;
    ivec2 key_location;
};

global struct {
    Map loaded_map;
    b32 playing = false;
    b32 paused_for_key_animation = false;
    int startup_state = 0;
    
    long player_score = 0;
    float last_score_timer = 0.f;
    u32 last_score_num = 0u;
    
    float player_time = 80.f;
    int player_lives = 3;
    bool player_has_key = false;
    bool time_is_low_enough = false;
} g_scene;


inline internal Sprite make_effect(fvec2 position, u32 effect_type) {
    return Sprite {
        .tilemap = &GET_CHAR_TILEMAP(Effect),
        .position = position,
        .collision_box = {0,0,64,64},
        .current_animation = effect_type,
        .animation_set = GET_CHAR_ANIMSET(Effect),
        .entity = {
            .type = eEffect,
        }
    };
}

inline internal Sprite make_goblin(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Goblin),
        .position = position,
        .collision_box = {5, 0, 45, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Goblin),
        .entity = {
            .type = eEnemy,
        }
    };
    
    res.entity.params[0].as_etype = EnemyType::Goblin;
    
    return res;
}

inline internal Sprite make_ghost(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Ghost),
        .position = position,
        .collision_box = {0, 0, 64, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Ghost),
        .entity = {
            .type = eEnemy,
        }
    };
    
    res.entity.params[0].as_etype = EnemyType::Ghost;
    
    return res;
}

inline internal Sprite make_player(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(test_player),
        .size = fvec2{64, 64},
        .position = position,
        .collision_box = {5, 14, 45, 50},
        .mirror = {true, false},
        .mark_for_removal = false,
        
        .animation_set = GET_CHAR_ANIMSET(test_player),
        .entity = {
            ePlayer,
            {0,0}
        }
    };
    
    return res;
}

inline internal Sprite make_door(fvec2 position) {
    return Sprite {
        .tilemap = &GET_CHAR_TILEMAP(Door),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .current_animation = GET_CHAR_ANIMENUM(Door, Close),
        .animation_set = GET_CHAR_ANIMSET(Door),
        .entity = {
            eDoor,
            {0,0}
        }
    };
}

inline internal Sprite make_dfireball(fvec2 position)
{
    Sprite res =
    {
        .tilemap = &GET_CHAR_TILEMAP(DFireball),
        .size = {55,55},
        .position = position,
        .collision_box = {8,8,40,40},
        .mirror = {true, false},
        .animation_set = GET_CHAR_ANIMSET(DFireball),
        
        .entity =
        {
            eDFireball,
            {0,0}
        }
    };
    
    SET_ANIMATION(&res, DFireball, Middle);
    
    return res;
}


inline internal Sprite make_starring(fvec2 position) {
    Sprite res =
    {
        .tilemap = &GET_CHAR_TILEMAP(Effect),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .current_animation = GET_CHAR_ANIMENUM(Effect, Star),
        .animation_set = GET_CHAR_ANIMSET(Effect),
        .entity = {
            EntityBaseType_Count,
            {u64(0.f),u64(0.f)}
        }
    };
    
    return res;
    
}

inline internal Sprite make_fairie(fvec2 position, u64 type) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Fairie),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        
        // type will go here!!!! and not in custom params
        .current_animation = GET_CHAR_ANIMENUM(Fairie, Default),
        .animation_set = GET_CHAR_ANIMSET(Fairie),
        .entity = {
            eFairie,
            {0,0}
        }
    };
    
    return res;
}

inline internal Sprite make_key(fvec2 position) {
    Sprite res =
    {
        .tilemap = &GET_CHAR_TILEMAP(Key),
        .size = {64,64},
        .position = position,
        .rotation = 0.f,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        
        .animation_playing = false,
        .current_animation = 0,
        .animation_set = GET_CHAR_ANIMSET(Key),
        
        .entity = {
            eKey,
            {u64(0.f),u64(0.f)}
        }
    };
    
    return res;
    
}

inline internal Sprite make_pickup(fvec2 position, u64 type, u64 id = 0) {
    Sprite res = {
        
        .tilemap = &GET_TILEMAP_TEXTURE(TM_pickups),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .animation_playing = false,
        .current_animation = 0,
        .animation_set = 0,
        .entity = {
            ePickup,
            {type, 0}
        }
    };
    
    res.entity.params[2].as_u64 = id;
    
    return res;
}

inline internal Sprite make_blueflame(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(BlueFlame),
        .size = {64,64},
        .position = position,
        .collision_box = {16, 0, 32, 64},
        .animation_playing = true,
        .current_animation = GET_CHAR_ANIMENUM(BlueFlame, Normal),
        .animation_set = GET_CHAR_ANIMSET(BlueFlame),
        .entity = {
            eEnemy,
            {0, 0, 0}
        }
    };
    
    result.entity.params[0].as_etype = EnemyType::BlueFlame;
    result.entity.params[1].as_f64 = +1.0;
    result.entity.params[2].as_f64 = -FLT_MAX;
    
    return result;
}

inline internal Sprite make_kmirror(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(KMirror),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .animation_playing = false,
        .current_animation = GET_CHAR_ANIMENUM(KMirror, Default),
        .animation_set = GET_CHAR_ANIMSET(KMirror),
        .entity = {
            eEnemy,
            {0,0,0, 0, 0,}
        }
    };
    
    result.entity.params[0].as_etype = EnemyType::KMirror;
    result.entity.params[1].as_f64 = 1.f;
    result.entity.params[2].as_f64 = 1.f;
    result.entity.params[3].as_ptr = 0;
    result.entity.params[4].as_ptr = 0;
    
    return result;
}

internal char *parse_double(char *c, double *d){
    char *end;
    *d = strtod(c, &end);
    return end;
}

internal char *parse_long(char *c, long *d) {
    char *end;
    *d = strtol(c, &end, 10);
    return end;
}

internal char *parse_custom_double(Sprite *s, char *c, int index, double default_val) {
    s->entity.params[index].as_f64 = default_val;
    
    if (*c == ',') {
        c++;
        double cval;
        c = parse_double(c, &cval);
        s->entity.params[index].as_f64 = cval;
    }
    return c;
}

internal char *parse_custom_bool32(i32 *dst, char *c, bool default_val) {
    *dst = default_val;
    if (*c == ',') {
        c++;
        long cval;
        c = parse_long(c, &cval);
        *dst = (cval == 0) ? false : true;
    }
    return c;
}

internal char *parse_enemy(Sprite *enemy, char *c, fvec2 sprite_initial_pos);
internal char *parse_enemy_type(char *c, EnemyType *type);
internal char *parse_enemy_custom(Sprite *s, char *c, EnemyType type, fvec2 sprite_initial_pos);

internal char *Goblin_custom(Sprite *goblin, char *c) {
    c = parse_custom_double(goblin, c, 1, 80.f);
    c = parse_custom_bool32(&goblin->mirror.x, c, false);
    
    return c;
}

internal char *Ghost_custom(Sprite *ghost, char *c) {
    c = parse_custom_double(ghost, c, 1, 200.f);
    //c = parse_custom_double(ghost, c, 1, 200.f);
    
    //c = parse_custom_bool32(&ghost->mirror.x, c, false);
    return c;
}

internal char* BlueFlame_custom(Sprite *flame, char *c) {
    c = parse_custom_double(flame, c, 1, 1.f);
    
    return c;
}

internal char *parse_kmirror_enemy(Sprite *mirror, char *c, int index, void *def, fvec2 pos) {
    
    mirror->entity.params[index].as_ptr = (void*)0;
    if (*c == ',') {
        c++;
        assert(*c == '{');
        c++;
        
        EnemyType type;
        {
            long tl;
            c = parse_long(c, &tl);
            type = (EnemyType)tl;
            assert(type < EnemyType::Count);
        }
        Sprite *spr = (Sprite *)malloc(sizeof(Sprite));
        c = parse_enemy_custom(spr, c, type, pos);
        mirror->entity.params[index].as_ptr = (void*)spr;
        assert(*c == '}');
        c++;
    }
    return c;
}

internal char *KMirror_custom(Sprite *mirror, char *c, fvec2 pos) {
    // NOTE(miked): we dont store exact enemy info in the params
    // (we'd have to add a lot of custom params then). Instead we malloc a Sprite,
    // parse the params for it and add a pointer to it in the mirror. Whenever its
    // spawned we just clone that pointer with map_add
    // TODO(NAME): highly propable memory leak!
    mirror->entity.params[1].as_f64 = 0.f;
    mirror->entity.params[2].as_f64 = 0.f;
    c = parse_custom_double(mirror, c, 3, 1.f);
    c = parse_custom_double(mirror, c, 4, 1.f);
    c = parse_kmirror_enemy(mirror, c, 5, 0, pos);
    c = parse_kmirror_enemy(mirror, c, 6, 0, pos);
    return c;
}

internal char *parse_enemy(Sprite *enemy, char *c, fvec2 sprite_initial_pos) {
    EnemyType type;
    c = parse_enemy_type(c, &type);
    c = parse_enemy_custom(enemy, c, type, sprite_initial_pos);
    
    return c;
}

internal char *parse_enemy_type(char *c, EnemyType *type) {
    long tl;
    assert(*c == ',');
    c++;
    
    c = parse_long(c, &tl);
    *type = (EnemyType)tl;
    return c;
}

internal char *parse_enemy_custom(Sprite *s, char *c, EnemyType type, fvec2 sprite_initial_pos) {
    switch(type) {
        case EnemyType::Goblin: {
            *s = make_goblin(sprite_initial_pos);
            c = Goblin_custom(s, c);
        }break;
        
        case EnemyType::Ghost: {
            *s = make_ghost(sprite_initial_pos);
            c = Ghost_custom(s, c);
        }break;
        
        case EnemyType::BlueFlame: {
            *s = make_blueflame(sprite_initial_pos);
            c = BlueFlame_custom(s, c);
        }break;
        
        case EnemyType::KMirror: {
            *s = make_kmirror(sprite_initial_pos);
            c = KMirror_custom(s, c, sprite_initial_pos);
        }break;
    }
    return c;
}

internal char *ePickup_parse(Sprite* pickup, char* c) {
    if (*c == ',') {
        c++;
        long type;
        c = parse_long(c, &type);
        
        if (pickup_type_is_valid((PickupType)type)) {
            pickup->entity.params[0].as_u64 = type;
            
        }
        
    }
    return c;
}

internal void draw(Sprite const * sprite) {
    
    if (!sprite->tilemap) {
        exit_error("tilemap %d !!", sprite->entity.type);
    }
    
    i32 frame_to_render = sprite->current_frame;
    
    if (sprite->animation_set) {
        Animation* anim_ref = &sprite->animation_set[sprite->current_animation];
        frame_to_render =
            anim_ref->start.y * sprite->tilemap->cols
            + anim_ref->start.x + sprite->current_frame;
    }
    
    
    gl_slow_tilemap_draw(sprite->tilemap,
                         {(float)sprite->position.x, (float)sprite->position.y},
                         {(float)sprite->size.x, (float)sprite->size.y},
                         sprite->rotation,
                         frame_to_render,
                         sprite->mirror.x, sprite->mirror.y);
}

internal void
draw_pickup(Sprite const *sprite) {
    assert(sprite->tilemap);
    PickupType type = (PickupType)sprite->entity.params[0].as_u64;
    
    u64 index_to_draw;
    index_to_draw = sprite->entity.params[0].as_u64;
    
    if (sprite->entity.params[1].as_u64 == 0) {
        gl_slow_tilemap_draw(sprite->tilemap,
                             {(float)sprite->position.x, (float)sprite->position.y},
                             {(float)sprite->size.x, (float)sprite->size.y},
                             sprite->rotation,
                             index_to_draw,
                             sprite->mirror.x, sprite->mirror.y);
    }
}