
internal void add_score(u32 amount) {
    g_scene.last_score_timer = 0.f;
    g_scene.last_score_num = amount;
    
    g_scene.player_score += amount;
}

internal void draw_score(float dt) {
    
    draw_num(g_scene.player_score, 0, 5 , false, 32, true);
    
    if (g_scene.last_score_timer < LAST_SCORE_TIME) {
        // draw last score
        NRGBA tint = {
            (sinf(g_scene.last_score_timer)+ 1.f)*.5f,
            (cosf(g_scene.last_score_timer)+ 1.f)*.5f,
            1,
            1
        };
        
        //draw_num(g_scene.last_score_num, 1, 5 , false, 32, true, tint);
        g_scene.last_score_timer += dt;
    }
    
}