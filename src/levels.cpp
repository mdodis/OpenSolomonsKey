/*
eBlocks + eEmptySpace
> associate a pickup within that tile; if the tile
> is occupied by a block, then breaking the block will reveal
> the item. If it's not occupied then we read an extra param to say
> whether or not you have to create & break a block there to reveal it,
> or if it's there at startup


*/
// NOTE: change map to level (g_scene...)
internal Sprite *map_add(Map *map, Sprite *sprite) {
    fail_unless(sprite, "Passing null sprite to map_add");
    
    if (sprite->entity.type == ePickup) {
        map->pickups.push_back(*sprite);
        //inform("Added pickup!");
        return &map->pickups.back();
    } else {
        map->sprites.push_back(*sprite);
        return &map->sprites.back();
    }
}

internal Sprite *scene_sprite_add(Sprite *sprite)
{
    fail_unless(sprite, "Passing null sprite to scene_add");
    
    if (sprite->entity.type == ePickup) {
        g_scene.loaded_map.pickups.push_back(*sprite);
        return &g_scene.loaded_map.pickups.back();
    } else {
        g_scene.loaded_map.sprites.push_back(*sprite);
        return &g_scene.loaded_map.sprites.back();
    }
}

internal char* string_nextline(char* c)
{
    while (*c != '\n') c++;
    return c + 1;
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

internal char* string_parse_uint(char* c, u64* out_i)
{
#define IS_DIGIT(x) (x >= '0' && x <= '9')
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
#undef IS_DIGIT
}

internal b32 is_valid_tilemap_object(EntityBaseType type)
{
    return ((u64)type <= (u64)eBlockSolid);
}

internal char* eat_whitepspace(char *c) {
    while(*c == ' ' || *c == '\t') c++;
    return c;
}


internal char *parse_custom(Map *const map, char *c, fvec2 pos) {
    c = eat_whitepspace(c);
    
    while (*c && *c == ',') {
        u64 object_id;
        c++;
        c = string_parse_uint(c, &object_id);
        
        Sprite pickup = make_pickup(pos, object_id);
        
        map_add(map, &pickup);
        //inform("Under Normal tile: %lld", object_id);
    }
    return c;
}

internal bool load_map(Map *const map, const char *path) {
    constexpr char * loader_version = "V0.2";
    bool level_validity[] = {
        false,  // Door exists
        false,  // Key exists
        false   // Player exists
    };
    
    char *data = platform_load_entire_file(path);
    char *c = data;
    // if no path is given use map's name for reloading
    if (!path) path = map->name;
    
    if (!data) {
        error("Failed to load map %s!", path);
        return false;
    }
    
    map->sprites.clear();
    map->pickups.clear();
    
    map->name = path;
    if (!string_parse(c, loader_version)) {
        warn("Map %s does not match loader version", path);
    }
    c += 5;
    
    u32 counter_x = 0;
    u32 counter_y = 0;
    
    while (*c) {
        switch(*c) {
            
            case '#': {
                c = string_nextline(c);
            } break;
            
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                u64 res;
                c = string_parse_uint(c, &res);
                
                if (res >= EntityBaseType_Count) {
                    error("Entity type %lld does not exist in loader verison %s!", res, loader_version);
                    return false;
                }
                
                if (counter_x >= TILEMAP_COLS) {
                    counter_x = 0;
                    counter_y++;
                }
                
                if (is_valid_tilemap_object((EntityBaseType) res)) {
                    
                    map->tiles[counter_x][counter_y] = (EntityBaseType)res;
                    
                    c = parse_custom(map, c,fvec2{ counter_x * 64.f,counter_y * 64.f});
                } else {
                    
                    if (res == eDoor) {
                        level_validity[0] = true;
                        map->exit_location = ivec2{i32(counter_x), i32(counter_y)};
                    } else if (res == eKey) {
                        level_validity[1] = true;
                        map->key_location = ivec2{i32(counter_x), i32(counter_y)};
                    }
                    
                    Sprite sprite_to_make;
                    fvec2 sprite_initial_pos = fvec2{ (float)counter_x * 64, (float)counter_y * 64};
                    
                    switch((EntityBaseType)res) {
                        case eGoblin:{
                            sprite_to_make = make_goblin(sprite_initial_pos);
                            c = eGoblin_parse(&sprite_to_make, c);
                        }break;
                        
                        case ePlayerSpawnPoint:{
                            level_validity[2] = true;
                            sprite_to_make = make_player(sprite_initial_pos);
                        }break;
                        
                        case eGhost:{
                            sprite_to_make = make_ghost(sprite_initial_pos);
                        }break;
                        
                        case eDoor: {
                            sprite_to_make = make_door(sprite_initial_pos);
                        }break;
                        
                        case eKey: {
                            sprite_to_make = make_key(sprite_initial_pos);
                        }break;
                        
                        case eBlueFlame: {
                            sprite_to_make = make_blueflame(sprite_initial_pos);
                            c = eBlueFlame_parse(&sprite_to_make, c);
                        }break;
                        
                        case ePickup: {
                            sprite_to_make = make_pickup(sprite_initial_pos, 0);
                            c = ePickup_parse(&sprite_to_make, c);
                        }break;
                        
                        default:{
                            error("sprite type not available for make_");
                            exit(0);
                        }break;
                    }
                    
                    sprite_to_make.position.y += 64.f - sprite_to_make.collision_box.max_y;
                    
                    map_add(map, &sprite_to_make);
                    map->tiles[counter_x][counter_y] = eEmptySpace;
                }
                
                counter_x++;
                
            } break;
            
            default: {
                c++;
            } break;
        }
    }
    
    for (int i = 0; i < ARRAY_COUNT(level_validity); i += 1) {
        fail_unless(level_validity[i], "Level invalid");
    }
    
    
    free(data);
    return true;
}

inline EntityBaseType scene_get_tile(ivec2 p) {
    
    if (p.x > (TILEMAP_COLS - 1) || p.y > (TILEMAP_ROWS - 1) ||
        p.x < 0 || p.y < 0) return eBlockSolid;
    
    return g_scene.loaded_map.tiles[p.x][p.y];
}

inline bool scene_tile_empty(ivec2 p) {
    return tile_is_empty(scene_get_tile(p));
}

inline void scene_set_tile(ivec2 p, EntityBaseType t) {
    g_scene.loaded_map.tiles[p.x][p.y] = t;
}

internal void scene_init(const char* level_path) {
}

internal void
scene_draw_tilemap() {
    for(int i = 0; i < 15; ++i ) {
        for(int j = 0; j < 12; ++j ) {
            u32 id;
            
            EntityBaseType type = (EntityBaseType)scene_get_tile(ivec2{i,j});
            
            if (type == eEmptySpace) continue;
            else if (type == eBlockFrail) id = 0 * 5 + 0;
            else if (type == eBlockFrailHalf) id = 0 * 5 + 4;
            else if (type == eBlockSolid) id = 1 * 5 + 0;
            else continue;
            
            gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_essentials),
                                 fvec2{float(i) * 64.f, j * 64.f},
                                 fvec2{64, 64},
                                 0.f,
                                 id);
        }
    }
}




// Finds first sprite of specific type
// if not found; return 0
internal Sprite* scene_get_first_sprite(EntityBaseType type) {
    for (i32 i = 0; i < g_scene.loaded_map.sprites.size(); i += 1) {
        Sprite* spref = &g_scene.loaded_map.sprites[i];
        
        if (spref->entity.type == type) return spref;
    }
    return 0;
    
}

internal Sprite *scene_find_nthsprite(EntityBaseType type, int *n) {
    for (i32 i = *n; i < g_scene.loaded_map.sprites.size(); i += 1) {
        Sprite* spref = &g_scene.loaded_map.sprites[i];
        
        if (spref->entity.type == type) {
            *n = i;
            return spref;
        }
    }
    
    *n = -1;
    return 0;
}

internal Sprite *scene_find_first_sprite_on_tile(EntityBaseType type, ivec2 tile) {
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


inline internal bool is_frail_block(EntityBaseType type) {
    return (type == eBlockFrail || type == eBlockFrailHalf);
}


// Decreases block's health properly
inline internal void scene_hit_frail_block(ivec2 tile) {
    EntityBaseType type = scene_get_tile(tile);
    
    if (type == eBlockFrail) scene_set_tile(tile, eBlockFrailHalf);
    else if (type == eBlockFrailHalf) scene_set_tile(tile, eEmptySpace);
    else {
        exit_error("scene_hit_frail_block only takes eBlockFrail\\Half");
    }
}

// Returns first view-blocking tile in search direction specified
// otherwise, returns {-1, -1};
internal ivec2 scene_get_first_nonempty_tile(ivec2 start_tile, ivec2 end_tile) {
    i32 xdiff = sgn(end_tile.x - start_tile.x);
    while (start_tile != end_tile) {
        if (scene_get_tile(start_tile) != eEmptySpace) {
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

global float startup_anim_time = 0.f;
internal void startup_animation_reset() {
    g_scene.startup_state = 0;
    g_scene.playing = false;
    startup_anim_time = 0.f;
}

// One star from door to key
// show key
// Circle star from key to player
internal void scene_startup_animation(float dt) {
    
    const int STATE_START = 0;
    const int STATE_SHOW_KEY = 1;
    const int STATE_SHOW_PLAYER = 2;
    const int STATE_DONE = 3;
    
    const float anim_dur = 0.8f;
    
    static Sprite ring_static;
    Sprite *ring;
    
    ring = &ring_static;
    
    const Sprite *const player = scene_get_first_sprite(ePlayer);
    Sprite *door = scene_get_first_sprite(eDoor);
    
    // placeholder
    const fvec2 DOOR = tile_to_position(g_scene.loaded_map.exit_location);
    const fvec2 KEY = tile_to_position(g_scene.loaded_map.key_location);
    
    if (GET_KEYPRESS(space_pressed)) {
        g_scene.startup_state = 4;
        //ring->mark_for_removal = true;
        g_scene.playing = true;
    }
    
    switch (g_scene.startup_state) {
        
        case STATE_START: {
            ring_static = make_starring(DOOR);
            
            g_scene.startup_state = STATE_SHOW_KEY;
        } break;
        
        case STATE_SHOW_KEY: {
            
            ring->update_animation(dt);
            draw(ring);
            door->update_animation(dt);
            draw(door);
            
            if (startup_anim_time < anim_dur) {
                ring->position = lerp2(DOOR, KEY, (startup_anim_time/anim_dur));
                startup_anim_time = fclamp(0.f, anim_dur, startup_anim_time + dt);
            } else {
                g_scene.startup_state = STATE_SHOW_PLAYER;
                startup_anim_time = 0.f;
            }
            
        } break;
        
        case STATE_SHOW_PLAYER: {
            
            if (startup_anim_time < anim_dur) {
                ring->position = lerp2(KEY, player->position, (startup_anim_time/anim_dur));
                startup_anim_time = fclamp(0.f, anim_dur, startup_anim_time + dt);
            } else {
                g_scene.startup_state = 4;
                ring->mark_for_removal = true;
                g_scene.playing = true;
            }
            
            if (startup_anim_time < anim_dur) {
                ring->update_animation(dt);
                
                float progress = (anim_dur - startup_anim_time + anim_dur/4.f) / anim_dur;
                float radius = (progress) * 128.f;
                radius = MAX(radius, 44.f);
                float phase = ((anim_dur - startup_anim_time) / anim_dur) * 90.f;
                
                for (int i = 0; i < 16; ++i) {
                    Sprite tmp = *ring;
                    
                    float angle = (360.f / 16.f) * i;
                    angle += phase;
                    tmp.position += fvec2{cosf(angle * D2R), sinf(angle * D2R)} * radius;
                    
                    draw(&tmp);
                }
                
                draw(door);
            }
            
        } break;
    }
}

global float key_anim_time = 0.f;
global int key_anim_state = 0;
// rotate the key at first
internal void scene_key_animation(float dt) {
    const int KEYROT = 0;
    const int STAR_DOOR = 1;
    bool finished = false;
    const float anim_dur = 2.f;
    Sprite *key = scene_get_first_sprite(eKey);
    static Sprite effect;
    static Sprite star;
    // update
    const float t = key_anim_time / anim_dur;
    
    key_anim_time += dt;
    if (key_anim_time >= anim_dur) {
        key_anim_time = 0.f;
        finished = true;
    }
    
    switch(key_anim_state) {
        case KEYROT: {
            
            key->rotation = lerp(0.f, 360.f, t);
            fail_unless(key, "da key");
            if (finished) {
                key_anim_state = STAR_DOOR;
                key->mark_for_removal = true;
                
                // initialize next state
                effect = make_effect(key->position, GET_CHAR_ANIM_HANDLE(Effect, Smoke));
                star = make_starring(key->position);
            }
        }break;
        
        case STAR_DOOR: {
            Sprite *door = scene_get_first_sprite(eDoor);
            fail_unless(door, "where is the door???");
            
            if (finished) {
                key_anim_state = KEYROT;
                g_scene.paused_for_key_animation = false;
                
                SET_ANIMATION(door, Door, Open);
            }
            star.update_animation(dt);
            effect.update_animation(dt);
            
            Sprite temp = star;
            temp.position = lerp2(star.position, tile_to_position(g_scene.loaded_map.exit_location),t);
            
            draw(&temp);
            if (!effect.mark_for_removal)
                draw(&effect);
        } break;
    }
    
}



internal void ePlayer_update(Sprite* spref, InputState* istate, float dt);
internal void eGoblin_update(Sprite* spref, InputState* istate, float dt);
internal void eDFireball_update(Sprite* spref, InputState* istate, float dt);
internal void eGhost_update(Sprite* spref, InputState* istate, float dt);
internal void eBlueFlame_update(Sprite* flame, InputState* istate, float dt);
internal void eBlueFlame_cast(Sprite* flame);
internal void player_pickup(Sprite *player, Sprite *pickup);

internal void
scene_update(InputState* istate, float dt) {
    
    for (int i = 0; i < g_scene.loaded_map.pickups.size(); ++i) {
        draw_pickup(&g_scene.loaded_map.pickups[i]);
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
    
    List_Sprite &l = g_scene.loaded_map.sprites;
    for (int i = 0; i < l.size(); ++i) {
        Sprite* spref = &l[i];
        
        if (g_scene.paused_for_key_animation) {
            scene_key_animation(dt);
        } else {
            spref->update_animation(dt);
            switch(spref->entity.type) {
                case ePlayer:{
                    ePlayer_update(spref, istate, dt);
                }break;
                
                case eGoblin:{
                    eGoblin_update(spref, istate, dt);
                } break;
                
                case eEffect:{
                    if (!spref->animation_playing) {
                        spref->mark_for_removal = true;
                    }
                }break;
                
                case eDFireball:{
                    eDFireball_update(spref, istate, dt);
                }break;
                
                case eGhost:{
                    eGhost_update(spref, istate,dt);
                }break;
                
                case eBlueFlame: {
                    eBlueFlame_update(spref, istate,dt);
                }break;
                
                default:
                break;
            }
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
}

