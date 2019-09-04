
#define CA_TYPE Sprite
#include "calist.h"
#undef CA_TYPE

struct
{
    u64 tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    u64 hidden_tilemap[TILEMAP_COLS][TILEMAP_ROWS] = {};
    List_Sprite spritelist = {};
    
} g_scene;

internal void 
Sprite_update_animation(Sprite *const sprite, float dt)
{
    assert(sprite);
    
    Animation* anim_ref;
    
    anim_ref = &sprite->animation_set[sprite->current_animation];
    if (anim_ref->size == 0)
        return;
    
    sprite->time_accumulator += dt;
    
    if (sprite->time_accumulator >= anim_ref->duration)
    {
        if (sprite->current_frame < anim_ref->size)
            sprite->current_frame++;
        else if (sprite->current_frame >= anim_ref->size && anim_ref->loop)
            sprite->current_frame = 0;
        
        sprite->time_accumulator = 0.f;
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

internal void
Sprite_set_anim(Sprite *const sprite, u32 new_anim)
{
    sprite->time_accumulator = 0.f;
    sprite->current_frame = 0;
    sprite->current_animation = new_anim;
}


#define GRAVITY 900
#define MAX_YSPEED 450
#define JUMP_STRENGTH 350

RESSound test_sound;

// ePlayer
internal void ePlayer_update(Sprite* player, InputState* _istate, float dt)
{
    if (GET_KEYDOWN(move_right))
    {
        player->velocity.x = 250.f;
    }
    else if (GET_KEYDOWN(move_left))
    {
        player->velocity.x = -250.f;
    }
    else
        player->velocity.x = 0;
    
    if (GET_KEYPRESS(move_up) && !player->is_on_air)
    {
        player->velocity.y = -JUMP_STRENGTH;
        audio_play_sound(&test_sound);
        player->is_on_air = true;
    }
    
    ivec2 before = player->position;
    
    player->velocity.y += GRAVITY * dt;
    
    player->velocity.y = iclamp(-JUMP_STRENGTH, MAX_YSPEED, player->velocity.y);
    player->position.x += player->velocity.x * dt;
    player->position.y += player->velocity.y * dt;
    
    
    ivec2 ipos = {(i32)player->position.x + 32, (i32)player->position.y + 32};
    ivec2 player_tile = map_position_to_tile(ipos);
    
    
    //
    // Collision detection around 3x3 grid
    //
    ivec2 start_tile = player_tile - 1;
    start_tile = iclamp({0,0}, {14,11}, start_tile);
    b32 collided_on_bottom = false;
    for (i32 j = 0; j < 3; ++j)
    {
        for (i32 i = 0; i < 3; ++i)
        {
            if (i == 1 && j == 1) continue;
            
            if (g_scene.tilemap[start_tile.x + i][start_tile.y + j] == eEmptySpace) continue;
            
            ivec2 tile_coords =
            {
                (start_tile.x + i) * 64,
                (start_tile.y + j) * 64
            };
            
            AABox collision = {0,0,64,64};
            collision = collision.translate(tile_coords);
            AABox player_trans = player->get_transformed_AABox();
            
            ivec2 diff;
            b32 collided = aabb_minkowski(&player_trans, &collision, &diff);
            if (collided)
            {
                player->position = player->position - (diff);
                
                if (player->velocity.y < 0 &&
                    iabs(diff.y) < 5)
                {
                    player->position.y += diff.y;
                    continue;
                }
                
                if (j == 2) collided_on_bottom = true;
                
                if (j == 2 &&
                    iabs(diff.y) > 0)
                {
                    player->is_on_air = false;
                    player->velocity.y = 0;
                    //puts("GRND");
                }
                
                if (player->velocity.y < 0 && 
                    player->is_on_air &&
                    (player_trans.min_x < collision.max_x && 
                     diff.x >= 0))
                {
                    player->velocity.y = -player->velocity.y;
                    printf("%d %d %d %d %d\n",
                           player_trans.min_x,
                           collision.max_x,
                           collision.max_y,
                           player_trans.min_y, diff.y);
                    
                    player->is_on_air = true;
                }
                
                if (i == 0 && j == 2 &&
                    iabs(diff.x) > 0)
                {
                    player->position.x += diff.x;
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
    {
        player->is_on_air = true;
    }
    
    AABox box = player->get_transformed_AABox();
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {box.min_x, box.min_y},
        {box.max_x - box.min_x, box.max_y - box.min_y},
        0,5 * 5 + 1 );
    
    if (!player->is_on_air)
    {
        gl_slow_tilemap_draw(
            &GET_TILEMAP_TEXTURE(test),
            {0, 0},
            {30, 30},
            0,5 * 2 + 4 );
    }
    
    
}
