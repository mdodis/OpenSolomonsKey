void Sprite::move_and_collide(float dt, const float GRAVITY, const float MAX_YSPEED, const float JUMP_STRENGTH, float XSPEED, b32 damage_tiles) {
    this->velocity.x = XSPEED;
    this->velocity.y += GRAVITY * dt;
    this->velocity.y = clamp(-JUMP_STRENGTH, MAX_YSPEED, this->velocity.y);
    
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
    ivec2 start_tile = clamp(ivec2{0,0}, ivec2{14,11},
                             map_position_to_tile_centered(ipos) - 1);
    
    b32 collided_on_bottom = false;
    
    for (i32 j = 0; j < 3; ++j) {
        for (i32 i = 0; i < 3; ++i) {
            if (scene_tile_empty(ivec2{start_tile.x + i,start_tile.y + j})) continue;
            
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
            if (collided) {
                this->position = this->position - (diff);
                
                // If we are moving up and diff moved us in the Y dir,
                // then negate the collision.
                // (fixes bouncing when hitting corner of a tile)
                if (this->velocity.y < 0 && iabs(diff.y) < 5) {
                    this->position.y += diff.y;
                    continue;
                }
                
                
                if (j == 2 || (start_tile.y <= 1 && j == 1)) {
                    // NOTE(miked): j == 2 is the bottom tile in most cases,
                    // but if y == 1 in tile space, then j will be 1 in that case
                    
                    collided_on_bottom = true;
                    if (iabs(diff.y) > 0) {
                        this->is_on_air = false;
                        this->velocity.y = 0;
                    }
                }
                
                // Bouncing
                if (this->velocity.y < 0 &&
                    this->is_on_air &&
                    this_trans.min_x < collision.max_x &&
                    diff.x >= 0) {
                    
                    this->velocity.y = -this->velocity.y * 0.737;
                    this->is_on_air = true;
                    
                    // it has to be mostly above the this, in order to avoid
                    // destroying blocks diagonally
                    if ( i != 1 || j != 0 || !damage_tiles) continue;
                    
                    ivec2 current_tile = {start_tile.x + i,start_tile.y + j};
                    if (is_frail_block(scene_get_tile(current_tile))) {
                        
                        scene_hit_frail_block(current_tile);
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
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials), {bound_bottom.min_x, bound_bottom.min_y}, {bound_bottom.max_x, bound_bottom.max_y}, 0.f, 1 * 5 + 1, false, false, NRGBA{0,1,1,.7f});
    
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials), {bound_right.min_x, bound_right.min_y}, {bound_right.max_x, bound_right.max_y}, 0.f, 1* 5 + 1, false, false, NRGBA{0,1,1,.7f});
    
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials), {bound_left.min_x + 32, bound_left.min_y}, {bound_left.max_x + 32, bound_left.max_y}, 0.f, 1* 5 + 1,false, false, NRGBA{0,1,1,.7f});
#endif
}


internal void ePlayer_cast(Sprite* player, float dt) {
    // Get the upper left tile based on the center of the player sprite
    ivec2 player_tile = clamp({0,0}, {14,11}, map_position_to_tile_centered(player->position));
    ivec2 target_tile = player_tile;
    
    if (player->mirror.x) {
        if (player_tile.x >= 14) return;
        target_tile.x += 1;
    } else {
        if (player_tile.x <= 0) return;
        target_tile.x -= 1;
    }
    
    // if we were crouching: special condition
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Crouch)) {
        target_tile.y = clamp(0, 14,target_tile.y + 1);
    }
    Sprite *enemy_on_tile = find_first_sprite_on_tile(eEnemy, target_tile);
    if (!enemy_on_tile) enemy_on_tile = find_first_sprite_on_tile(eDoor, target_tile);
    
    if (enemy_on_tile && get_enemy_type(enemy_on_tile) == EnemyType::BlueFlame) enemy_on_tile = 0;
    
    SET_ANIMATION(player, test_player, Cast);
    if (!enemy_on_tile) {
        EntityBaseType type = (EntityBaseType)scene_get_tile(target_tile);
        bool should_spawn_flash_effect = true;
        bool put_frail_block;
        
        if (is_frail_block(type)) {
            scene_set_tile(target_tile, eEmptySpace);
            put_frail_block = false;
        } else if (type == eEmptySpace) {
            
            Sprite *blue_flame;
            blue_flame = find_first_enemy_on_tile(EnemyType::BlueFlame, target_tile);
            if (blue_flame) {
                BlueFlame_cast(blue_flame);
                should_spawn_flash_effect = false;
            } else {
                put_frail_block = true;
                scene_set_tile(target_tile, eBlockFrail);
            }
            
        }
        
        u64 pref_id = g_scene.loaded_map.hidden_pickups[target_tile.x][target_tile.y];
        
        if (pref_id > 0) {
            
            Sprite *pref = scene_get_pickup_with_id(pref_id);
            if (pref) {
                assert(pref->entity.type == ePickup);
                pref->entity.params[1].as_u64 = put_frail_block ? 1u : 0u;
            }
            
        }
        
        if (should_spawn_flash_effect) {
            Sprite flash = make_effect(tile_to_position(target_tile), GET_CHAR_ANIM_HANDLE(Effect, Flash));
            scene_sprite_add(&flash);
        }
    } else {
        Sprite hit = make_effect(enemy_on_tile->position, GET_CHAR_ANIM_HANDLE(Effect, Hit));
        
        if (player->position.x < enemy_on_tile->position.x) {
            hit.mirror.x = true;
        }
        
        scene_sprite_add(&hit);
    }
}


internal void eDFireball_update(Sprite* dfire, InputState* _istate, float dt)
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
NOTE: when revisiting think about this:

while (there is a wall in front of us)
    -    turn left
move forward
if (there is no wall to our right)
    -    turn right

Wall follower algorithm

And tile-wise there are two general situations:
     1) The tile in front is blocking the way.
2) The tile in front(target_tile) is empty.
What is important in every one of these, is
knowing which tile we're going to be "attached"
to (side_tile).
=======================HORIZONTAL=========================
============ROT 0===========||==========ROT 180===========
|            |    [XX]      ||            |      [XX]    |
|   o>  (+90)|  o>[XX] (-90)||(-90)  <o   |(+90) [XX]<o  |
|[XX] | [XX] |[XX]          ||        [XX]|          [XX]|
|[XX]   [XX] |[XX]          ||        [XX]|          [XX]|
|------------|--------------||------------|--------------|
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
                
                //printf("2 is %f 4 is %f\n", col2, col4);
                
                // NOTE(miked): when the fireball is spawned right next
                // to a tile it could attach, it's basically already inside the
                // tile, so special to case to make it behave
                if (col2 < 0.f) {
                    if (fabs(col2) < 15.f) {
                        col2 = -col2 -10.f;
                    }
                }
                
                float comp = osk__min(col2, col4, FLT_MAX);
                assert(comp >= 0 );
                
                if (comp <= proximity_thresh) {
                    dfire->rotation += side_tile == right_tile ? -90.f : 90.f;
                    //printf("DO\n");
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

internal void player_reach_door(Sprite *player, Sprite *door) {
    if (g_scene.player_has_key) {
        SET_ANIMATION(door, Door, Close);
    }
}

internal void ePlayer_update(Sprite* player, InputState* _istate, float dt) {
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
    
    if (did_jump)  audio_play_sound(GET_SOUND(SND_jump));
    
    // Item pickup
    {
        List_Sprite &pickups = scene_get_pickup_list();
        AABox *player_box = &player->get_transformed_AABox();
        for (Sprite &s : pickups) {
            AABox *s_box = &s.get_transformed_AABox();
            if (intersect(player_box, s_box)) {
                player_pickup(player, &s);
            }
        }
    }
    
    // Key Pickup
    {
        Sprite *key = find_first_sprite(eKey);
        if (key) {
            AABox key_box = key->get_transformed_AABox();
            AABox player_box = player->get_transformed_AABox();
            
            if (intersect(&player_box, &key_box)) {
                play_key_get_animation();
            }
        }
    }
    
    // Door
    {
        Sprite *door = find_first_sprite(eDoor);
        assert(door);
        
        AABox box = door->get_transformed_AABox();
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials),
                             {box.min_x, box.min_y},
                             {box.max_x - box.min_x, box.max_y - box.min_y},
                             0,1 * 5 + 1,
                             false, false,
                             NRGBA{1.f, 0, 1.f, 0.7f});
        AABox player_box = player->get_transformed_AABox();
        if (intersect(&player_box,&box)) {
            player_reach_door(player, door);
        }
    }
    
    // Casting
    if (GET_KEYPRESS(cast) && player->current_animation != GET_CHAR_ANIMENUM(test_player, Cast)) {
        // save yspeed
        player_last_yspeed = player->velocity.y;
        // cast!
        player->velocity.x = 0;
        player->velocity.y = 0;
        ePlayer_cast(player, dt);
    }
    
    // Fireball Casting
    if (GET_KEYPRESS(fireball) && player->current_animation != GET_CHAR_ANIMENUM(test_player, Cast)) {
        
        Sprite f = make_dfireball(player->position + fvec2{16, 10});
        
        f.rotation = player->mirror.x ? 0.f  : 180.f;
        Sprite *p = scene_sprite_add(&f);
        
    }
    
    if (player->current_animation == GET_CHAR_ANIMENUM(test_player, Cast)) {
        
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
    
    player->move_and_collide(dt,
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

internal void Goblin_update(Sprite* goblin, InputState* _istate, float dt) {
    /*
    NOTE: Sprite::mirror is a bool, and sprites by default look to the left,
    so invert direction vector to get the axis-compliant direction in X.
    */
    
    double &goblin_walk_speed = goblin->entity.params[1].as_f64;
    const float goblin_run_speed = 120;
    b32 ignore_player = false;
    b32 is_dying = goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Fall);
    
    ignore_player = is_dying;
    
    if (goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Punch) || goblin->current_animation == GET_CHAR_ANIMENUM(Goblin, Wait)) {
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
        const Sprite* const player = find_first_sprite(ePlayer);
        fail_unless(player, "Player sprite not found scene_get_first_sprite");
        
        fvec2 ppos = player->position;
        ivec2 ppos_tile = map_position_to_tile_centered(ppos);
        
        ivec2 goblin_tile = map_position_to_tile_centered(goblin->position);
        
        if (goblin_tile.y == ppos_tile.y && !ignore_player) {
            // search in the direction of the goblin to see if there is
            // an obstacle blocking its view of the player
            i32 tdiff = sgn(goblin_tile.x - ppos_tile.x);
            
            ivec2 block_tile = scene_get_first_nonempty_tile(goblin_tile, ppos_tile);
            if (block_tile == ivec2{-1, -1}) {
                persist i32 blocking_tiles = 0;
                
                if (tdiff == -goblin->direction()) {
                    SET_ANIMATION(goblin, Goblin, Chase);
                }
            } else {
#ifndef NDEBUG
                gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials),
                                     {block_tile.x * 64.f, block_tile.y * 64.f},
                                     {64.f, 64.f},
                                     0,1 * 5 + 2,
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
                             &GET_TILEMAP_TEXTURE(TM_essentials),
                             {dir_tile_under.x * 64.f, dir_tile_under.y * 64.f},
                             {64, 64},
                             0,1 * 5 + 1,
                             false, false,
                             NRGBA{1.f, 0.f, 0.f, 1.f});
#endif
        if (scene_get_tile(dir_tile_under) == eEmptySpace && scene_get_tile(dir_tile) == eEmptySpace && goblin_tile.y != 11) {
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
                             &GET_TILEMAP_TEXTURE(TM_essentials),
                             {tile_index.x * 64.f, tile_index.y * 64.f},
                             {64, 64},
                             0,1 * 5 + 1,
                             false, false,
                             NRGBA{0.f, 1.f, 0.f, 1.f});
#endif
        if ((scene_get_tile(tile_index) != eEmptySpace || is_at_edge_of_map) && !(goblin->is_on_air)) {
            SET_ANIMATION(goblin, Goblin, Punch);
        }
    }
    
    // Movement
    i32 move_amount;
    switch(goblin->current_animation) {
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
    
    if (!is_dying) {
        goblin->move_and_collide(dt, 900, 450, 450, move_amount * goblin->direction());
    } else {
        goblin->velocity.y += 900 * dt;
        goblin->velocity.y = clamp(-450.f, 450.f, goblin->velocity.y);
        
        goblin->position.y += goblin->velocity.y * dt;
    }
    
    if (iabs(goblin->velocity.x) > 0) goblin->mirror.x = goblin->velocity.x > 0;
    
    if (goblin->is_on_air && !is_dying) {
        SET_ANIMATION(goblin, Goblin, Fall);
    } else if (is_dying) {
        if (goblin->position.y > (12 * 64)) {
            goblin->mark_for_removal = true;
            inform("You killed a Goblin, ouchie!");
        }
    }
    
}

// eGhost
internal void Ghost_update(Sprite* ghost, InputState* istate, float dt) {
    const float ghost_turn_offset_amount = 64;
    double &ghost_speed = ghost->entity.params[1].as_f64;
    
    float ghost_turn_offset =ghost->direction()*ghost_turn_offset_amount - ghost->direction()*(ghost_turn_offset_amount/2);
    
    ivec2 ctile = map_position_to_tile_centered(ghost->position +fvec2{ghost_turn_offset,0.f});
    
    // NOTE: Whenever map centered will add 32p to our current value
    // so we'll always never be < 0; add a special case here
    if (ghost->position.x < 0) {
        ctile.x = -1;
    }
    
#ifndef NDEBUG
    gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials),
                         {ctile.x * 64.f, ctile.y * 64.f},
                         {64, 64},
                         0,1 * 5 + 2,
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
            ghost->mirror.x = !ghost->mirror.x;
            SET_ANIMATION(ghost, Ghost, Fly);
        }
    }
}

internal void BlueFlame_cast(Sprite* flame) {
    const double &flame_tame_dur   = flame->entity.params[1].as_f64;
    double &flame_tame_timer       = flame->entity.params[2].as_f64;
    
    flame_tame_timer = 0;
    if (flame->current_animation == GET_CHAR_ANIMENUM(BlueFlame, Normal)) {
        SET_ANIMATION(flame, BlueFlame, Tame);
    }
}

// eBlueFlame
internal void BlueFlame_update(Sprite* flame, InputState* istate, float dt) {
    const double &flame_tame_dur   = flame->entity.params[1].as_f64;
    double &flame_tame_timer       = flame->entity.params[2].as_f64;
    
    if (flame_tame_timer != -FLT_MAX) {
        flame_tame_timer += dt;
        
        if (flame_tame_timer >= flame_tame_dur) {
            // revert
            flame_tame_timer = -FLT_MAX;
            SET_ANIMATION(flame, BlueFlame, Normal);
        }
    }
}

internal void eFairie_update(Sprite* fairie, InputState* istate, float dt) {
    u64 &state          = fairie->entity.params[0].as_u64;
    double &time_passed = fairie->entity.params[1].as_f64;
    
    // Try to move towards the player diagonally, making \/ shapes at most 2 times
    // Then perform a half circle movement around the player on a specific radius.
    // DIAG1, one whole diagonal movement?
    //
    //
    
    // get player location
    Sprite *player = find_first_sprite(ePlayer);
    if (!player) return;
    
    fvec2 f_to_player = normalize(player->position - fairie->position);
    fvec2 dir = {0,0};
    
    if (state == 0) {
        dir = f_to_player;
        fairie->velocity = dir;
    } else if (state < 4) {
        dir = fairie->velocity;
    } else {
        // circle
        float angle = time_passed;
        dir = direction_from_rotation(angle)* 512.f + player->position;
        
        dir = normalize(dir - fairie->position);
    }
    
    draw_num(state, 5);
    
    fairie->position.x += (100.f * dt * dir.x);
    fairie->position.y += (100.f * dt * dir.y);
    
    bool switch_state = false;
    fvec2 ipos = {fairie->position.x, fairie->position.y};
    ivec2 start_tile = map_position_to_tile_centered(ipos) - 1;
    for (i32 j = 0; j < 3; ++j) {
        for (i32 i = 0; i < 3; ++i) {
            if (scene_tile_empty(ivec2{start_tile.x + i,start_tile.y + j})) continue;
            
            fvec2 tile_coords = {
                (start_tile.x + i) * 64.f,
                (start_tile.y + j) * 64.f
            };
            
            AABox collision = {0, 0, 64, 64};
            collision = collision.translate(tile_coords);
            AABox this_trans = fairie->get_transformed_AABox();
            
            fvec2 diff;
            b32 collided = aabb_minkowski(&this_trans, &collision, &diff);
            if (collided) {
                switch_state = true;
                
                fairie->position = fairie->position - (diff);
            }
        }
    }
    
    if (switch_state) {
        state = clamp(0, 5, state + 1);
        if (state != 5) {
            fairie->velocity = normalize({f_to_player.x * random11(), f_to_player.y * random11()});
        }
    }
    
    if (state == 5) time_passed += dt;
    
    if (time_passed > 2.f) {
        time_passed = 0.f;
        state = 0;
    }
}

internal void player_pickup(Sprite *player, Sprite *pickup) {
    
    if (pickup->entity.params[1].as_u64 == 0) {
        
        pickup->mark_for_removal = true;
        PickupType type = (PickupType)pickup->entity.params[0].as_u64;
        
        if (pickup_type_is_non_effect(type)) {
            long score_to_add = get_pickup_worth(type);
            
            add_score(score_to_add);
            
            Sprite flash2 = make_effect(pickup->position, GET_CHAR_ANIM_HANDLE(Effect, Flash2));
            scene_sprite_add(&flash2);
        } else if (pickup_type_is_valid(type)) {
            
            if (type == PickupType::Bell) {
                Sprite fairie = make_fairie(tile_to_position(g_scene.loaded_map.exit_location), 0);
                scene_sprite_add(&fairie);
            }
            
        }
    }
}

internal void KMirror_spawn(Sprite *kmirror) {
    Sprite *mv1 = (Sprite *)kmirror->entity.params[5].as_ptr;
    Sprite *mv2 = (Sprite *)kmirror->entity.params[6].as_ptr;
    
    if (mv1) {
        const Sprite *sp = mv1;
        scene_sprite_add(sp);
    }
    
    printf("%p %p\n", mv1, mv2);
    // swap spawn sprite primitives
    if (mv2 != 0) {
        Sprite tmp = *mv1;
        *mv1 = *mv2;
        *mv2 = tmp;
    }
}

internal void KMirror_update(Sprite* kmirror, InputState* istate, float dt) {
    double &current_timer = kmirror->entity.params[1].as_f64;
    double &current_delay_timer = kmirror->entity.params[2].as_f64;
    const double &delay = kmirror->entity.params[3].as_f64;
    const double &interval = kmirror->entity.params[4].as_f64;
    
    if (current_delay_timer >= 0.f) {
        // initial delay
        current_delay_timer += dt;
        
        if (current_delay_timer > delay) {
            current_delay_timer = -1.f;
            KMirror_spawn(kmirror);
        }
        
    } else {
        
        current_timer += dt;
        if (current_timer > interval) {
            current_timer = 0.f;
            KMirror_spawn(kmirror);
        }
    }
}