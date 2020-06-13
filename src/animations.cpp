internal void scene_update(InputState* istate, float dt);
internal void reload_map();
internal void load_next_map();
internal void dim_map();

global float startup_anim_time = 0.f;
global bool startup_anim_include_key = 0;
internal void startup_animation_reset() {
    g_scene.startup_state = 0;
    startup_anim_time = 0.f;
    startup_anim_include_key = false;
}

// One star from door to key
// show key
// Circle star from key to player
internal void scene_startup_animation(float dt) {
    
    const int STATE_START = 0;
    const int STATE_SHOW_KEY = 1;
    const int STATE_SHOW_PLAYER = 2;
    const int STATE_DONE = 3;
    const int STATE_HAD_KEY = 4;
    const int STATE_HAD_KEY_SHOW_DOOR = 5;
    const float anim_dur = 1.5f;
    
    static Sprite ring_static;
    Sprite *ring;
    
    ring = &ring_static;
    
    Sprite *const player = find_first_sprite(ET_Player);
    Sprite *door = find_first_sprite(ET_Door);
    assert(door);
    // placeholder
    const fvec2 DOOR = tile_to_position(g_scene.loaded_map.exit_location);
    const fvec2 KEY = tile_to_position(g_scene.loaded_map.key_location);
    
    if (GET_KEYPRESS(space_pressed)) {
        goto STARTUP_ANIMATION_FINISH;
    }
    static Sprite key = make_key(player->position);
    
    dim_map();
    switch (g_scene.startup_state) {
        
        case STATE_START: {
            ring_static = make_starring(DOOR);
            g_scene.startup_state = STATE_SHOW_KEY;
            audio_play_sound(GET_SOUND(SND_show_key));
            
            if (startup_anim_include_key) g_scene.startup_state = STATE_SHOW_PLAYER;
        } break;
        
        case STATE_SHOW_KEY: {
            
            ring->update_animation(dt);
            draw(ring);
            door->update_animation(dt);
            draw(door);
            
            if (startup_anim_time < anim_dur) {
                ring->position = lerp2(DOOR, KEY, (startup_anim_time/anim_dur));
                startup_anim_time = clamp(0.f, anim_dur, startup_anim_time + dt);
            } else {
                g_scene.startup_state = STATE_SHOW_PLAYER;
                startup_anim_time = 0.f;
                audio_play_sound(GET_SOUND(SND_show_player));
            }
            
        } break;
        
        case STATE_SHOW_PLAYER: {
            
            if (startup_anim_time < anim_dur) {
                ring->position = lerp2(KEY, player->position, (startup_anim_time/anim_dur));
                startup_anim_time = clamp(0.f, anim_dur, startup_anim_time + dt);
            } else {
                if (startup_anim_include_key) {
                    startup_anim_time = 0.f;
                    g_scene.startup_state = STATE_HAD_KEY;
                } else {
                    goto STARTUP_ANIMATION_FINISH;
                }
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
        
        case STATE_HAD_KEY: {
            const float dur = 1.f;
            startup_anim_time += dt;
            if (startup_anim_time >= dur) {
                startup_anim_time = 0.f;
                ring->position = player->position;
                
                g_scene.startup_state = STATE_HAD_KEY_SHOW_DOOR;
            }
            
            const float t = startup_anim_time / dur;
            key.rotation = lerp(0.f, 360.f, t);
            
            draw(player);
            draw(&key);
        }break;
        
        case STATE_HAD_KEY_SHOW_DOOR: {
            Sprite *door = find_first_sprite(ET_Door);
            assert(door);
            const float dur = 1.f;
            startup_anim_time += dt;
            if (startup_anim_time >= dur) {
                Sprite *key = find_first_sprite(ET_Key);
                Sprite *door = find_first_sprite(ET_Door);
                key->mark_for_removal = true;
                player->entity.params[0].as_u64 = 1;
                SET_ANIMATION(door, Door, Open);
                goto STARTUP_ANIMATION_FINISH;
            }
            ring->update_animation(dt);
            const float t = startup_anim_time / dur;
            
            ring->position = lerp2(player->position, door->position, t);
            
            draw(player);
            draw(ring);
            
        }break;
        
    }
    return;
    
    
    STARTUP_ANIMATION_FINISH:
    startup_animation_reset();
    g_scene.current_state = SS_PLAYING;
    g_scene.playing = true;
    audio_start(SoundType::Music);
    
    g_scene.player_lives -= 1;
    assert(g_scene.player_lives >= 0);
    
    return;
}

global float key_anim_time = 0.f;
global int key_anim_state = 0;
internal void play_key_get_animation() {
    key_anim_state = 0;
    key_anim_time = 0.f;
    g_scene.paused_for_key_animation = true;
    audio_stop(SoundType::Music);
    
    audio_play_sound(GET_SOUND(SND_get_key));
}

internal void finish_key_get_animation() {
    key_anim_state = 0;
    g_scene.paused_for_key_animation = false;
    audio_resume(SoundType::Music);
}

// rotate the key at first
internal void scene_key_animation(float dt) {
    const int KEYROT = 0;
    const int STAR_DOOR = 1;
    bool finished = false;
    const float anim_dur = 1.f;
    Sprite *key = find_first_sprite(ET_Key);
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
            //fail_unless(key, "da key");
            key->rotation = lerp(0.f, 360.f, t);
            if (finished) {
                key_anim_state = STAR_DOOR;
                key->mark_for_removal = true;
                audio_play_sound(GET_SOUND(SND_show_key));
                // initialize next state
                effect = make_effect(key->position, GET_CHAR_ANIM_HANDLE(Effect, Smoke));
                star = make_starring(key->position);
            }
        }break;
        
        case STAR_DOOR: {
            Sprite *door = find_first_sprite(ET_Door);
            fail_unless(door, "where is the door???");
            
            if (finished) {
                finish_key_get_animation();
                
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

global float win_animation_timer = 0.f;
global int   win_animation_state = 0;
global long  win_player_time = 0;
internal void play_win_animation() {
    win_animation_timer = 0.f;
    win_animation_state = 0;
    assert(g_scene.current_state == SS_PLAYING);
    g_scene.current_state = SS_WIN;
    audio_remove(SoundType::Music);
    win_player_time = long(g_scene.player_time * 100.f);
    
    audio_play_sound(GET_SOUND(SND_win), false, SoundType::Music);
}

internal void finish_win_animation() {
    g_scene.player_lives += 1;
    load_next_map();
}

internal void scene_win_animation(float dt) {
    const int STATE_ENTER = 0;
    const int STATE_STAR_RING = 1;
    const int STATE_SHOW_SCORE = 2;
    static Sprite ring_static;
    Sprite *ring = &ring_static;
    
    const fvec2 DOOR = tile_to_position(g_scene.loaded_map.exit_location);
    const fvec2 DEST = fvec2{(8.5f*64.f), (6.f*64.f)};
    
    switch(win_animation_state) {
        case STATE_ENTER: {
            ring_static = make_starring(DOOR);
            win_animation_state = STATE_STAR_RING;
        }break;
        
        case STATE_STAR_RING: {
            const float dur = 1.5f;
            win_animation_timer += dt;
            if (win_animation_timer > dur) {
                win_animation_timer = 0.f;
                win_animation_state = STATE_SHOW_SCORE;
                audio_remove(SoundType::Music);
                audio_play_sound(GET_SOUND(SND_rest_bonus));
                add_score(win_player_time);
            }
            
            float t = win_animation_timer/dur;
            ring->position = lerp2(DOOR, DEST, t);
            
            float radius = (t) * 128.f;
            radius = MAX(radius, 44.f);
            float phase = (t) * 90.f;
            
            scene_draw_tilemap();
            for (int i = 0; i < 16; ++i) {
                Sprite tmp = *ring;
                
                float angle = (360.f / 16.f) * i;
                angle += phase;
                tmp.position += fvec2{cosf(angle * D2R), sinf(angle * D2R)} * radius;
                
                draw(&tmp);
            }
        }break;
        
        case STATE_SHOW_SCORE: {
            const float dur = 2.5f;
            win_animation_timer += dt;
            
            if (win_animation_timer >= dur) {
                finish_win_animation();
            }
            
            draw_text("% Your Rest Bonus %", 7, 3, false, 40.f);
            draw_num_long(win_player_time, 9, 9, false, 40.f, false, NRGBA{1.f,0.5f,1.f,1.f});
            draw_text("PTS", 15, 23, false, 24.f, NRGBA{0.8f, 1.f, 1.f, 1.f});
        }break;
    }
}

float lose_animation_timer = 0.f;
int lose_animation_state = 0;
internal void play_lose_animation(Sprite *player) {
    startup_anim_include_key = player->entity.params[0].as_u64 == 1;
    lose_animation_state = 0;
    lose_animation_timer = 0.f;
    g_scene.current_state = SS_LOSE;
    g_scene.paused_for_key_animation = true;
    audio_remove_all();
}

internal void finish_lose_animation(Sprite *player) {
    reload_map();
    g_scene.current_state = SS_STARTUP;
    g_scene.paused_for_key_animation = false;
    
    if (g_scene.player_lives == 0) {
        // TODO(miked): actually implement game over screen
        puts("GAME OVER");
        exit(0);
    }
}

internal void scene_lose_animation(float dt) {
    Sprite *player = find_first_sprite(ET_Player);
    
    const int STATE_ENTRY = 0;
    const int STATE_PLAYER_DIE = 1;
    const int STATE_SPARKLES = 2;
    
    static Sprite sparkles = make_effect(player->position, GET_CHAR_ANIM_HANDLE(Effect, DanaDie));
    
    switch (lose_animation_state) {
        
        case STATE_ENTRY: {
            SET_ANIMATION(player, Dana, Die);
            lose_animation_state = STATE_PLAYER_DIE;
        }break;
        
        case STATE_PLAYER_DIE: {
            const float dur = 1.f;
            lose_animation_timer += dt;
            if (lose_animation_timer >= dur) {
                player->mark_for_removal = true;
                lose_animation_state = STATE_SPARKLES;
                lose_animation_timer = 0.f;
            }
            
            player->update_animation(dt);
            scene_update(0, 0.f);
        }break;
        
        case STATE_SPARKLES: {
            const float dur = 1.f;
            lose_animation_timer += dt;
            
            if (lose_animation_timer >= dur) {
                finish_lose_animation(player);
            }
            
            sparkles.update_animation(dt);
            scene_update(0, 0.f);
        }break;
        
        default: assert(false);
    }
    
}
