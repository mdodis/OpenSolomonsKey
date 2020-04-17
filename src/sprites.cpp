void Sprite::move_and_collide(float dt,
                              const float GRAVITY,
                              const float MAX_YSPEED,
                              const float JUMP_STRENGTH,
                              float XSPEED,
                              b32 damage_tiles) {
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
// TODO(miked): Test cases for different speeds
// TODO(miked): Make ball rotate correctly (now its all over the place)
{
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
=======================HORIZONTAL=========================
============ROT 0===========||==========ROT 180===========   And tile-wise there are two general situations:
|            |    [XX]      ||            |      [XX]    |     1) The tile in front is blocking the way.
|   o>  (+90)|  o>[XX] (-90)||(-90)  <o   |(+90) [XX]<o  |     2) The tile in front(target_tile) is empty.
|[XX] | [XX] |[XX]          ||        [XX]|          [XX]|   What is important in every one of these, is
|[XX]   [XX] |[XX]          ||        [XX]|          [XX]|   knowing which tile we're going to be "attached"
|------------|--------------||------------|--------------|   to (side_tile).
|[XX]        |[XX]          ||        [XX]|          [XX]|
|[XX]        |[XX]          ||        [XX]|          [XX]|
|  o>   (-90)|  o>[XX] (+90)||(+90)   <o  |(-90) [XX]<o  |
|            |    [XX]      ||            |      [XX]    |
|            |              ||            |              |
============================||============================

========================VERTICAL==========================
===========Rot 90===========||==========ROT 270===========
|[XX]o  (+90)|[XX]o    (-90)||       (-90)|    [XX] (+90)|
|[XX]|       |[XX]|         ||            |    [XX]      |
|            |    [XX]      ||[XX]|       |[XX]|         |
|            |    [XX]      ||[XX]o       |[XX]o         |
|------------|--------------||------------|--------------|
|o[XX]  (-90)|   o[XX] (+90)||       (+90)|[XX]     (-90)|
||[XX]       |   |[XX]      ||            |[XX]          |
|            |[XX]          |||[XX]       |   |[XX]      |
|            |[XX]          ||o[XX]       |   o[XX]      |
==========================================================

 |> Get side_tile :: ST.
 |> If on horizontal rotation:
 |    > If ST is empty (case 2):
 |        >             For 1st column          For 3rd column
 |        > H = osk__min(ST.x* 64.f - aabb.min_x, aabb.max_x - (ST.x + 1) * 64.f)
 |        > If H < proximity_thresh
 |            > TURN side_tile == right_tile ? 90 : -90;
 |    > If FW is NOT emprty (case 1):
 |        >
 |        > H = osk__min(FW.x * 64 - aabb.max_x, aabb.min_x - (FW.x + 1) * 64)
 |        > If H < proximity_thresh
 |            > TURN side_tile == right_tile ? -90 : 90;
 |        >
 |> If on vertical rotation:
 |    > If ST is empty
 |        >                   For 1st column
 |        > H = osk__min(ST.y * 64 - aabb.min_y, aabb.max_y - ST.y * 64)
 |        > If H < proximity_thresh
 |            > TURN side_tile == right_tile ? 90 : -90;
 |    > If FW is NOT empty:
 |        >                   For 2nd column
 |        > H = osk__min(FW.y * 64 - aabb.max_y, aabb.min_y - (FW.y + 1) * 64)
 |        > If H < proximity_thresh
 |            > TURN side_tile == right_tile ? -90 : 90;
 |        > TODO write this up or del you fucking idiot


 At Any point there are two cases in which the tile should be attached:
   | o>   It should be            o>   The same
 |[XX]  attached to         [XX]
 |[XX]  the side tile       [XX]

So either the side_tile shouldn't be eEmptySpace ||
side_tile should be eEmptySpace and side_tile + (tile_behind)
should't be eEmptySpace

    */
    const float proximity_thresh = 5.f;
    const float tile_attach_thresh = 66.f;
    ivec2 side_tile = ivec2{-1, -1};
    fvec2 aabb_center = fvec2{(aabb.min_x + aabb.max_x) / 2.f,(aabb.min_y + aabb.max_y) / 2.f};
    fvec2 right_tile_center = fvec2{right_tile.x * 64.f + 32.f, right_tile.y * 64.f + 32.f};
    fvec2 left_tile_center = fvec2{left_tile.x * 64.f + 32.f, left_tile.y * 64.f + 32.f};
    fvec2 side_vector = fvec2{-1,-1};
    
    if (distance(right_tile_center, aabb_center) < tile_attach_thresh) {
        side_vector = right;
        side_tile = right_tile;
        
        bool should_attach = scene_get_tile(side_tile) != eEmptySpace ||
            (scene_get_tile(side_tile) == eEmptySpace &&
             scene_get_tile(get_tile_behind(side_tile, forward))  != eEmptySpace);
        
        if (!should_attach) side_tile = ivec2{-1,-1};
    }
    if (distance(left_tile_center, aabb_center) < tile_attach_thresh &&
        (distance(left_tile_center, aabb_center) < distance(right_tile_center, aabb_center) ||
         side_tile.x == -1) ) {
        side_vector = left;
        side_tile = left_tile;
        
        
        bool should_attach = scene_get_tile(side_tile) != eEmptySpace ||
            (scene_get_tile(side_tile) == eEmptySpace &&
             scene_get_tile(get_tile_behind(side_tile, forward))  != eEmptySpace);
        
        if (!should_attach) side_tile = ivec2{-1,-1};
    }
    
    draw_num(side_tile.x, 5, 0); draw_num(side_tile.y, 5, 20);
    
    if (side_tile.x != -1) {
        
        if (dfire->rotation == 0.f || dfire->rotation == 180) {
            
            if (scene_get_tile(side_tile) == eEmptySpace) { // columns 1 & 3
                
                float col1 = dfire->rotation == 000.f
                    ? (side_tile.x * 64.f - aabb.min_x)
                    : FLT_MAX;
                float col3 = dfire->rotation == 180.f
                    ? (aabb.max_x - (side_tile.x + 1) * 64.f)
                    : FLT_MAX;
                float comp = osk__min(col1, col3, FLT_MAX);
                
                assert(comp >= 0 );
                
                if (comp <= proximity_thresh) {
                    dfire->rotation += side_tile == right_tile ? 90.f : -90.f;
                    goto END_ROT;
                    
                }
            }
            if (scene_get_tile(forward_tile) != eEmptySpace) { // columns 2 & 4
                
                float col2 = dfire->rotation == 000.f
                    ? forward_tile.x * 64.f - aabb.max_x
                    : FLT_MAX;
                float col4 = dfire->rotation == 180.f
                    ? aabb.min_x - (forward_tile.x + 1) * 64.f
                    : FLT_MAX;
                
                float comp = osk__min(col2, col4, FLT_MAX);
                assert(comp >= 0 );
                
                if (comp <= proximity_thresh) {
                    dfire->rotation += side_tile == right_tile ? -90.f : 90.f;
                    goto END_ROT;
                }
            }
            
        } else {
            
            if (scene_get_tile(side_tile) == eEmptySpace) { // columns 1 & 3
                float col1 = dfire->rotation == 90.f
                    ? (side_tile.y + 0) * 64.f - aabb.min_y
                    : FLT_MAX;
                float col3 = dfire->rotation == 270.f
                    ? (aabb.max_y - (side_tile.y + 1) * 64.f)
                    : FLT_MAX;
                
                float comp = osk__min(col1, col3, FLT_MAX);
                assert(comp >= 0 );
                
                if (comp <= proximity_thresh){
                    
                    dfire->rotation += side_tile == right_tile ? 90.f : -90.f;
                    goto END_ROT;
                }
            }
            if (scene_get_tile(forward_tile) != eEmptySpace) { // columns 2 & 4
                
                float col2 = dfire->rotation == 90.f
                    ? (forward_tile.y * 64.f - aabb.max_y)
                    : FLT_MAX;
                float col4 = dfire->rotation == 270.f
                    ? (aabb.min_y - (forward_tile.y + 1) * 64.f)
                    : FLT_MAX;
                
                float comp = osk__min(col2, col4, FLT_MAX);
                assert(comp >= 0 );
                
                if (comp <= proximity_thresh){
                    dfire->rotation += side_tile == right_tile ? -90.f : 90.f;
                    goto END_ROT;
                }
            }
            
        }
    } else {
        
        // if we haven't attached to a nearby (side) tile.
        // Check the forward tile and react accordingly
        if (scene_get_tile(forward_tile) != eEmptySpace){
            
            if (dfire->rotation == 180.f) {
                if ((aabb.min_x - (forward_tile.x + 1) * 64.f) < proximity_thresh){
                    dfire->rotation += 90.f;
                    goto END_ROT;
                }
            }
            
            if (dfire->rotation == 90.f) {
                if (forward_tile.y * 64.f - aabb.max_y < proximity_thresh) {
                    dfire->rotation -= 90.f;
                    goto END_ROT;
                }
            }
            
            if (dfire->rotation == 270.f) {
                if (aabb.min_y - (forward_tile.y + 1) * 64.f < proximity_thresh) {
                    dfire->rotation += 90.f;
                    goto END_ROT;
                }
            }
            
            if (dfire->rotation == 0.f) {
                if ((forward_tile.x) * 64.f - aabb.max_x < proximity_thresh) {
                    dfire->rotation += 90.f;
                    goto END_ROT;
                }
            }
            
        }
        
    }
    
    END_ROT:
    dfire->rotation = deg_0_360(dfire->rotation);
    
    float output_rotation = dfire->rotation;
    dfire->position += direction_from_rotation(D2R * (output_rotation)) * 200.f * dt;
    
}


internal void ePlayer_update(Sprite* player, InputState* _istate, float dt)
{
    
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
        //f.mirror.x = player->mirror.x;
        f.rotation = player->mirror.x ? 0.f  : 180.f;
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
    
    
    const float goblin_walk_speed = 80;
    const float goblin_run_speed = 120;
    b32 ignore_player = false;
    b32 is_dying = goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Fall);
    
    
    ignore_player = is_dying;
    
    if (goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Punch) ||
        goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Wait)) {
        // Switch direction and continue when either of these states ends
        if (!goblin->animation_playing) {
            SET_ANIMATION(goblin, Goblin, Walk);
            goblin->mirror.x = !goblin->mirror.x;
        } else {
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
                
                if (tdiff == -goblin->direction())
                {
                    SET_ANIMATION(goblin, Goblin, Chase);
                }
            }
            else
            {
#ifndef NDEBUG
                gl_slow_tilemap_draw(
                                     &GET_TILEMAP_TEXTURE(test),
                                     {block_tile.x * 64.f, block_tile.y * 64.f},
                                     {64.f, 64.f},
                                     0,5 * 5 + 0,
                                     false, false,
                                     NRGBA{0.f, 1.f, 1.f, 1.f});
#endif
            }
            
        }
    }
    
    // Stop at tile edges
    if (!is_dying) {
        const i32 block_stop_offset = 32;
        const ivec2 goblin_tile = map_position_to_tile_centered(goblin->position);
        ivec2 dir_tile = map_position_to_tile_centered(goblin->position + fvec2{goblin->direction() * block_stop_offset, 0});
        ivec2 dir_tile_under = dir_tile + ivec2{0, 1};
        
#ifndef NDEBUG
        gl_slow_tilemap_draw(
                             &GET_TILEMAP_TEXTURE(test),
                             {dir_tile_under.x * 64.f, dir_tile_under.y * 64.f},
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
        const float punch_offset = goblin->direction() * (punch_offset_amount);
        const ivec2 goblin_tile = map_position_to_tile(goblin->position);
        const b32 is_at_edge_of_map =
            (goblin_tile.x == 14 && goblin->direction() == 1) ||
            (goblin->position.x <= 2 && goblin->direction() == -1);
        
        
        ivec2 tile_index = map_position_to_tile_centered(goblin->position + fvec2{punch_offset, 0 });
#ifndef NDEBUG
        gl_slow_tilemap_draw(
                             &GET_TILEMAP_TEXTURE(test),
                             {tile_index.x * 64.f, tile_index.y * 64.f},
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
        goblin->move_and_collide(dt, 900, 450, 450, move_amount * goblin->direction());
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
    
}


internal void eStarRing_update(Sprite* spref, InputState* istate, float dt) {
    double &radius = spref->entity.params[0].as_f64;
    
    const Sprite *const player = scene_get_first_sprite(ePlayer);
    static const fvec2 initial_pos = spref->position;
    static float time = 0.f;
    
    time = fclamp(0.f, 1.f, time + dt * 1.f);
    
    radius = 135.f - 128.f * time + 64.f;
    spref->rotation += dt * 1.f;
    
    
    spref->position.x = lerp(initial_pos.x, player->position.x, time);
    spref->position.y = lerp(initial_pos.y, player->position.y, time);
}


// eGhost
internal void eGhost_update(Sprite* ghost, InputState* istate, float dt) {
    const float ghost_turn_offset_amount = 64;
    const float ghost_speed = 200;
    
    float ghost_turn_offset =ghost->direction()*ghost_turn_offset_amount - ghost->direction()*(ghost_turn_offset_amount/2);
    
    ivec2 ctile = map_position_to_tile_centered(ghost->position +fvec2{ghost_turn_offset,0.f});
    
    // NOTE: Whenever map centered will add 32p to our current value
    // so we'll always never be < 0; add a special case here
    if (ghost->position.x < 0) {
        ctile.x = -1;
    }
    
#ifndef NDEBUG
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(test),
                         {ctile.x * 64.f, ctile.y * 64.f},
                         {64, 64},
                         0,5 * 5 + 0,
                         false, false,
                         NRGBA{1.f, 1.f, 1.f, 1.f});
#endif
    
    EntityBaseType tile_front = (EntityBaseType)scene_get_tile(ctile);
    
    if (ghost->current_animation == GET_CHAR_ANIMENUM(Ghost, Fly)) {
        
        if (tile_front == eBlockFrail) {
            SET_ANIMATION(ghost, Ghost, Punch);
        } else if (tile_front == eBlockSolid) {
            ghost->mirror.x = !ghost->mirror.x;
        }
        
        ghost->position.x += ghost->direction() * ghost_speed * dt;
    } else if (ghost->current_animation == GET_CHAR_ANIMENUM(Ghost, Punch)){
        // destroy block if finished
        if (!ghost->animation_playing) {
            scene_set_tile(ctile, eEmptySpace);
            SET_ANIMATION(ghost, Ghost, Fly);
        }
    }
}