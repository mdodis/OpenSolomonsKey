/*
eBlocks + eEmptySpace
> associate a pickup within that tile; if the tile
> is occupied by a block, then breaking the block will reveal
> the item. If it's not occupied then we read an extra param to say
> whether or not you have to create & break a block there to reveal it,
> or if it's there at startup


*/
// NOTE: change map to level (g_scene...)
internal Sprite *map_add(Map *map, const Sprite *sprite) {
    fail_unless(sprite, "Passing null sprite to map_add");
    
    if (sprite->entity.type == ET_Pickup) {
        map->pickups.push_back(*sprite);
        //inform("Added pickup!");
        return &map->pickups.back();
    } else {
        map->sprites.push_back(*sprite);
        return &map->sprites.back();
    }
}

internal Sprite *scene_sprite_add(const Sprite *const sprite) {
    fail_unless(sprite, "Passing null sprite to scene_add");
    if (sprite->entity.type == ET_Pickup) {
        g_scene.loaded_map.pickups.push_back(*sprite);
        return &g_scene.loaded_map.pickups.back();
    } else {
        // NOTE(mdodis): Sprites like enemies etc sometimes add new sprites to the scene on their update loop, so buffer them per-frame and add them to the central object next frame
        g_scene.sprite_buffer.push_back(*sprite);
        return &g_scene.sprite_buffer.back();
    }
}

internal char* string_parse(char* c, const char *str)
{
    while (*str && *c && *c == *str) {
        c++;
        str++;
    }
    
    if (!(*str)) return c;
    return 0;
}

inline EntityType scene_get_tile(ivec2 p) {
    
    if (p.x > (TILEMAP_COLS - 1) || p.y > (TILEMAP_ROWS - 1) ||
        p.x < 0 || p.y < 0) return ET_BlockSolid;
    
    return g_scene.loaded_map.tiles[p.x][p.y];
}

inline bool scene_tile_empty(ivec2 p) {
    return tile_is_empty(scene_get_tile(p));
}

inline void scene_set_tile(ivec2 p, EntityType t) {
    g_scene.loaded_map.tiles[p.x][p.y] = t;
}

internal void scene_init(const char* level_path) {
}

internal void scene_draw_tilemap() {
    for(int i = 0; i < 15; ++i ) {
        for(int j = 0; j < 12; ++j ) {
            u32 id;
            
            EntityType type = (EntityType)scene_get_tile(ivec2{i,j});
            
            if (type == ET_EmptySpace) continue;
            else if (type == ET_BlockFrail) id = 0 * 5 + 0;
            else if (type == ET_BlockFrailHalf) id = 0 * 5 + 4;
            else if (type == ET_BlockSolid) id = 1 * 5 + 0;
            else continue;
            
            gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials),fvec2{float(i) * 64.f,j *64.f},fvec2{64, 64},0.f,id);
        }
    }
}


// Finds first sprite of specific type
// if not found; return 0
internal Sprite* find_first_sprite(EntityType type) {
    for (i32 i = 0; i < g_scene.loaded_map.sprites.size(); i += 1) {
        Sprite* spref = &g_scene.loaded_map.sprites[i];
        
        if (spref->entity.type == type) return spref;
    }
    return 0;
    
}

internal Sprite *scene_get_pickup_with_id(u64 id) {
    for (i32 i = 0; i < g_scene.loaded_map.pickups.size(); i += 1) {
        Sprite* spref = &g_scene.loaded_map.pickups[i];
        
        if (spref->entity.type == ET_Pickup && spref->entity.params[2].as_u64 == id) {
            return spref;
        }
    }
    return 0;
}

internal Sprite *scene_find_nthsprite(EntityType type, int *n, Map *opt_map = 0) {
    Map *map_to_search = &g_scene.loaded_map;
    if (opt_map) map_to_search = opt_map;
    
    for (i32 i = *n; i < map_to_search->sprites.size(); i += 1) {
        Sprite* spref = &map_to_search->sprites[i];
        
        if (spref->entity.type == type) {
            *n = i;
            return spref;
        }
    }
    
    *n = -1;
    return 0;
}

internal Sprite *find_dragon_fire_sprite(u64 id) {
    for (i32 i = 0; i < g_scene.loaded_map.sprites.size(); i += 1) {
        Sprite *result = &g_scene.loaded_map.sprites[i];
        
        if (result->entity.type == ET_Enemy && get_enemy_type(result) == MT_DragonFire && result->entity.params[1].as_u64 == id) {
            return result;
        }
    }
    return 0;
}

internal Sprite *find_first_enemy_on_tile(EnemyType type, ivec2 tile, Map *opt_map = 0) {
    int n = 0;
    
    do {
        Sprite *next = scene_find_nthsprite(ET_Enemy, &n, opt_map);
        
        if (next) {
            ivec2 tpos = map_position_to_tile_centered(next->position);
            if (tpos == tile && next->entity.params[0].as_etype == type) {
                return next;
            }
            n++;
        }
        
    } while(n != -1);
    return 0;
}

internal Sprite *find_first_sprite_on_tile(EntityType type, ivec2 tile) {
    int n = 0;
    
    do {
        Sprite *next = scene_find_nthsprite(type, &n);
        
        if (next) {
            ivec2 tpos = map_position_to_tile_centered(next->position);
            if (tpos == tile) {
                return next;
            }
            n++;
        }
        
    } while(n != -1);
    
    return 0;
}

static inline bool is_empty_space(ivec2 pos) {
    EntityType type = scene_get_tile(pos);
    return (type == ET_Door || type == ET_EmptySpace);
}

inline internal bool is_frail_block(EntityType type) {
    return (type == ET_BlockFrail || type == ET_BlockFrailHalf);
}

inline internal bool is_block(EntityType type) {
    return (type == ET_BlockFrail || type == ET_BlockFrailHalf || type == ET_BlockSolid);
}

inline internal bool is_frail_block(ivec2 pos) {
    EntityType type = scene_get_tile(pos);
    return (type == ET_BlockFrail || type == ET_BlockFrailHalf);
}

inline internal bool is_block(ivec2 pos) {
    EntityType type = scene_get_tile(pos);
    return (type == ET_BlockFrail || type == ET_BlockFrailHalf || type == ET_BlockSolid);
}

internal void snap_towards_tile(Sprite *spr, ivec2 towards) {
    ivec2 tile = map_position_to_tile_centered(spr->position) + towards;
    fvec2 move = {0.f, 0.f};
    AABox tbox = get_tile_box(tile);
    AABox cbox = spr->get_transformed_AABox();
    
    if (towards.y > 0) {
        move.y = tbox.min_y - cbox.max_y;
    } else if (towards.y < 0) {
        move.y = tbox.max_y - cbox.min_y;
    } else if (towards.x > 0) {
        move.x = tbox.min_x - cbox.max_x;
    } else if (towards.x < 0) {
        move.x = tbox.max_x - cbox.min_x;
    }
    
    spr->position += move;
}

inline internal bool is_out_of_bounds(fvec2 p) {
    return (p.x > 960 || p.x < -64) || (p.y > 768 || p.y < -64);
}

// Decreases block's health properly
inline internal void scene_hit_frail_block(ivec2 tile) {
    EntityType type = scene_get_tile(tile);
    
    if (type == ET_BlockFrail) scene_set_tile(tile, ET_BlockFrailHalf);
    else if (type == ET_BlockFrailHalf) scene_set_tile(tile, ET_EmptySpace);
    else {
        exit_error("scene_hit_frail_block only takes eBlockFrail\\Half");
    }
}

// Returns first view-blocking tile in search direction specified
// otherwise, returns {-1, -1};
internal ivec2 scene_get_first_nonempty_tile(ivec2 start_tile, ivec2 end_tile) {
    i32 xdiff = sgn(end_tile.x - start_tile.x);
    while (start_tile != end_tile) {
        if (scene_get_tile(start_tile) != ET_EmptySpace) {
            return start_tile;
        }
        
        start_tile.x += xdiff;
        
        if (start_tile.x < 0 || start_tile.x > 14) return {-1, -1};
    }
    
    return ivec2{-1 ,-1};
}

internal List_Sprite &scene_get_pickup_list() {
    return g_scene.loaded_map.pickups;
}


static Map smap;
static int hidden_pickup_count = 1;
internal void scene_reset() {
    g_scene.paused_for_key_animation = false;
    g_scene.current_state = SS_STARTUP;
    g_scene.last_score_timer = 0.f;
    g_scene.last_score_num = 0;
    g_scene.player_time = 80.f;
}


// add a normal (non-enemy, non-pickup) entity
// Current is EmptySpace, Blocks, Door, Key
bool add_tilemap_entity(EntityType type, int row, int col, void *custom1) {
    if (is_valid_tilemap_object(type)) {
        smap.tiles[col][row] = type;
    } else {
        Sprite sprite_to_make;
        fvec2 pos = fvec2{(float)col * 64.f, (float)row * 64.f};
        
        switch(type) {
            case ET_Door: {
                sprite_to_make = make_door(pos);
                smap.exit_location = ivec2{col, row};
            }break;
            
            case ET_Key: {
                sprite_to_make = make_key(pos);
                smap.key_location = ivec2{col, row};
                // TODO(mdodis): do key type thingy
            }break;
            
            case ET_PlayerSpawnPoint: {
                sprite_to_make = make_player(pos);
                sprite_to_make.mirror.x = (*((long*)custom1)) == 1;
            }break;
            
            default: {
                assert(0);
            }break;
        }
        
        map_add(&smap, &sprite_to_make);
    }
    return true;
}
// add a pickup that's hidden in row,col
bool add_tilemap_hidden_entity(PickupType type, int row, int col) {
    fvec2 pos = fvec2{(float)col * 64.f, (float)row * 64.f};
    Sprite sprite_to_make = make_pickup(pos, type);
    sprite_to_make.entity.params[1].as_u64 = 1;
    //printf("%d %d %d\n", type, row, col);
    if (!(smap.hidden_pickups[col][row] == 0)) {
        assert(false);
    }
    smap.hidden_pickups[col][row] = hidden_pickup_count;
    sprite_to_make.entity.params[2].as_u64 = hidden_pickup_count;
    ++hidden_pickup_count;
    
    map_add(&smap, &sprite_to_make);
    return true;
}
// add a normal pickup
bool add_tilemap_pickup(PickupType type, int row, int col) {
    fvec2 pos = fvec2{(float)col * 64.f, (float)row * 64.f};
    Sprite sprite_to_make = make_pickup(pos, type);
    map_add(&smap, &sprite_to_make);
    return true;
}
// add an enemy
bool add_tilemap_enemy(EnemyType type, int row, int col, void *param1, void *param2, bool kmirror) {
    fvec2 pos = fvec2{(float)col * 64.f, (float)row * 64.f};
    Sprite sprite_to_make;
    
    switch(type) {
        case MT_Goblin: {
            sprite_to_make = make_goblin(pos);
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
            long dir = *(long*)param2;
            if (dir == 1) sprite_to_make.mirror.x = true;
        }break;
        
        case MT_Ghost: {
            sprite_to_make = make_ghost(pos);
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
            long dir = *(long*)param2;
            if (dir == 1) sprite_to_make.mirror.x = true;
        }break;
        
        case MT_BlueFlame: {
            sprite_to_make = make_blueflame(pos);
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
        }break;
        
        case MT_KMirror: {
            sprite_to_make = make_kmirror(pos);
            sprite_to_make.entity.params[3].as_f64 = *(double*)param1;
            sprite_to_make.entity.params[4].as_f64 = *(double*)param2;
            assert(sprite_to_make.entity.params[6].as_ptr == 0);
        }break;
        
        case MT_Demonhead: {
            sprite_to_make = make_demon_head(pos);
            long dir = *(long*)param2;
            if (dir == 1) sprite_to_make.mirror.x = true;
            sprite_to_make.entity.params[1].as_f64 = 0.0;
        }break;
        
        case MT_Wyvern: {
            sprite_to_make = make_wyvern(pos);
            long dir = *(long*)param2;
            if (dir == 1) sprite_to_make.mirror.x = true;
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
        }break;
        
        case MT_PanelMonster: {
            sprite_to_make = make_panel_monster(pos);
            
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
            
            long *dir = (long*)param2;
            if (*dir == 0) {
                sprite_to_make.mirror.x = false;
                sprite_to_make.rotation = 0.f;
            } else if (*dir == 1) {
                sprite_to_make.mirror.x = true;
                sprite_to_make.rotation = 0.f;
            } else if (*dir == 2) {
                sprite_to_make.mirror.x = true;
                sprite_to_make.rotation = 90.f;
            } else if (*dir == 3) {
                sprite_to_make.mirror.x = false;
                sprite_to_make.rotation = 90.f;
            }
            
        }break;
        
        case MT_Dragon: {
            sprite_to_make = make_dragon(pos);
            long dir = *(long*)param2;
            if (dir == 1) sprite_to_make.mirror.x = false;
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
        }break;
        
        case MT_SparkBall: {
            sprite_to_make = make_spark_ball(pos);
            
            long *dir = (long*)param2;
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
            ivec2 towards;
            
            if (*dir == 0) towards = {+1, 0};
            if (*dir == 1) towards = {-1, 0};
            if (*dir == 2) towards = {0, +1};
            if (*dir == 3) towards = {0, -1};
            
            snap_towards_tile(&sprite_to_make, towards);
        }break;
        
        case MT_Gargoyle: {
            sprite_to_make = make_gargoyle(pos);
            long dir = *(long*)param2;
            if (dir == 1) sprite_to_make.mirror.x = false;
            sprite_to_make.entity.params[1].as_f64 = *(double*)param1;
        }break;
        
        default:{
            assert(0);
        }break;
    }
    
    if (kmirror) {
        Sprite *ksprite = (Sprite*)malloc(sizeof(Sprite));
        *ksprite = sprite_to_make;
        assert(ksprite);
        
        Sprite *kmirror = find_first_enemy_on_tile(MT_KMirror, ivec2{col, row}, &smap);
        assert(kmirror);
        
        if (kmirror->entity.params[5].as_ptr == 0) {
            kmirror->entity.params[5].as_ptr = ksprite;
        } else {
            kmirror->entity.params[6].as_ptr = ksprite;
        }
        
    } else {
        map_add(&smap, &sprite_to_make);
    }
    return true;
}

internal void scene_lose() {
    g_scene.player_is_dead = true;
    g_scene.current_state = SS_LOSE;
}

bool add_tilemap_background(long num) {
    gl_load_background_texture(num);
    return true;
}

internal void clear_map(Map *map) {
    map->sprites.clear();
    map->pickups.clear();
    for (int j = 0; j < TILEMAP_COLS; j += 1){
        for (int i = 0; i < TILEMAP_ROWS; i += 1){
            map->tiles[j][i] = ET_EmptySpace;
            map->hidden_pickups[j][i] = 0;
        }
    }
    
}

internal void load_map(Map *m, const char *path) {
    hidden_pickup_count = 1;
    load_map_from_file(path, 0);
    *m = smap;
}

internal void load_next_map() {
    g_scene.current_level_counter++;
    clear_map(&smap);
    
    static char buf[256];
    sprintf(buf, "level_%u.osk", g_scene.current_level_counter);
    
    
    scene_reset();
    load_map(&g_scene.loaded_map, buf);
    audio_play_sound(GET_SOUND(SND_background), true, SoundType::Music, false);
}

internal void reload_map() {
    clear_map(&smap);
    
    static char buf[256];
    sprintf(buf, "level_%u.osk", g_scene.current_level_counter);
    
    load_map(&g_scene.loaded_map, buf);
}

#include "animations.cpp"

typedef UPDATE_ENTITY_FUNC(UpdateEntityFunc);

global UpdateEntityFunc *enemy_update_table[] = {
    0,
    DemonHead_update,
    Dragon_update,
    Gargoyle_update,
    Ghost_update,
    Goblin_update,
    0,
    0,
    Wyvern_update,
    PanelMonster_update,
    0,
    SparkBall_update,
    BlueFlame_update,
    0,
    0,
    0,
    0,
    KMirror_update,
    PanelMonsterFlame_update,
    DragonFire_update,
};


UPDATE_ENTITY_FUNC(Enemy_update) {
    if (enemy_update_table[spref->entity.params[0].as_etype] != 0) {
        (*enemy_update_table[spref->entity.params[0].as_etype])(spref, istate, dt);
    } else {
        exit_error("Enemy_update");
    }
}

UPDATE_ENTITY_FUNC(Effect_update) {
    if (!spref->animation_playing) {
        spref->mark_for_removal = true;
        
        if (spref->current_animation == GET_CHAR_ANIMENUM(Effect, FlashBlk)) {
            ivec2 target_tile;
            target_tile.x = spref->entity.params[0].as_i64;
            target_tile.y = spref->entity.params[1].as_i64;
            EntityType type = scene_get_tile(target_tile);
            if (is_frail_block(type)) {
                scene_set_tile(target_tile, ET_EmptySpace);
            } else {
                scene_set_tile(target_tile, ET_BlockFrail);
            }
        }
    }
}

UPDATE_ENTITY_FUNC(Pickup_update) {
    if (spref->entity.params[3].as_i64 == 1)
        spref->move_and_collide(dt, 600, 450, 0, 0, false);
}

global UpdateEntityFunc *entity_update_table[] = {
    0,
    0,
    0,
    0,
    0,
    Player_update,
    Enemy_update,
    0,
    0,
    Pickup_update,
    Fairie_update,
    Effect_update,
    DFireball_update
};

UPDATE_ENTITY_FUNC(Entity_update) {
    spref->update_animation(dt);
    if (entity_update_table[spref->entity.type] != 0) {
        (*entity_update_table[spref->entity.type])(spref, istate, dt);
    }
}

internal void scene_update(InputState* istate, float dt) {
    
    for (int i = 0; i < g_scene.loaded_map.pickups.size(); ++i) {
        Sprite *pickup = &g_scene.loaded_map.pickups[i];
        draw_pickup(pickup);
        
        Entity_update(pickup, istate, dt);
    }
    scene_draw_tilemap();
    
    {
        List_Sprite &l = g_scene.loaded_map.sprites;
        // remove marked elements
        auto it = l.begin();
        while (it != l.end()) {
            Sprite& spref = (*it);
            if (spref.mark_for_removal) {
                it = l.erase(it);
            } else {
                it++;
            }
        }
    }
    
    {
        List_Sprite &l = g_scene.loaded_map.pickups;
        // remove marked elements
        auto it = l.begin();
        while (it != l.end()) {
            Sprite& spref = (*it);
            if (spref.mark_for_removal) {
                it = l.erase(it);
            } else {
                it++;
            }
        }
    }
    
    if (g_scene.paused_for_key_animation)
        scene_key_animation(dt);
    
    
    while(g_scene.sprite_buffer.size() != 0) {
        g_scene.loaded_map.sprites.push_back(g_scene.sprite_buffer.back());
        g_scene.sprite_buffer.pop_back();
    }
    
    List_Sprite &l = g_scene.loaded_map.sprites;
    for (int i = 0; i < l.size(); ++i) {
        Sprite* spref = &l[i];
        
        if (!g_scene.paused_for_key_animation) {
            Entity_update(spref, istate, dt);
        }
#ifndef NDEBUG
        // Draw the Bounding box sprite
        AABox box = spref->get_transformed_AABox();
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials),
                             {box.min_x, box.min_y},
                             {box.max_x - box.min_x, box.max_y - box.min_y},
                             0,1 * 5 + 1,
                             false, false,
                             NRGBA{1.f, 0, 1.f, 0.7f});
#endif
        draw(&l[i]);
    }
    if (!g_scene.paused_for_key_animation) {
        g_scene.player_time -= dt;
        
        if ((g_scene.player_time * 100) < 2000u && !g_scene.time_is_low_enough) {
            audio_remove(SoundType::Music);
            audio_play_sound(GET_SOUND(SND_hurry), false, SoundType::Music);
            g_scene.time_is_low_enough = true;
            
        }
    }
}

