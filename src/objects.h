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
    
    int player_num_fireballs = 0;
    int player_num_slots = 3;
    int player_fireball_range = 2; // number of blocks
    
    bool player_will_get_double_rest_bonus = false;
    bool time_is_low_enough = false;
#ifndef NDEBUG
    i32 current_level_counter = -1;
#else
    i32 current_level_counter = 0;
#endif
} g_scene;

inline internal Sprite make_effect(fvec2 position, u32 effect_type) {
    Sprite result;
    result.tilemap = &GET_CHAR_TILEMAP(Effect);
    result.position = position;
    result.collision_box = {0,0,64,64};
    result.current_animation = effect_type;
    result.animation_set = GET_CHAR_ANIMSET(Effect);
    result.entity.type = ET_Effect;
    return result;
}

inline internal Sprite make_goblin(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Goblin);
    res.position = position;
    res.collision_box = {5, 0, 45, 64};
    res.mirror = {false, false};
    res.animation_set = GET_CHAR_ANIMSET(Goblin);
    res.entity.type = ET_Enemy;
    res.entity.params[0].as_etype = MT_Goblin;
    /*
param 0 etype
param 1 walk speed
param 2 run speed
param 3 spawn/fall state
*/
    res.entity.params[3].as_u64 = true;
    return res;
}

inline internal Sprite make_demon_head(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(DemonHead);
    res.position = position;
    res.collision_box = {5,0,45,64};
    res.mirror = {false, false};
    res.animation_set = GET_CHAR_ANIMSET(DemonHead);
    res.entity.type = ET_Enemy;
    res.entity.params[0].as_etype = MT_Demonhead;
    
    return res;
}

inline internal Sprite make_ghost(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Ghost);
    res.position = position;
    res.collision_box = {0, 0, 64, 64};
    res.mirror = {false, false};
    res.animation_set = GET_CHAR_ANIMSET(Ghost);
    res.entity.type = ET_Enemy;
    
    res.entity.params[0].as_etype = MT_Ghost;
    
    return res;
}

inline internal Sprite make_wyvern(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Wyvern);
    res.position = position;
    res.collision_box = {10, 0, 54, 64};
    res.mirror = {false, false};
    res.animation_set = GET_CHAR_ANIMSET(Wyvern);
    res.entity.type = ET_Enemy;
    
    res.entity.params[0].as_etype = MT_Wyvern;
    return res;
}


internal bool player_add_fireball() {
    if (g_scene.player_num_fireballs + 1 <= g_scene.player_num_slots) {
        g_scene.player_num_fireballs++;
        return true;
    } else {
        return false;
    }
}

inline internal Sprite make_player(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Dana);
    res.size = fvec2{64, 64};
    res.position = position;
    res.collision_box = {5, 14, 45, 50};
    res.mirror = {true, false};
    res.mark_for_removal = false;
    
    res.animation_set = GET_CHAR_ANIMSET(Dana);
    res.entity = {
        ET_Player,
        {0,0}
    };
    
    return res;
}

inline internal Sprite make_door(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Door);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.mirror = {false, false};
    res.current_animation = GET_CHAR_ANIMENUM(Door, Close);
    res.animation_set = GET_CHAR_ANIMSET(Door);
    res.entity = {
        ET_Door,
        {0,0}
    };
    return res;
}

inline internal Sprite make_dfireball(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(DFireball);
    res.size = {55,55};
    res.position = position;
    res.collision_box = {8,8,40,40};
    res.mirror = {true, false};
    res.animation_set = GET_CHAR_ANIMSET(DFireball);
    
    res.entity = {
        ET_DFireball,
        {0,0}
    };
    res.entity.params[0].as_f64 = 1.0;
    res.entity.params[1].as_f64 = 0.0;
    
    SET_ANIMATION(&res, DFireball, Middle);
    return res;
}


inline internal Sprite make_starring(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Effect);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.mirror = {false, false};
    res.current_animation = GET_CHAR_ANIMENUM(Effect, Star);
    res.animation_set = GET_CHAR_ANIMSET(Effect);
    
    res.entity = {
        ET_Count,
        {u64(0.f),u64(0.f)}
    };
    
    return res;
    
}

inline internal Sprite make_fairie(fvec2 position, long type) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Fairie);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.mirror = {false, false};
    
    // type will go here!!!! and not in custom params
    res.current_animation = u32(type);
    res.animation_set = GET_CHAR_ANIMSET(Fairie);
    
    res.entity = {
        ET_Fairie,
        {0,0}
    };
    
    return res;
}

inline internal Sprite make_key(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Key);
    res.size = {64,64};
    res.position = position;
    res.rotation = 0.f;
    res.collision_box = {0,0,64,64};
    res.mirror = {false, false};
    
    res.animation_playing = false;
    res.current_animation = 0;
    res.animation_set = GET_CHAR_ANIMSET(Key);
    
    
    res.entity = {
        ET_Key,
        {u64(0.f),u64(0.f)}
    };
    
    return res;
    
}

internal void set_pickup_collision_box(Sprite *pickup) {
    pickup->collision_box = AABox{10,10,54,54};
}

inline internal Sprite make_pickup(fvec2 position, u64 type, u64 id = 0) {
    Sprite res;
    res.tilemap = &GET_TILEMAP_TEXTURE(TM_pickups);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.mirror = {false, false};
    res.animation_playing = false;
    res.current_animation = 0;
    res.animation_set = 0;
    
    res.entity = {
        ET_Pickup,
        {type, 0, 0, 0, 0}
    };
    set_pickup_collision_box(&res);
    /*
param 0 is te pickup's type
param 1 controls whether the pickup is "enabled" or not TODO: move this to Sprite::enabled
param 2 is the pickup's id, to be able to enable/disable it if it's hidden
param 3 controls whether or not the pickup can move (gravity)
param 4 states the pickup's original type (see Player_cast)
*/
    res.entity.params[2].as_u64 = id;
    res.entity.params[3].as_i64 = 0;
    return res;
}

internal void pickup_change_to(Sprite *pickup, PickupType to) {
    PickupType &current_type = (PickupType&)pickup->entity.params[0].as_u64;
    PickupType &original_type = (PickupType&)pickup->entity.params[4].as_u64;
    
    if (current_type == PT_JewelChange) {
        original_type = PT_JewelChange;
    }
    
    if (original_type == PT_JewelChange) {
        current_type = to;
    }
}

inline internal Sprite make_blueflame(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(BlueFlame);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {16, 16, 32, 48};
    res.animation_playing = true;
    res.current_animation = GET_CHAR_ANIMENUM(BlueFlame, Normal);
    res.animation_set = GET_CHAR_ANIMSET(BlueFlame);
    
    res.entity = {
        ET_Enemy,
        {0, 0, 0}
    };
    
    res.entity.params[0].as_etype = MT_BlueFlame;
    res.entity.params[1].as_f64 = +1.0;
    res.entity.params[2].as_f64 = -FLT_MAX;
    
    return res;
}

inline internal Sprite make_kmirror(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(KMirror);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.animation_playing = false;
    res.current_animation = GET_CHAR_ANIMENUM(KMirror, Default);
    res.animation_set = GET_CHAR_ANIMSET(KMirror);
    
    res.entity = {
        ET_Enemy,
        {0,0,0, 0, 0,}
    };
    
    res.entity.params[0].as_etype = MT_KMirror;
    res.entity.params[1].as_f64 = 0.f;
    res.entity.params[2].as_f64 = 0.f;
    res.entity.params[3].as_f64 = 1.f;
    res.entity.params[4].as_f64 = 1.f;
    res.entity.params[5].as_ptr = 0;
    res.entity.params[6].as_ptr = 0;
    
    return res;
}

inline internal Sprite make_panel_monster(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(PanelMonster);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.animation_playing = true;
    res.current_animation = GET_CHAR_ANIMENUM(PanelMonster, Wait);
    res.animation_set = GET_CHAR_ANIMSET(PanelMonster);
    
    res.entity = {
        ET_Enemy,
        {0,0,0, 0, 0,}
    };
    
    res.entity.params[0].as_etype = MT_PanelMonster;
    return res;
}

inline internal Sprite make_panel_monster_flame(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(PanelMonsterFlame);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {10,16,42,32};
    res.mirror = {true, false};
    res.animation_playing = true;
    res.current_animation = GET_CHAR_ANIMENUM(PanelMonsterFlame, Create);
    res.animation_set = GET_CHAR_ANIMSET(PanelMonsterFlame);
    
    res.entity = {
        ET_Enemy,
        {0,0,0,0,0,}
    };
    
    res.entity.params[0].as_etype = MT_PanelMonsterFlame;
    return res;
}

inline internal Sprite make_dragon(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Dragon);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {10,0,54,64};
    res.mirror = {true, false};
    res.animation_playing = true;
    res.current_animation = GET_CHAR_ANIMENUM(Dragon, Walk);
    res.animation_set = GET_CHAR_ANIMSET(Dragon);
    
    res.entity = {
        ET_Enemy,
        {0,0,0,0,0,}
    };
    res.entity.params[0].as_etype = MT_Dragon;
    return res;
}

inline internal Sprite make_dragon_fire(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Dragon);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {0,0,64,64};
    res.mirror = {false, false};
    res.animation_playing = true;
    res.current_animation = GET_CHAR_ANIMENUM(DragonFire, Default);
    res.animation_set = GET_CHAR_ANIMSET(DragonFire);
    
    res.entity = {
        ET_Enemy,
        {0,0,0,0,0,}
    };
    res.entity.params[0].as_etype = MT_DragonFire;
    return res;
}

inline internal Sprite make_spark_ball(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(SparkBall);
    res.size = {55,55};
    res.position = position;
    res.collision_box = {8,8,40,40};
    res.mirror = {false, false};
    res.animation_set = GET_CHAR_ANIMSET(SparkBall);
    
    res.entity = {
        ET_Enemy,
        {0,0}
    };
    
    res.entity.params[0].as_etype = MT_SparkBall;
    return res;
}

inline internal Sprite make_gargoyle(fvec2 position) {
    Sprite res;
    res.tilemap = &GET_CHAR_TILEMAP(Gargoyle);
    res.size = {64,64};
    res.position = position;
    res.collision_box = {8,0,40,64};
    res.mirror = {false, false};
    res.animation_set = GET_CHAR_ANIMSET(Gargoyle);
    
    res.entity = {
        ET_Enemy,
        {0,0}
    };
    /*
1: speed
2: fire time
3: fire timer
*/
    res.current_animation = GET_CHAR_ANIMENUM(Gargoyle, Walk);
    res.entity.params[0].as_etype = MT_Gargoyle;
    res.entity.params[1].as_f64 = 80.f;
    res.entity.params[2].as_f64 = 2.f;
    res.entity.params[3].as_f64 = 0.f;
    return res;
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

