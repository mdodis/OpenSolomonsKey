#define MAX_ENTITY_PARAMS (3 + 4)
// Custom Parameters
// eEmptySpace :: hidden item
// eBlockFrail :: health, hidden item

/* NOTE: eEffect - scene_update _automatically_ marks
 * any eEffect entity for removal once it's animation is finished
*/

union CustomParameter {
    u64    as_u64;
    double as_f64;
    i64    as_i64;
    EnemyType as_etype;
    void *as_ptr;
};

struct Entity {
    EntityType type;
    CustomParameter params[MAX_ENTITY_PARAMS];
};

#include "sprites.h"

#include <vector>
/* Thanks c++ stl! Now this'll speed up my productivity */
typedef std::vector<Sprite> List_Sprite;

struct Map {
    const char *name; // path!!!
    EntityType tiles[TILEMAP_COLS][TILEMAP_ROWS];
    int hidden_pickups[TILEMAP_COLS][TILEMAP_ROWS];
    
    List_Sprite sprites;
    List_Sprite pickups;
    
    ivec2 exit_location;
    ivec2 key_location;
};

enum SceneState {
    SS_MENU,
    SS_STARTUP,
    SS_PLAYING,
    SS_WIN,
    SS_LOSE,
};

global struct {
    Map loaded_map;
    b32 playing = false;
    b32 paused_for_key_animation = false;
    int startup_state = 0;
    List_Sprite sprite_buffer;
    int current_state = SS_MENU;
    
    long player_score = 0;
    float last_score_timer = 0.f;
    u32 last_score_num = 0u;
    
    float player_time = 80.f;
    int player_lives = 3;
    bool time_is_low_enough = false;
    i32 current_level_counter = -1;
    bool player_is_dead = false;
} g_scene;

inline internal Sprite make_effect(fvec2 position, u32 effect_type) {
    return Sprite {
        .tilemap = &GET_CHAR_TILEMAP(Effect),
        .position = position,
        .collision_box = {0,0,64,64},
        .current_animation = effect_type,
        .animation_set = GET_CHAR_ANIMSET(Effect),
        .entity = {
            .type = ET_Effect,
        }
    };
}

inline internal Sprite make_goblin(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Goblin),
        .position = position,
        .collision_box = {5, 0, 45, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Goblin),
        .entity = {
            .type = ET_Enemy,
        }
    };
    res.entity.params[0].as_etype = MT_Goblin;
    
    return res;
}

inline internal Sprite make_demon_head(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(DemonHead),
        .position = position,
        .collision_box = {5,0,45,64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(DemonHead),
        .entity = {
            .type = ET_Enemy,
        }
    };
    res.entity.params[0].as_etype = MT_Demonhead;
    
    return res;
}

inline internal Sprite make_ghost(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Ghost),
        .position = position,
        .collision_box = {0, 0, 64, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Ghost),
        .entity = {
            .type = ET_Enemy,
        }
    };
    
    res.entity.params[0].as_etype = MT_Ghost;
    
    return res;
}

inline internal Sprite make_wyvern(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Wyvern),
        .position = position,
        .collision_box = {0, 0, 64, 64},
        .mirror = {false, false},
        .animation_set = GET_CHAR_ANIMSET(Wyvern),
        .entity = {
            .type = ET_Enemy
        }
    };
    
    res.entity.params[0].as_etype = MT_Wyvern;
    return res;
}

inline internal Sprite make_player(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Dana),
        .size = fvec2{64, 64},
        .position = position,
        .collision_box = {5, 14, 45, 50},
        .mirror = {true, false},
        .mark_for_removal = false,
        
        .animation_set = GET_CHAR_ANIMSET(Dana),
        .entity = {
            ET_Player,
            {0,0}
        }
    };
    
    return res;
}

inline internal Sprite make_door(fvec2 position) {
    return Sprite {
        .tilemap = &GET_CHAR_TILEMAP(Door),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .current_animation = GET_CHAR_ANIMENUM(Door, Close),
        .animation_set = GET_CHAR_ANIMSET(Door),
        .entity = {
            ET_Door,
            {0,0}
        }
    };
}

inline internal Sprite make_dfireball(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(DFireball),
        .size = {55,55},
        .position = position,
        .collision_box = {8,8,40,40},
        .mirror = {true, false},
        .animation_set = GET_CHAR_ANIMSET(DFireball),
        .entity = {
            ET_DFireball,
            {0,0}
        }
    };
    
    SET_ANIMATION(&res, DFireball, Middle);
    return res;
}


inline internal Sprite make_starring(fvec2 position) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Effect),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .current_animation = GET_CHAR_ANIMENUM(Effect, Star),
        .animation_set = GET_CHAR_ANIMSET(Effect),
        .entity = {
            ET_Count,
            {u64(0.f),u64(0.f)}
        }
    };
    
    return res;
    
}

inline internal Sprite make_fairie(fvec2 position, u64 type) {
    Sprite res = {
        .tilemap = &GET_CHAR_TILEMAP(Fairie),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        
        // type will go here!!!! and not in custom params
        .current_animation = GET_CHAR_ANIMENUM(Fairie, Default),
        .animation_set = GET_CHAR_ANIMSET(Fairie),
        .entity = {
            ET_Fairie,
            {0,0}
        }
    };
    
    return res;
}

inline internal Sprite make_key(fvec2 position) {
    Sprite res =
    {
        .tilemap = &GET_CHAR_TILEMAP(Key),
        .size = {64,64},
        .position = position,
        .rotation = 0.f,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        
        .animation_playing = false,
        .current_animation = 0,
        .animation_set = GET_CHAR_ANIMSET(Key),
        
        .entity = {
            ET_Key,
            {u64(0.f),u64(0.f)}
        }
    };
    
    return res;
    
}

inline internal Sprite make_pickup(fvec2 position, u64 type, u64 id = 0) {
    Sprite res = {
        
        .tilemap = &GET_TILEMAP_TEXTURE(TM_pickups),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .mirror = {false, false},
        .animation_playing = false,
        .current_animation = 0,
        .animation_set = 0,
        .entity = {
            ET_Pickup,
            {type, 0}
        }
    };
    
    res.entity.params[2].as_u64 = id;
    
    return res;
}

inline internal Sprite make_blueflame(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(BlueFlame),
        .size = {64,64},
        .position = position,
        .collision_box = {16, 0, 32, 64},
        .animation_playing = true,
        .current_animation = GET_CHAR_ANIMENUM(BlueFlame, Normal),
        .animation_set = GET_CHAR_ANIMSET(BlueFlame),
        .entity = {
            ET_Enemy,
            {0, 0, 0}
        }
    };
    
    result.entity.params[0].as_etype = MT_BlueFlame;
    result.entity.params[1].as_f64 = +1.0;
    result.entity.params[2].as_f64 = -FLT_MAX;
    
    return result;
}

inline internal Sprite make_kmirror(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(KMirror),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .animation_playing = false,
        .current_animation = GET_CHAR_ANIMENUM(KMirror, Default),
        .animation_set = GET_CHAR_ANIMSET(KMirror),
        .entity = {
            ET_Enemy,
            {0,0,0, 0, 0,}
        }
    };
    
    result.entity.params[0].as_etype = MT_KMirror;
    result.entity.params[1].as_f64 = 0.f;
    result.entity.params[2].as_f64 = 0.f;
    result.entity.params[3].as_f64 = 1.f;
    result.entity.params[4].as_f64 = 1.f;
    result.entity.params[5].as_ptr = 0;
    result.entity.params[6].as_ptr = 0;
    
    return result;
}

inline internal Sprite make_panel_monster(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(PanelMonster),
        .size = {64,64},
        .position = position,
        .collision_box = {0,0,64,64},
        .animation_playing = true,
        .current_animation = GET_CHAR_ANIMENUM(PanelMonster, Wait),
        .animation_set = GET_CHAR_ANIMSET(PanelMonster),
        .entity = {
            ET_Enemy,
            {0,0,0, 0, 0,}
        }
    };
    
    result.entity.params[0].as_etype = MT_PanelMonster;
    return result;
}

inline internal Sprite make_panel_monster_flame(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(PanelMonsterFlame),
        .size = {64,64},
        .position = position,
        .collision_box = {10,16,42,32},
        .mirror = {true, false},
        .animation_playing = true,
        .current_animation = GET_CHAR_ANIMENUM(PanelMonsterFlame, Create),
        .animation_set = GET_CHAR_ANIMSET(PanelMonsterFlame),
        .entity = {
            ET_Enemy,
            {0,0,0,0,0,}
        }
    };
    
    result.entity.params[0].as_etype = MT_PanelMonsterFlame;
    return result;
}

inline internal Sprite make_dragon(fvec2 position) {
    Sprite result = {
        .tilemap = &GET_CHAR_TILEMAP(Dragon),
        .size = {64,64},
        .position = position,
        .collision_box = {10,16,42,32},
        .mirror = {true, false},
        .animation_playing = true,
        .current_animation = GET_CHAR_ANIMENUM(Dragon, Walk),
        .animation_set = GET_CHAR_ANIMSET(Dragon),
        .entity = {
            ET_Enemy,
            {0,0,0,0,0,}
        }
    };
    result.entity.params[0].as_etype = MT_Dragon;
    return result;
}

internal void draw(Sprite const * sprite) {
    
    if (!sprite->tilemap) {
        exit_error("tilemap %d !!", sprite->entity.type);
    }
    
    i32 frame_to_render = sprite->current_frame;
    
    if (sprite->animation_set) {
        Animation* anim_ref = &sprite->animation_set[sprite->current_animation];
        frame_to_render =
            anim_ref->start.y * sprite->tilemap->cols
            + anim_ref->start.x + sprite->current_frame;
    }
    
    
    gl_slow_tilemap_draw(sprite->tilemap,
                         {(float)sprite->position.x, (float)sprite->position.y},
                         {(float)sprite->size.x, (float)sprite->size.y},
                         sprite->rotation,
                         frame_to_render,
                         sprite->mirror.x, sprite->mirror.y);
}

internal void
draw_pickup(Sprite const *sprite) {
    assert(sprite->tilemap);
    PickupType type = (PickupType)sprite->entity.params[0].as_u64;
    
    u64 index_to_draw;
    index_to_draw = sprite->entity.params[0].as_u64;
    
    if (sprite->entity.params[1].as_u64 == 0) {
        gl_slow_tilemap_draw(sprite->tilemap,
                             {(float)sprite->position.x, (float)sprite->position.y},
                             {(float)sprite->size.x, (float)sprite->size.y},
                             sprite->rotation,
                             index_to_draw,
                             sprite->mirror.x, sprite->mirror.y);
    }
}

