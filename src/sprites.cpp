
struct Sprite
{
    GLTilemapTexture const * tilemap = 0;
    ivec2 size = {64, 64};
    ivec2 position = {0,0};
    float rotation = 0.f;
    AABox collision_box = {0,0,64,64};
    ivec2 mirror = {false, false};
    ivec2 velocity = {0,0};
    b32 is_on_air = false;
    
    b32 animation_playing = false;
    i32 current_frame = 0;
    i32 current_animation = -1;
    float time_accumulator = 0.f;
    Animation* animation_set;
    
    Entity entity;
    
    inline AABox get_transformed_AABox() const
    {
        return this->collision_box.translate(this->position);
    }
    
    void update_animation(float dt)
    {
        Animation* anim_ref;
        
        anim_ref = &this->animation_set[this->current_animation];
        if (anim_ref->size == 0 || !this->animation_playing)
            return;
        
        this->time_accumulator += dt;
        if (this->time_accumulator >= anim_ref->duration)
        {
            this->current_frame++;
            if (this->current_frame >= anim_ref->size )
            {
                
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
        if (this->current_animation != anim_idx)
        {
            this->animation_playing = true;
            this->current_animation = anim_idx;
            this->current_frame = 0;
            this->time_accumulator = 0;
        }
        
    }
};

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
    ivec2 ipos = {(i32)player->position.x + 32, (i32)player->position.y + 32};
    ivec2 player_tile = iclamp({0,0}, {14,11}, map_position_to_tile(ipos));
    
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
    
}

// ePlayer
internal void ePlayer_update(Sprite* player, InputState* _istate, float dt)
{
    const i32 GRAVITY = 900;
    const i32 MAX_YSPEED = 450;
    const i32 JUMP_STRENGTH = 450;
    const i32 XSPEED = 200;
    
    if (GET_KEYDOWN(move_right))
        player->velocity.x =  XSPEED;
    else if (GET_KEYDOWN(move_left))
        player->velocity.x = -XSPEED;
    else 
        player->velocity.x =  0;
    
    if (GET_KEYPRESS(move_up) && !player->is_on_air)
    {
        player->velocity.y = -JUMP_STRENGTH;
        player->is_on_air = true;
        audio_play_sound(&player_jump_sound);
    }
    
    player->velocity.y += GRAVITY * dt;
    player->velocity.y = iclamp(-JUMP_STRENGTH, MAX_YSPEED, player->velocity.y);
    
    // Casting basics
    if (GET_KEYPRESS(cast))
    {
        if (player->current_animation != GET_CHAR_ANIMENUM(test_player, Cast))
        {
            // save yspeed
            player_last_yspeed = player->velocity.y;
            // cast!
            player->velocity.x = 0;
            player->velocity.y = 0;
            puts("cast!");
            SET_ANIMATION(player, test_player, Cast);
            ePlayer_cast(player, dt);
        }
    }
    
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Cast))
    {
        if (player->animation_playing)
            return;
        player->velocity.y = player_last_yspeed;
    }
    
    // NOTE(miked): This should teach me not to mix integers
    // and floats ever again:
#if 1
    // This would cause less movement in Y. pos.x + vel.y * dt
    // will result in a float, which will then truncate to an int.
    player->position.x += (i32)(player->velocity.x * dt);
    player->position.y += (i32)(player->velocity.y * dt);
#else
    // But this implied calculation would be int i = vel.x * dt; pos.x += i
    //etc... So it results in more movement in the y direction.
    player->position += player->velocity * dt;
#endif
    
    // Get the upper left tile based on the center of the player sprite
    ivec2 ipos = {(i32)player->position.x + 32, (i32)player->position.y + 32};
    ivec2 start_tile = iclamp({0,0}, {14,11},
                              map_position_to_tile(ipos) - 1);
    
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
            AABox player_trans = player->get_transformed_AABox();
            
            ivec2 diff;
            b32 collided = aabb_minkowski(&player_trans, &collision, &diff);
            if (collided)
            {
                player->position = player->position - (diff);
                
                // If we are moving up and diff moved us in the Y dir,
                // then negate the collision.
                // (fixes bouncing when hitting corner of a tile)
                if (player->velocity.y < 0 && iabs(diff.y) < 5)
                {
                    player->position.y += diff.y;
                    continue;
                }
                
                if (j == 2)
                {
                    collided_on_bottom = true;
                    if (iabs(diff.y) > 0)
                    {
                        player->is_on_air = false;
                        player->velocity.y = 0;
                    }
                }
                
                // Bouncing
                if (player->velocity.y < 0 &&
                    player->is_on_air &&
                    player_trans.min_x < collision.max_x && 
                    diff.x >= 0)
                {
                    player->velocity.y = -player->velocity.y * 0.737;
                    player->is_on_air = true;
                    
                    // it has to be mostly above the player, in order to avoid
                    // destroying blocks diagonally
                    if ( i != 1 || j != 0) continue;
                    
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
    
    if (!collided_on_bottom && player->velocity.y != 0)
        player->is_on_air = true;
    
    // Draw the Bounding box sprite
    AABox box = player->get_transformed_AABox();
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {box.min_x, box.min_y},
        {box.max_x - box.min_x, box.max_y - box.min_y},
        0,5 * 5 + 1 );
    
    // Draw the "air indicator"
    if (!player->is_on_air)
    {
        gl_slow_tilemap_draw(
            &GET_TILEMAP_TEXTURE(test),
            {0, 0},
            {30, 30},
            0,5 * 2 + 4 );
    }
    
    ////////////////////////////////
    // Animation Logic
    if (iabs(player->velocity.x) > 0)
    {
        SET_ANIMATION(player, test_player, Run);
        
        player->mirror.x = player->velocity.x > 0;
    }
    else
    {
        SET_ANIMATION(player, test_player, Idle);
    }
    
}
