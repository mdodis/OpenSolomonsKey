#define MAX_ENTITY_PARAMS 2
// Custom Parameters
// eEmptySpace :: hidden item
// eBlockFrail :: health, hidden item

/* NOTE: eEffect - scene_update _automatically_ marks
 * any eEffect entity for removal once it's animation is finished
*/

enum EntityBaseType {
    eEmptySpace,
    eBlockFrail,
    eBlockSolid,
    eBlockFrailHalf,
    ePlayerSpawnPoint,
    ePlayer,
    eEnemy,
    eDoor,
    eKey,
    ePickup,
    
    eBlueFlame,
    eFairie,
    eEffect,
    eDFireball,
    EntityBaseType_Count,
};

enum class EnemyType {
    Goblin,
    Ghost,
    KMirror,
    Count
};

enum PickupType {
    Bag100,
    Bag200,
    Bag500,
    Bag1000,
    Bag2000,
    Bag5000,
    Bag10000,
    Bag20000,
    Jewel100,
    Jewel200,
    Jewel500,
    Jewel1000,
    Jewel2000,
    Jewel5000,
    Jewel10000,
    Jewel20000,
    Jewel50000,
    
    Bell,
    Bell2,
    
    // effect stuff
    Count
};


union CustomParameter {
    u64    as_u64;
    double as_f64;
    i64    as_i64;
    EnemyType as_etype;
};

internal bool pickup_type_is_valid(PickupType type) {
    return(type >= 0 && type < PickupType::Count);
}

internal bool pickup_type_is_non_effect(PickupType type) {
    return pickup_type_is_valid(type) && (type < Jewel50000);
}

internal bool pickup_is_bell(PickupType type) {
    return pickup_type_is_valid(type) && (type == Bell || type == Bell2);
}

internal long get_pickup_worth(PickupType type) {
    assert(pickup_type_is_valid(type));
    
    switch(type) {
        case Bag100:
        case Jewel100: {
            return 100;
        }break;
        
        case Bag200:
        case Jewel200: {
            return 200;
        }break;
        
        case Bag500:
        case Jewel500: {
            return 500;
        }break;
        
        case Bag1000:
        case Jewel1000: {
            return 1000;
        }break;
        
        case Bag2000:
        case Jewel2000: {
            return 2000;
        }break;
        
        case Bag5000:
        case Jewel5000: {
            return 5000;
        }break;
        
        case Bag10000:
        case Jewel10000: {
            return 10000;
        }break;
        
        case Bag20000:
        case Jewel20000: {
            return 20000;
        }break;
        case Jewel50000: {
            return 50000;
        }break;
    }
    assert(0);
    return -1;
}

inline bool tile_is_empty(EntityBaseType type) {
    if (type == eDoor || type == eEmptySpace) return true;
    
    return false;
}

struct Entity {
    EntityBaseType type;
    CustomParameter params[MAX_ENTITY_PARAMS];
};

struct Sprite {
    GLTilemapTexture const * tilemap = 0;
    fvec2 size = {64, 64};
    fvec2 position = {0,0};
    float rotation = 0.f;
    AABox collision_box = {0,0,64,64};
    ivec2 mirror = {false, false};
    fvec2 velocity = {0,0};
    b32 is_on_air = false;
    b32 mark_for_removal = false;                 // set to true to remove the sprite
    
    // Animation stuff
    b32 animation_playing = true;
    i32 current_frame = 0;
    u32 current_animation = 0;
    float time_accumulator = 0.f;
    Animation* animation_set;
    
    Entity entity;
    
    inline AABox get_transformed_AABox() const
    {
        return this->collision_box.translate(this->position);
    }
    
    void collide_aabb(AABox* target)
    {
        fail_unless(target, "");
        
        AABox this_collision = this->get_transformed_AABox();
        
        fvec2 diff;
        b32 collided = aabb_minkowski(&this_collision, target, &diff);
        b32 collided_on_bottom = false;
        if (collided)
        {
            this->position = this->position - (diff);
            
            // If we are moving up and diff moved us in the Y dir,
            // then negate the collision.
            // (fixes bouncing when hitting corner of a tile)
            if (this->velocity.y < 0 && iabs(diff.y) < 5)
            {
                this->position.y += diff.y;
                goto finished;
            }
            
            // NOTE(miked): if on top?
            if (this_collision.min_y < target->min_y)
            {
                collided_on_bottom = true;
                if (iabs(diff.y) > 0)
                {
                    this->is_on_air = false;
                    this->velocity.y = 0;
                }
            }
        }
        
        finished:
        if (!collided_on_bottom && this->velocity.y != 0)
            this->is_on_air = true;
        
    }
    
    
    void update_animation(float dt) {
        
        if (!this->animation_set) return;
        // get animation ref
        Animation* anim_ref;
        anim_ref = &this->animation_set[this->current_animation];
        if (anim_ref->size == 0 || !this->animation_playing)
            return;
        
        // increase time by dt
        if (this->animation_playing) {
            
            this->time_accumulator += dt;
            if (this->time_accumulator >= anim_ref->duration) {
                this->current_frame++;
                if (this->current_frame >= anim_ref->size ) {
                    // if its looping
                    if (anim_ref->loop) {
                        this->current_frame = 0;
                    }else {
                        this->animation_playing = false;
                        this->current_frame--;
                    }
                }
                
                this->time_accumulator = 0.f;
            }
        }
        
    }
    
#define SET_ANIMATION(spr, c, n) (spr)->set_animation_index(GET_CHAR_ANIMENUM(c, n))
    void set_animation_index(u32 anim_idx) {
        fail_unless(this->current_animation >= 0, "set_animation_index, invalid animation index!");
        
        if (this->current_animation != anim_idx) {
            this->animation_playing = true;
            this->current_animation = anim_idx;
            this->current_frame = 0;
            this->time_accumulator = 0;
        }
        
    }
    
    void move_and_collide(float dt,
                          const float GRAVITY,
                          const float MAX_YSPEED,
                          const float JUMP_STRENGTH,
                          float XSPEED,
                          b32 damage_tiles = false);
    
    void collide_sprite(float dt);
    
    inline float direction() {
        return this->mirror.x ? 1 : -1;
    }
    
    b32 jump(i32 strength)
    {
        
        if (!this->is_on_air)
        {
            this->velocity.y = -strength;
            this->is_on_air = true;
            // TODO(miked): return if we jumped
            //audio_play_sound(&this_jump_sound);
            
            return true;
        }
        return false;
        
    }
};

#include <vector>
/* Thanks c++ stl! Now this'll speed up my productivity */
typedef std::vector<Sprite> List_Sprite;

struct Map{
    const char *name; // path!!!
    EntityBaseType tiles[TILEMAP_COLS][TILEMAP_ROWS];
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

inline internal Sprite make_pickup(fvec2 position, u64 type) {
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
            eBlueFlame,
            {0, 0}
        }
    };
    
    result.entity.params[0].as_f64 = +1.0;
    result.entity.params[1].as_f64 = -FLT_MAX;
    
    return result;
}

internal char *parse_double(char *c, double *d) {
    char *end;
    *d = strtod(c, &end);
    return end;
}

internal char *parse_long(char *c, long *d) {
    char *end;
    *d = strtol(c, &end, 10);
    return end;
}

internal char *eEnemy_parse(char *c, EnemyType *type) {
    *type = EnemyType::Goblin;
    
    if (*c == ',') {
        c++;
        long tl;
        c = parse_long(c, &tl);
        assert(tl < long(EnemyType::Count));
        
        *type = (EnemyType)tl;
    }
    
    return c;
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

internal char *Goblin_custom(Sprite *goblin, char *c) {
    c = parse_custom_double(goblin, c, 1, 80.f);
    c = parse_custom_bool32(&goblin->mirror.x, c, false);
    
    return c;
}

internal char *Ghost_custom(Sprite *ghost, char *c) {
    c = parse_custom_double(ghost, c, 1, 200.f);
    c = parse_custom_bool32(&ghost->mirror.x, c, false);
    return c;
}

internal char* eBlueFlame_parse(Sprite *flame, char *c) {
    if (*c == ',') {
        c++;
        double dur;
        c = parse_double(c, &dur);
        
        flame->entity.params[0].as_f64 = dur;
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
            
            if (pickup_is_bell((PickupType)type)) {
                pickup->tilemap = &GET_TILEMAP_TEXTURE(TM_essentials);
            }
        }
        
    }
    return c;
}

internal void
draw(Sprite const * sprite)
{
    assert(sprite->tilemap);
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
    
    
    if (pickup_type_is_non_effect(type)) {
        index_to_draw = sprite->entity.params[0].as_u64;
    } else if (type == PickupType::Bell) {
        index_to_draw = sprite->entity.params[0].as_u64 + (4 * 5 + 3);
    }
    
    gl_slow_tilemap_draw(sprite->tilemap,
                         {(float)sprite->position.x, (float)sprite->position.y},
                         {(float)sprite->size.x, (float)sprite->size.y},
                         sprite->rotation,
                         index_to_draw,
                         sprite->mirror.x, sprite->mirror.y);
}