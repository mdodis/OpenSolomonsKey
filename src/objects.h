#define MAX_ENTITY_PARAMS 2
// Custom Parameters
// eEmptySpace :: hidden item
// eBlockFrail :: health, hidden item

union CustomParameter
{
    i64    as_i64;
    double as_f64;
};

/*
 NOTE: eEffect - scene_update _automatically_ marks
 any eEffect entity for removal once it's animation is finished
 
 NOTE/TODO: Mixing scene entities with entities you can
 specify in the level format isn't a good idea.
*/
enum EntityBaseType
{
    eEmptySpace,
    eBlockFrail,
    eBlockSolid,
    ePlayerSpawnPoint,
    
    
    ePlayer,
    eGoblin,
    eGhost,
    eEffect,
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
    fvec2 size = {64, 64};
    fvec2 position = {0,0};
    float rotation = 0.f;
    AABox collision_box = {0,0,64,64};
    fvec2 mirror = {false, false};
    fvec2 velocity = {0,0};
    b32 is_on_air = false;
    b32 mark_for_removal = false;                 // set to true to remove the sprite
    
    // Animation stuff
    b32 animation_playing = true;
    i32 current_frame = 0;
    i32 current_animation = 0;
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
    
    
    void update_animation(float dt)
    {
        // get animation ref
        Animation* anim_ref;
        anim_ref = &this->animation_set[this->current_animation];
        if (anim_ref->size == 0 || !this->animation_playing)
            return;
        
        // increase time by dt
        this->time_accumulator += dt;
        if (this->time_accumulator >= anim_ref->duration)
        {
            this->current_frame++;
            if (this->current_frame >= anim_ref->size )
            {
                // if its looping
                if (anim_ref->loop)
                    this->current_frame = 0;
                else
                {
                    this->animation_playing = false;
                    this->current_frame--;
                }
            }
            
            this->time_accumulator = 0.f;
        }
        
    }
    
#define SET_ANIMATION(spr, c, n) spr->set_animation_index(GET_CHAR_ANIMENUM(c, n))
    void set_animation_index(u32 anim_idx)
    {
        fail_unless(this->current_animation >= 0, "set_animation_index, invalid animation index!");
        
        if (this->current_animation != anim_idx)
        {
            this->animation_playing = true;
            this->current_animation = anim_idx;
            this->current_frame = 0;
            this->time_accumulator = 0;
        }
        
    }
    
    void move_and_collide(
        float dt,
        const float GRAVITY,
        const float MAX_YSPEED,
        const float JUMP_STRENGTH,
        float XSPEED,
        b32 damage_tiles = false);
    
    void collide_sprite(float dt);
    
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

inline internal Sprite make_goblin(fvec2 position)
{
    return Sprite
    {
        .tilemap = &GET_CHAR_TILEMAP(Goblin),
        .position = position,
        .collision_box = {5, 0, 45, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Goblin),
        .entity =
        {
            .type = eGoblin,
        }
    };
}

inline internal Sprite make_ghost(fvec2 position)
{
    return Sprite
    {
        .tilemap = &GET_CHAR_TILEMAP(Ghost),
        .position = position,
        .collision_box = {0, 0, 64, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Ghost),
        .entity =
        {
            .type = eGhost,
        }
    };
}

inline internal Sprite make_player(fvec2 position)
{
    const i32 player_height = 50;
    return Sprite
    {
        .tilemap = &GET_CHAR_TILEMAP(test_player),
        .size = fvec2{64, player_height},
        .position = position,
        
        .collision_box = {5, 0, 45, player_height},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(test_player),
        
        .entity =
        {
            ePlayer,
            {0,0}
        }
    };
}

inline internal Sprite make_effect(fvec2 position)
{
    return Sprite
    {
        .tilemap = &GET_CHAR_TILEMAP(Effect),
        .position = position,
        .animation_set = GET_CHAR_ANIMSET(Effect),
        .entity = 
        {
            eEffect,
            {0,0}
        }
    };
}

internal const char* goblin_parse_custom(Sprite* goblin, const char* c)
{
    if (*c == ',')
    {
        c++;
        
        // parse initial direction
        if (*c == 'R')
            goblin->mirror.x = true;
        else
            goblin->mirror.x = false;
        
    }
    
    return c;
}


internal void
draw(Sprite const * sprite)
{
    assert(sprite->tilemap);
    Animation* anim_ref = &sprite->animation_set[sprite->current_animation];
    i32 frame_to_render = anim_ref->start.y * sprite->tilemap->cols
        + anim_ref->start.x + sprite->current_frame;
    
    gl_slow_tilemap_draw(
        sprite->tilemap,
        {(float)sprite->position.x, (float)sprite->position.y},
        {(float)sprite->size.x, (float)sprite->size.y},
        sprite->rotation,
        frame_to_render,
        sprite->mirror.x, sprite->mirror.y);
}
