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

internal Sprite *scene_sprite_add(const Sprite *sprite)
{
    fail_unless(sprite, "Passing null sprite to scene_add");
    
    if (sprite->entity.type == ET_Pickup) {
        g_scene.loaded_map.pickups.push_back(*sprite);
        return &g_scene.loaded_map.pickups.back();
    } else {
        g_scene.loaded_map.sprites.push_back(*sprite);
        return &g_scene.loaded_map.sprites.back();
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


inline internal bool is_frail_block(EntityType type) {
    return (type == ET_BlockFrail || type == ET_BlockFrailHalf);
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

#include "animations.cpp"

internal void scene_update(InputState* istate, float dt) {
    
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
    
    if (g_scene.paused_for_key_animation)
        scene_key_animation(dt);
    
    List_Sprite &l = g_scene.loaded_map.sprites;
    for (int i = 0; i < l.size(); ++i) {
        Sprite* spref = &l[i];
        
        if (!g_scene.paused_for_key_animation) {
            spref->update_animation(dt);
            switch(spref->entity.type) {
                case ET_Player:{
                    ePlayer_update(spref, istate, dt);
                }break;
                
                case ET_Enemy: {
                    
                    switch(spref->entity.params[0].as_etype) {
                        case MT_Goblin: {
                            Goblin_update(spref, istate, dt);
                        }break;
                        
                        case MT_Ghost: {
                            Ghost_update(spref, istate,dt);
                        }break;
                        
                        case MT_BlueFlame: {
                            BlueFlame_update(spref, istate,dt);
                        }break;
                        
                        case MT_KMirror: {
                            KMirror_update(spref, istate,dt);
                        }break;
                        
                    }
                } break;
                
                case ET_Effect:{
                    if (!spref->animation_playing) {
                        spref->mark_for_removal = true;
                    }
                }break;
                
                case ET_DFireball:{
                    eDFireball_update(spref, istate, dt);
                }break;
                
                case ET_Fairie: {
                    eFairie_update(spref, istate,dt);
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
    
    if (!g_scene.paused_for_key_animation) {
        g_scene.player_time -= dt;
        
        if ((g_scene.player_time * 100) < 2000u && !g_scene.time_is_low_enough) {
            audio_remove(SoundType::Music);
            audio_play_sound(GET_SOUND(SND_hurry), false, SoundType::Music);
            g_scene.time_is_low_enough = true;
            
        }
    }
}

