
global float startup_anim_time = 0.f;
internal void startup_animation_reset() {
    g_scene.startup_state = 0;
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
    
    const float anim_dur = 1.5f;
    
    static Sprite ring_static;
    Sprite *ring;
    
    ring = &ring_static;
    
    const Sprite *const player = find_first_sprite(ET_Player);
    Sprite *door = find_first_sprite(ET_Door);
    assert(door);
    // placeholder
    const fvec2 DOOR = tile_to_position(g_scene.loaded_map.exit_location);
    const fvec2 KEY = tile_to_position(g_scene.loaded_map.key_location);
    
    if (GET_KEYPRESS(space_pressed)) {
        goto STARTUP_ANIMATION_FINISH;
    }
    
    switch (g_scene.startup_state) {
        
        case STATE_START: {
            ring_static = make_starring(DOOR);
            g_scene.startup_state = STATE_SHOW_KEY;
            audio_play_sound(GET_SOUND(SND_show_key));
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
                goto STARTUP_ANIMATION_FINISH;
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
    return;
    
    
    STARTUP_ANIMATION_FINISH:
    startup_animation_reset();
    g_scene.current_state = SS_PLAYING;
    g_scene.playing = true;
    audio_start(SoundType::Music);
    
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
            key->rotation = lerp(0.f, 360.f, t);
            fail_unless(key, "da key");
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
    win_player_time = long(g_scene.player_time * 100);
    audio_play_sound(GET_SOUND(SND_win), false, SoundType::Music);
}

internal void finish_win_animation() {
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
