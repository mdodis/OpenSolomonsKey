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
    
} g_scene;

internal void 
Sprite_update_animation(Sprite *const sprite, float dt)
{
    assert(sprite);
    
    Animation* anim_ref;
    
    anim_ref = &sprite->animation_set[sprite->current_animation];
    if (anim_ref->size == 0 || !sprite->animation_playing)
        return;
    
    sprite->time_accumulator += dt;
    if (sprite->time_accumulator >= anim_ref->duration)
    {
        sprite->current_frame++;
        if (sprite->current_frame >= anim_ref->size )
        {
            
            if (anim_ref->loop)
                sprite->current_frame = 0;
            else
            {
                sprite->animation_playing = false;
                sprite->current_frame--;
            }
        }
        
        sprite->time_accumulator = 0.f;
    }
    
}

#define Sprite_change_anim(spr, c, n)  _Sprite_change_anim(spr, GET_CHAR_ANIMENUM(c,n)) 
internal void _Sprite_change_anim(Sprite* const spr, u32 anim_idx)
{
    if (spr->current_animation != anim_idx)
    {
        spr->animation_playing = true;
        spr->current_animation = anim_idx;
        spr->current_frame = 0;
        spr->time_accumulator = 0;
    }
    
}

internal void
Sprite_draw(Sprite const * sprite, i32 frame)
{
    assert(sprite->tilemap);
    
    gl_slow_tilemap_draw(
        sprite->tilemap,
        {(float)sprite->position.x, (float)sprite->position.y},
        {(float)sprite->size.x, (float)sprite->size.y},
        sprite->rotation,
        frame,
        sprite->mirror.x, sprite->mirror.y);
    
}

internal void
Sprite_draw_anim(Sprite const * sprite)
{
    assert(sprite->tilemap);
    Animation* anim_ref = &sprite->animation_set[sprite->current_animation];
    i32 frame_to_render = anim_ref->start.y * sprite->tilemap->cols
        + anim_ref->start.x + sprite->current_frame;
    //Sprite_draw(sprite, frame_to_render);
    
    gl_slow_tilemap_draw(
        sprite->tilemap,
        {(float)sprite->position.x, (float)sprite->position.y},
        {(float)sprite->size.x, (float)sprite->size.y},
        sprite->rotation,
        frame_to_render,
        sprite->mirror.x, sprite->mirror.y);
}

RESSound test_sound;

// ePlayer
internal void ePlayer_update(Sprite* player, InputState* _istate, float dt)
{
    const i32 GRAVITY = 900;
    const i32 MAX_YSPEED = 450;
    const i32 JUMP_STRENGTH = 400;
    
    if (GET_KEYDOWN(move_right))
        player->velocity.x =  250.f;
    else if (GET_KEYDOWN(move_left))
        player->velocity.x = -250.f;
    else 
        player->velocity.x =  0;
    
    if (GET_KEYPRESS(move_up) && !player->is_on_air)
    {
        player->velocity.y = -JUMP_STRENGTH;
        player->is_on_air = true;
        audio_play_sound(&test_sound);
    }
    
    player->velocity.y += GRAVITY * dt;
    player->velocity.y = iclamp(-JUMP_STRENGTH, MAX_YSPEED, player->velocity.y);
    
    // NOTE(miked): This should teach me not to mix integers
    // and floats ever again:
#if 1
    // This would cause less movement in Y. pos.x + vel.y * dt
    // will result in a float, which will then truncate to an int.
    player->position.x += player->velocity.x * dt;
    player->position.y += player->velocity.y * dt;
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
                    player->velocity.y = -player->velocity.y;
                    player->is_on_air = true;
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
        Sprite_change_anim(player, test_player, Run);
        
        player->mirror.x = player->velocity.x > 0;
    }
    else
    {
        Sprite_change_anim(player, test_player, Idle);
    }
    
}
