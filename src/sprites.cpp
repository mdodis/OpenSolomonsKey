void Sprite::move_and_collide(
float dt,
const float GRAVITY,
const float MAX_YSPEED,
const float JUMP_STRENGTH,
float XSPEED,
b32 damage_tiles)
{
    this->velocity.x = XSPEED;
    
    this->velocity.y += GRAVITY * dt;
    this->velocity.y = fclamp(-JUMP_STRENGTH, MAX_YSPEED, this->velocity.y);
    
    // NOTE(miked): This should teach me not to mix integers
    // and floats ever again:
#if 0
    // This would cause less movement in Y. pos.x + vel.y * dt
    // will result in a float, which will then truncate to an int.
    this->position.x += (this->velocity.x * dt);
    this->position.y += (this->velocity.y * dt);
#else
    // But this implied calculation would be int i = vel.x * dt; pos.x += i
    //etc... So it results in more movement in the y direction.
    this->position += this->velocity * dt;
#endif
    
    // Get the upper left tile based on the center of the this sprite
    fvec2 ipos = {this->position.x, this->position.y};
    ivec2 start_tile = iclamp(ivec2{0,0}, ivec2{14,11},
                              map_position_to_tile_centered(ipos) - 1);
    
    b32 collided_on_bottom = false;
    
    for (i32 j = 0; j < 3; ++j)
    {
        for (i32 i = 0; i < 3; ++i)
        {
            if (g_scene.tilemap[start_tile.x + i][start_tile.y + j] == eEmptySpace) continue;
            
            fvec2 tile_coords =
            {
                (start_tile.x + i) * 64.f,
                (start_tile.y + j) * 64.f
            };
            
            AABox collision = {0, 0, 64, 64};
            collision = collision.translate(tile_coords);
            AABox this_trans = this->get_transformed_AABox();
            
            fvec2 diff;
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
                
                
                if (j == 2 || (start_tile.y <= 1 && j == 1))
                {
                    // NOTE(miked): j == 2 is the bottom tile in most cases,
                    // but if y == 1 in tile space, then j will be 1 in that case
                    
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
                    if ((EntityBaseType)scene_get_tile(current_tile) == eBlockFrail)
                    {
                        scene_set_tile(current_tile, eEmptySpace);
                    }
                    
                }
                
            }
            
            
#ifndef NDEBUG
            
#if 0            
            gl_slow_tilemap_draw(
                &GET_TILEMAP_TEXTURE(test),
                {tile_coords.x, tile_coords.y},
                {64, 64},
                0.f,
                5 * 5,
                false, false,
                NRGBA{1,1,1,.7f});
#endif
#endif
        }
    }
    
    
    if (!collided_on_bottom && this->velocity.y != 0)
        this->is_on_air = true;
    
    // NOTE(miked): collision checking for the bounds
    AABox bound_bottom = {0, 12 * 64, 64*15 , 12 * 64};
    AABox bound_right  = {15 * 64, 0, 16 * 64, 12 * 64};
    AABox bound_left   = {-64, 0, 0, 12 * 64};
    this->collide_aabb(&bound_bottom);
    this->collide_aabb(&bound_right);
    this->collide_aabb(&bound_left);
    
#ifndef NDEBUG
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {bound_bottom.min_x, bound_bottom.min_y},
        {bound_bottom.max_x, bound_bottom.max_y},
        0.f,
        5 * 5,
        false, false,
        NRGBA{0,1,1,.7f});
    
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {bound_right.min_x, bound_right.min_y},
        {bound_right.max_x, bound_right.max_y},
        0.f,
        5 * 5,
        false, false,
        NRGBA{0,1,1,.7f});
    
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {bound_left.min_x + 32, bound_left.min_y},
        {bound_left.max_x + 32, bound_left.max_y},
        0.f,
        5 * 5,
        false, false,
        NRGBA{0,1,1,.7f});
#endif
}


RESSound player_jump_sound;

internal void ePlayer_cast(Sprite* player, float dt)
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
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Crouch))
    {
        target_tile.y = iclamp(0, 14,target_tile.y + 1);
    }
    
    // TODO(miked): Secrets in tiles and empty space!
    EntityBaseType type = (EntityBaseType)scene_get_tile(target_tile);
    if (type == eBlockFrail)
    {
        scene_set_tile(target_tile, eEmptySpace);
    }
    else if (type == eEmptySpace)
    {
        scene_set_tile(target_tile, eBlockFrail);
    }
    
    SET_ANIMATION(player, test_player, Cast);
    
}


internal void eDFireball_update(Sprite* dfire, InputState* _istate, float dt)
{
    
#define going_right (dfire->mirror.x)
    
    const float proximity_thresh = 2.f;
    ivec2 target_tile = map_position_to_tile_centered(dfire->position);
    AABox aabb = dfire->get_transformed_AABox();
    
    dfire->rotation = (int)dfire->rotation % 360;
    
    fvec2 forward = direction_from_rotation(D2R * dfire->rotation);
    fvec2 back = direction_from_rotation(D2R * (dfire->rotation + 180.f));
    fvec2 right = direction_from_rotation(D2R * (dfire->rotation + 90.f));
    fvec2 left = direction_from_rotation(D2R * (dfire->rotation - 90.f));
    
    ivec2 forward_tile = target_tile + ivec2{(int)forward.x, (int)forward.y};
    ivec2 right_tile = target_tile + ivec2{(int)right.x, (int)right.y};
    ivec2 left_tile = target_tile + ivec2{(int)left.x, (int)left.y};
    
    
    /* 
    RIGHT CASES:                                 LEFT CASES:
      ROT 0       ROT 180    ROT 90     ROT 270     ROT 180    ROT 0      ROT 90     ROT 270
     ====1-2========3-4=========5-6========7-8===||===1-2========3-4=========5-6========7-8====
     | Go down  | Go right | Go right | Go right || Go down  | Go left  | Go left  | Go left  |
     |          |   [XX]   |[XX]o     |          ||          |   [XX]   |     o[XX]|          |
     | o>       |   [XX]   |[XX]▼     |          ||       <o |   [XX]   |     ▼[XX]|          |
     |[XX]      |    <o    |   [XX]   |   ▲[XX]  ||      [XX]|    o>    |   [XX]   |[XX]▲     |
     |[XX]      |          |   [XX]   |   o[XX]  ||      [XX]|          |   [XX]   |[XX]o     |
     |----------|----------|----------|----------||----------|----------|----------|----------|
     | Go up    | Go left  | Go left  | Go left  || Go up    | Go right | Go right | Go right |
     |    [XX]  |   [XX]   |[XX]o     | [XX]     ||  [XX]    |   [XX]   |     o[XX]|    [XX]  |
     | o> [XX]  |   [XX]   |[XX]▼     | [XX]     ||  [XX] <o |   [XX]   |     ▼[XX]|    [XX]  |
     |[XX]      |[XX] <o   |          |   ▲[XX]  ||      [XX]|    o>[XX]|          |[XX]▲     |
     |[XX]      |[XX]      |          |   o[XX]  ||      [XX]|      [XX]|          |[XX]o     |
     ============================================||===========================================|
    */
    float last_rot = dfire->rotation;
    
    if (going_right){                                           // RIGHT CASES
        
        if (dfire->rotation == 0) {                             // CASES 1-2
            bool guard1 = false;
            
            if (/*scene_get_tile(right_tile + ivec2{1,0}) != eEmptySpace &&*/
                (right_tile.x + 1) * 64.f - aabb.max_x < proximity_thresh)
                guard1 = true;
            
            if (/*scene_get_tile(target_tile) == eEmptySpace &&*/
                scene_get_tile(right_tile)  == eEmptySpace &&
                (right_tile.x + 1) * 64.f - aabb.max_x >= proximity_thresh &&
                fabs(right_tile.y * 64.f - aabb.max_y) <= proximity_thresh &&
                (right_tile.x * 64.f - aabb.min_x + proximity_thresh) < 0)
                dfire->rotation = 90 ; // CASE 1
            
            if (scene_get_tile(forward_tile) != eEmptySpace &&
                (forward_tile.x * 64.f - aabb.max_x + proximity_thresh) < 0)
                dfire->rotation = 270; // CASE 2
            
        }
        else if (dfire->rotation == 180){                       // CASES 3-4
            
            bool guard3 = false;
            if (/*scene_get_tile(right_tile + ivec2{-1, 0}) != eEmptySpace &&*/
                (aabb.max_x - right_tile.x * 64.f - 64.f) < proximity_thresh)
                guard3 = true;
            
            if (/*scene_get_tile(target_tile) == eEmptySpace &&*/
                scene_get_tile(right_tile)  == eEmptySpace &&
                !guard3 &&
                ((right_tile.x - 1) * 64.f - aabb.min_x) < 0)
                dfire->rotation = 270; // CASE 3
            
            if (scene_get_tile(forward_tile) != eEmptySpace &&
                (aabb.min_x - (forward_tile.x + 1) * 64.f) < 0)
                dfire->rotation = 90 ; // CASE 4
        }
        else if (dfire->rotation == 90){                        // CASES 5-6
            if (scene_get_tile(forward_tile) != eEmptySpace &&
                ((right_tile.y + 1) * 64.f - aabb.max_y) < proximity_thresh)
                dfire->rotation = 0  ; // CASE 5
            
            bool guard6 = false;
            if (/*scene_get_tile(right_tile + ivec2{0, 1}) != eEmptySpace &&*/
                (right_tile.y + 1) * 64.f - aabb.max_y < proximity_thresh)
                guard6 = true;
            
            if (scene_get_tile(target_tile) == eEmptySpace &&
                scene_get_tile(right_tile)  == eEmptySpace &&
                !guard6&&
                ((right_tile.y) * 64.f - aabb.min_y) < 0)
                dfire->rotation = 180; // CASE 6
        }
        else if (dfire->rotation == 270){                       // CASES 7-8
            
            bool guard7 = false;
            
            if (/*scene_get_tile(right_tile + ivec2{0,-1}) != eEmptySpace &&*/
                aabb.min_y - (right_tile.y) * 64.f < proximity_thresh)
                guard7 = true;
            
            if (scene_get_tile(target_tile) == eEmptySpace &&
                scene_get_tile(right_tile)  == eEmptySpace &&
                !guard7 &&
                aabb.max_y - (right_tile.y + 1) * 64.f < 0)
                dfire->rotation = 0  ; // CASE 7
            
            if (scene_get_tile(forward_tile) != eEmptySpace &&
                (aabb.min_y - right_tile.y * 64.f) < 0)
                dfire->rotation = 180; // CASE 8
            
        }
    }
    
    if (last_rot != dfire->rotation) printf("%3.0f -> %3.0f\n", last_rot, dfire->rotation);
    
    if (GET_KEYPRESS(sound_up)) dfire->rotation += 90.f;
    if (GET_KEYPRESS(sound_down)) dfire->rotation -= 90.f;
    
    dfire->rotation = (int)dfire->rotation % 360;
    draw_num(dfire->rotation, 3);
    
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {forward_tile.x * 64.f, forward_tile.y * 64.f},
        {64.f, 64.f},
        0.f, 6, false, false, 
        NRGBA{0.f,1.f,0.f,1.f});
    
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {right_tile.x * 64.f, right_tile.y * 64.f},
        {64.f, 64.f},
        0.f, 6, false, false, NRGBA{1.f,1.f,1.f,1.f});
    
    gl_slow_tilemap_draw(
        &GET_TILEMAP_TEXTURE(test),
        {left_tile.x * 64.f, left_tile.y * 64.f},
        {64.f, 64.f},
        0.f, 6, false, false, NRGBA{1.f,0.f,1.f,1.f});
    
    float output_rotation = dfire->rotation;
    output_rotation += going_right ? 0 : 180;
    output_rotation = (int)output_rotation % 360;
    
    dfire->position += direction_from_rotation(D2R * (output_rotation)) * 100.f * dt;
    
}


internal void ePlayer_update(Sprite* player, InputState* _istate, float dt)
{
    /* 
    TODO: dana cast fireball, tread on edge of blocks until decay
    
    */
    
    const float GRAVITY = 950;
    const float MAX_YSPEED = 500;
    const float JUMP_STRENGTH = 375;
    const float RUNNING_JUMP_STRENGTH = 350;
    const float XSPEED = 150;
    
    float xmove_amount = 0;
    b32 is_crouching = false;
    b32 did_jump     = false;
    // NOTE(miked): maybe not have it be a local-global?
    persist i32 player_last_yspeed;
    
    if      (GET_KEYDOWN(move_right)) xmove_amount = XSPEED;
    else if (GET_KEYDOWN(move_left )) xmove_amount = -XSPEED;
    
    if (GET_KEYDOWN(move_down) && !player->is_on_air)
        is_crouching = true;
    
    if (GET_KEYPRESS(move_up) && !is_crouching)  
        did_jump = player->jump(JUMP_STRENGTH);
    
    if (did_jump)  audio_play_sound(&player_jump_sound);
    
    // Casting 
    if (GET_KEYPRESS(cast) &&
        player->current_animation != GET_CHAR_ANIMENUM(test_player, Cast))
    {
        // save yspeed
        player_last_yspeed = player->velocity.y;
        // cast!
        player->velocity.x = 0;
        player->velocity.y = 0;
        ePlayer_cast(player, dt);
    }
    
    // Fireball Casting
    if (GET_KEYPRESS(fireball) &&
        player->current_animation != GET_CHAR_ANIMENUM(test_player, Cast))
    {
        Sprite f = make_dfireball(player->position + fvec2{16, 10});
        // TODO(miked): FLIP SPRITE image
        f.mirror.x = player->mirror.x;
        //f.rotation = player->mirror.x ? 0.f  : 180.f;
        Sprite *p = scene_sprite_add(&f);
        
    }
    
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Cast))
    {
        if (player->animation_playing)
            return;
        else
            player->velocity.y = player_last_yspeed;
    }
    
    if (is_crouching) xmove_amount = 0;
    
    // NOTE(miked): A running jump results in a 1-block
    // height-displacement rather than a 2 block displacement
    const i32 jump_strength =
        (player->velocity.x != 0) ? RUNNING_JUMP_STRENGTH : JUMP_STRENGTH;
    
    player->move_and_collide(
        dt, 
        GRAVITY, 
        MAX_YSPEED,
        jump_strength, 
        xmove_amount,
        true);
    
    if (iabs(player->velocity.x) > 0)
    {
        SET_ANIMATION(player, test_player, Run);
        player->mirror.x = player->velocity.x > 0;
    }
    else if (is_crouching) SET_ANIMATION(player, test_player, Crouch);
    else                   SET_ANIMATION(player, test_player, Idle);
    
}


internal void eGoblin_update(Sprite* goblin, InputState* _istate, float dt)
{
    /*  
    TODO: Goblin can live from a fall if it's falling and a block appears
    near it; SEE: https:youtu.be/jNi6DQEX3xQ?t=12
    
    TODO: Goblin can only fall _IF_ its currently in the walking,
    chasing, or waiting state. If it were in a punch state, it would have
    to finish that first, and then proceed to die by gravity
    
    NOTE: Sprite::mirror is a bool, and sprites by default look to the left,
    so invert direction vector to get the axis-compliant direction in X.
    */
    
#define direction ((float)(goblin->mirror.x ? 1 : -1))
    
    const float goblin_walk_speed = 80;
    const float goblin_run_speed = 120;
    b32 ignore_player = false;
    b32 is_dying = goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Fall);
    
    
    ignore_player = is_dying;
    
    if (goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Punch) ||
        goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Wait))
    {
        // Switch direction and continue when either of these states ends
        if (!goblin->animation_playing)
        {
            SET_ANIMATION(goblin, Goblin, Walk);
            goblin->mirror.x = !goblin->mirror.x;
        }
        else
        {
            ignore_player = true;
        }
    }
    
    // Chase
    {
        const Sprite* const player = scene_get_first_sprite(ePlayer);
        fail_unless(player, "Player sprite not found scene_get_first_sprite");
        
        fvec2 ppos = player->position;
        ivec2 ppos_tile = map_position_to_tile_centered(ppos);
        
        ivec2 goblin_tile = map_position_to_tile_centered(goblin->position);
        
        if (goblin_tile.y == ppos_tile.y &&
            !ignore_player)
        {
            // search in the direction of the goblin to see if there is
            // an obstacle blocking its view of the player
            i32 tdiff = sgn(goblin_tile.x - ppos_tile.x);
            
            ivec2 block_tile = scene_get_first_nonempty_tile(goblin_tile, ppos_tile);
            if (block_tile == ivec2{-1, -1})
            {
                persist i32 blocking_tiles = 0;
                
                if (tdiff == -direction)
                {
                    SET_ANIMATION(goblin, Goblin, Chase);
                }
            }
            else
            {
#ifndef NDEBUG
                gl_slow_tilemap_draw(
                    &GET_TILEMAP_TEXTURE(test),
                    {block_tile.x * 64, block_tile.y * 64},
                    {64, 64},
                    0,5 * 5 + 0,
                    false, false,
                    NRGBA{0.f, 1.f, 1.f, 1.f});
#endif
            }
            
        }
    }
    
    // Stop at tile edges
    if (!is_dying)
    {    
        const i32 block_stop_offset = 32;
        const ivec2 goblin_tile = map_position_to_tile_centered(goblin->position);
        ivec2 dir_tile = map_position_to_tile_centered(goblin->position + fvec2{direction * block_stop_offset, 0});
        ivec2 dir_tile_under = dir_tile + ivec2{0, 1};
        
#ifndef NDEBUG
        gl_slow_tilemap_draw(
            &GET_TILEMAP_TEXTURE(test),
            {dir_tile_under.x * 64, dir_tile_under.y * 64},
            {64, 64},
            0,5 * 5 + 1,
            false, false,
            NRGBA{1.f, 0.f, 0.f, 1.f});
#endif
        if (scene_get_tile(dir_tile_under) == eEmptySpace && 
            scene_get_tile(dir_tile) == eEmptySpace &&
            goblin_tile.y != 11)
        {
            SET_ANIMATION(goblin, Goblin, Wait);
            goblin->velocity.x = 0;
        }
    }
    
    // Punching
    {
        const float punch_offset_amount = 32;
        const float punch_offset = direction * (punch_offset_amount);
        const ivec2 goblin_tile = map_position_to_tile(goblin->position);
        const b32 is_at_edge_of_map = 
            (goblin_tile.x == 14 && direction == 1) ||
            (goblin->position.x <= 2 && direction == -1);
        
        
        ivec2 tile_index = map_position_to_tile_centered(goblin->position + fvec2{punch_offset, 0 });
#ifndef NDEBUG
        gl_slow_tilemap_draw(
            &GET_TILEMAP_TEXTURE(test),
            {tile_index.x * 64, tile_index.y * 64},
            {64, 64},
            0,5 * 5 + 1,
            false, false,
            NRGBA{0.f, 1.f, 0.f, 1.f});
#endif
        if ((scene_get_tile(tile_index) != eEmptySpace || is_at_edge_of_map) &&
            !(goblin->is_on_air))
        {
            SET_ANIMATION(goblin, Goblin, Punch);
        }
    }
    
    // Movement
    i32 move_amount;
    switch(goblin->current_animation)
    {
        case GET_CHAR_ANIMENUM(Goblin, Walk):
        move_amount = goblin_walk_speed;
        break;
        
        case GET_CHAR_ANIMENUM(Goblin, Chase):
        move_amount = goblin_run_speed;
        break;
        
        default:
        move_amount = 0;
        break;
        
    }
    
    if (!is_dying)
        goblin->move_and_collide(dt, 900, 450, 450, move_amount * direction);
    else
    {
        goblin->velocity.y += 900 * dt;
        goblin->velocity.y = fclamp(-450, 450, goblin->velocity.y);
        
        goblin->position.y += goblin->velocity.y * dt;
    }
    
    if (iabs(goblin->velocity.x) > 0) goblin->mirror.x = goblin->velocity.x > 0;
    
    if (goblin->is_on_air && !is_dying)
    {
        SET_ANIMATION(goblin, Goblin, Fall);
    }
    else if (is_dying)
    {
        if (goblin->position.y > (12 * 64))
        {
            goblin->mark_for_removal = true;
            inform("You killed a Goblin, ouchie!");
        }
    }
    
#undef direction
}
