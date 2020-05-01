#define MAX_ENTITY_PARAMS 2
// Custom Parameters
// eEmptySpace :: hidden item
// eBlockFrail :: health, hidden item

union CustomParameter {
    u64    as_u64;
    double as_f64;
};

/* NOTE: eEffect - scene_update _automatically_ marks
 * any eEffect entity for removal once it's animation is finished
*/

enum EntityBaseType {
    eEmptySpace = 0,
    eBlockFrail = 1,
    eBlockFrailHalf = 2,
    eBlockSolid = 3,
    ePlayerSpawnPoint = 4,
    
    ePlayer = 5,
    eGoblin = 6,
    eGhost = 7,
    eDoor = 8,
    eKey = 9,
    
    ePickup,
    eEffect,
    eDFireball,
    EntityBaseType_Count,
};

inline bool tile_is_empty(EntityBaseType type) {
    if (type == eDoor || type == eEmptySpace) return true;
    
    return false;
}

struct Entity
{
    EntityBaseType type;
    CustomParameter params[MAX_ENTITY_PARAMS];
};

struct Sprite
{
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
    return Sprite {
        .tilemap = &GET_CHAR_TILEMAP(Goblin),
        .position = position,
        .collision_box = {5, 0, 45, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Goblin),
        .entity = {
            .type = eGoblin,
        }
    };
}

inline internal Sprite make_ghost(fvec2 position) {
    return Sprite {
        .tilemap = &GET_CHAR_TILEMAP(Ghost),
        .position = position,
        .collision_box = {0, 0, 64, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Ghost),
        .entity = {
            .type = eGhost,
        }
    };
}

inline internal Sprite make_player(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(test_player),
        .size = fvec2{64, 50},
        .position = position,
        .collision_box = {5, 0, 45, 50},
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
        .tilemap = &GET_CHAR_TILEMAP(StarRing),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .current_animation = 0,
        .animation_set = GET_CHAR_ANIMSET(StarRing),
        .entity = {
            EntityBaseType_Count,
            {u64(0.f),u64(0.f)}
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
        .collision_box = {10,10,54,54},
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

internal char* goblin_parse_custom(Sprite* goblin, char* c) {
    if (*c == ',') {
        c++;
        // parse initial direction
        if (*c == 'R') {
            goblin->mirror.x = true;
        } else {
            goblin->mirror.x = false;
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
    
    
    gl_slow_tilemap_draw(
                         sprite->tilemap,
                         {(float)sprite->position.x, (float)sprite->position.y},
                         {(float)sprite->size.x, (float)sprite->size.y},
                         sprite->rotation,
                         frame_to_render,
                         sprite->mirror.x, sprite->mirror.y);
}
