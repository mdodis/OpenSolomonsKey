
// This is a simple array list I wrote for C99. I'll probably regret
// using it, but life is all about living on the edge, and that means
// having potential creeping bugs sneaking up you when you least want it.
#define CA_TYPE Sprite
#include "calist.h"
#undef CA_TYPE

struct
{
    
    u64 tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    // TODO(miked): Hidden items
    u64 hidden_tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    List_Sprite spritelist = {};
    
    u64 get_tile(ivec2 p)
    {
        return tilemap[p.x][p.y];
    }
    
    void set_tile(ivec2 p, EntityBaseType t)
    {
        tilemap[p.x][p.y] = t;
    }
    
} g_scene;

void Sprite::move_and_collide(
float dt,
const i32 GRAVITY,
const i32 MAX_YSPEED,
const i32 JUMP_STRENGTH,
i32 XSPEED,
b32 damage_tiles)
{
    this->velocity.x = XSPEED;
    
    this->velocity.y += GRAVITY * dt;
    this->velocity.y = iclamp(-JUMP_STRENGTH, MAX_YSPEED, this->velocity.y);
    
    // NOTE(miked): This should teach me not to mix integers
    // and floats ever again:
#if 1
    // This would cause less movement in Y. pos.x + vel.y * dt
    // will result in a float, which will then truncate to an int.
    this->position.x += (i32)(this->velocity.x * dt);
    this->position.y += (i32)(this->velocity.y * dt);
#else
    // But this implied calculation would be int i = vel.x * dt; pos.x += i
    //etc... So it results in more movement in the y direction.
    this->position += this->velocity * dt;
#endif
    
    // Get the upper left tile based on the center of the this sprite
    ivec2 ipos = {(i32)this->position.x, (i32)this->position.y};
    ivec2 start_tile = iclamp({0,0}, {14,11},
                              map_position_to_tile_centered(ipos) - 1);
    
    b32 collided_on_bottom = false;
    
    for (i32 j = 0; j < 3; ++j)
    {
        for (i32 i = 0; i < 3; ++i)
        {
            if (g_scene.tilemap[start_tile.x + i][start_tile.y + j] == eEmptySpace) continue;
            
            ivec2 tile_coords =
            {
                (start_tile.x + i) * 64,
                (start_tile.y + j) * 64
            };
            
            AABox collision = {0, 0, 64, 64};
            collision = collision.translate(tile_coords);
            AABox this_trans = this->get_transformed_AABox();
            
            ivec2 diff;
            b32 collided = aabb_minkowski(&this_trans, &collision, &diff);
            if (collided)
            {
                this->position = this->position - (diff);
                
                // If we are moving up and diff moved us in the Y dir,
                // then negate the collision.
                // (fixes bouncing when hitting corner of a tile)
                if (this->velocity.y < 0 && iabs(diff.y) < 5)
                {
                    this->position.y += diff.y;
                    continue;
                }
                
                if (j == 2)
                {
                    collided_on_bottom = true;
                    if (iabs(diff.y) > 0)
                    {
                        this->is_on_air = false;
                        this->velocity.y = 0;
                    }
                }
                
                // Bouncing
                if (this->velocity.y < 0 &&
                    this->is_on_air &&
                    this_trans.min_x < collision.max_x && 
                    diff.x >= 0)
                {
                    this->velocity.y = -this->velocity.y * 0.737;
                    this->is_on_air = true;
                    
                    // it has to be mostly above the this, in order to avoid
                    // destroying blocks diagonally
                    if ( i != 1 || j != 0 || !damage_tiles) continue;
                    
                    ivec2 current_tile = {start_tile.x + i,start_tile.y + j};
                    if ((EntityBaseType)g_scene.get_tile(current_tile) == eBlockFrail)
                    {
                        g_scene.set_tile(current_tile, eEmptySpace);
                    }
                    
                }
                
            }
            
            gl_slow_tilemap_draw(
                &GET_TILEMAP_TEXTURE(test),
                {tile_coords.x, tile_coords.y},
                {64, 64},
                0.f,
                5 * 5);
        }
    }
    
    if (!collided_on_bottom && this->velocity.y != 0)
        this->is_on_air = true;
    
    // Draw the Bounding box sprite
    AABox box = this->get_transformed_AABox();
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {box.min_x, box.min_y},
        {box.max_x - box.min_x, box.max_y - box.min_y},
        0,5 * 5 + 1 );
    
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

RESSound player_jump_sound;

global i32 player_last_yspeed;

internal void
ePlayer_cast(Sprite* player, float dt)
{
    // Get the upper left tile based on the center of the player sprite
    ivec2 player_tile = iclamp({0,0}, {14,11}, map_position_to_tile_centered(player->position));
    ivec2 target_tile = player_tile;
    
    if (player->mirror.x)
    {
        if (player_tile.x >= 14) return;
        target_tile.x += 1;
    }
    else
    {
        if (player_tile.x <= 0) return;
        target_tile.x -= 1;
    }
    
    // if we were crouching: special condition
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Crouch) && 
        g_scene.get_tile(target_tile) == eEmptySpace)
    {
        target_tile.y = iclamp(0, 14,target_tile.y + 1);
    }
    
    // TODO(miked): Secrets in tiles and empty space!
    EntityBaseType type = (EntityBaseType)g_scene.get_tile(target_tile);
    if (type == eBlockFrail)
    {
        g_scene.set_tile(target_tile, eEmptySpace);
    }
    else if (type == eEmptySpace)
    {
        g_scene.set_tile(target_tile, eBlockFrail);
    }
    
    SET_ANIMATION(player, test_player, Cast);
    
}

internal void ePlayer_update(Sprite* player, InputState* _istate, float dt)
{
    const i32 GRAVITY = 900;
    const i32 MAX_YSPEED = 450;
    const i32 JUMP_STRENGTH = 450;
    const i32 XSPEED = 200;
    
    // movement
    i32 vel = 0;
    if (GET_KEYDOWN(move_right)) vel = XSPEED;
    else if (GET_KEYDOWN(move_left)) vel = -XSPEED;
    
    b32 is_crouching = false;
    if (GET_KEYDOWN(move_down))
    {
        if (!player->is_on_air)
            is_crouching = true;
    }
    
    b32 did_jump = false;
    if (GET_KEYPRESS(move_up) && !is_crouching)
        did_jump = player->jump(JUMP_STRENGTH);
    
    if (did_jump)
    {
        audio_play_sound(&player_jump_sound);
    }
    
    b32 enable_move = !is_crouching;
    
    // Casting 
    if (GET_KEYPRESS(cast))
    {
        if (player->current_animation != GET_CHAR_ANIMENUM(test_player, Cast))
        {
            // save yspeed
            player_last_yspeed = player->velocity.y;
            // cast!
            player->velocity.x = 0;
            player->velocity.y = 0;
            ePlayer_cast(player, dt);
        }
    }
    
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Cast))
    {
        if (player->animation_playing)
            return;
        player->velocity.y = player_last_yspeed;
    }
    
    if (enable_move)
        player->move_and_collide(dt, GRAVITY, MAX_YSPEED, JUMP_STRENGTH, vel, true);
    
    
    if (iabs(player->velocity.x) > 0)
    {
        SET_ANIMATION(player, test_player, Run);
        
        player->mirror.x = player->velocity.x > 0;
    }
    else if (is_crouching)
    {
        SET_ANIMATION(player, test_player, Crouch);
    }
    else
    {
        SET_ANIMATION(player, test_player, Idle);
    }
    
}

internal void eGoblin_update(Sprite* goblin, InputState* _istate, float dt)
{
    persist b32 t_ = false;
    if (!t_)
    {
        t_ = true;
        SET_ANIMATION(goblin, Goblin, Walk);
    }
    
    goblin->move_and_collide(dt, 900, 450, 450, 100);
    
    
    if (iabs(goblin->velocity.x) > 0)
    {
        goblin->mirror.x = goblin->velocity.x > 0;
    }
    
}
