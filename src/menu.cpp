
int extract_file_level_number(char *str, char **out_end) {
    
    // level_<num>
    char *end;
    long int result = strtol(str + (6), &end, 10);
    
    if (out_end) *out_end = end;
    return result;
}

global int current_menu_item = 0;
global float menu_timer = 0.f;
enum MenuState {
    MENU_MAIN,
    MENU_SELECT
};
global int current_menu_state = MENU_MAIN;
global int select_menu_line_offset = 0;
global int select_menu_current_level = 0;
void scene_menu(float dt) {
    float sr = 1.f;
    float sg = 1.f;
    float sb = 1.f;
    
    menu_timer += dt * 4.f;
    
    sr = (sinf(menu_timer) + 1.f) / 4.f + (cosf(menu_timer) + 1.f) / 4.f;
    sg = (sinf(menu_timer + M_PI * 2) + 1.f) / 4.f + (cosf(menu_timer + M_PI) + 1.f) / 4.f;
    sb = (sinf(menu_timer - M_PI) + 1.f) / 4.f + (cosf(menu_timer - M_PI * 2) + 1.f) / 4.f;
    
    if (current_menu_state == MENU_MAIN) {
        
        const int MENU_START  = 0;
        const int MENU_SELECT = 1;
        const int MENU_QUIT   = 2;
        
        
        if (GET_KEYPRESS(move_down_menu)) current_menu_item++;
        if (GET_KEYPRESS(move_up_menu)) current_menu_item--;
        current_menu_item = clamp(0, 2, current_menu_item);
        
        if (GET_KEYPRESS(space_pressed)) {
            if (current_menu_item == MENU_START) {
                load_next_map();
            } else if (current_menu_item == MENU_SELECT) {
                current_menu_state = MENU_SELECT;
            } else if (current_menu_item == MENU_QUIT) {
                exit(0);
            }
        }
        
        // render
        glClearColor( 0.0, 0.0,  0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        draw_extra_stuff();
        
        gl_background_draw();
        
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_logo), fvec2{32.f*6.0f,32.f*3}, fvec2{644.f,352.f}, 0.f, 0, false, false, NRGBA{1,1,1,1}, false);
        
        NRGBA sel_color = NRGBA{sr,sg,sb,1};
        NRGBA norm = NRGBA{1,1,1,1};
        
        gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(TM_demonhead), fvec2{128.f * 2, 128.f * current_menu_item + 128.f * 3}, fvec2{64.f,64.f}, 0.f, 0);
        
        draw_text("Start",6,5,true, 64.f, current_menu_item == MENU_START ? sel_color:norm);
        draw_text("Select",8,5,true, 64.f, current_menu_item == MENU_SELECT ? sel_color:norm);
        draw_text("Quit",10,5,true, 64.f, current_menu_item == MENU_QUIT ? sel_color:norm);
    } else if (current_menu_state == MENU_SELECT) {
        
        if (GET_KEYPRESS(move_down_menu)) select_menu_current_level++;
        if (GET_KEYPRESS(move_up_menu)) select_menu_current_level--;
        if (GET_KEYPRESS(cast)) current_menu_state = MENU_MAIN;
        select_menu_current_level = clamp(0, g_map_list.size() - 1, select_menu_current_level);
        
        if (GET_KEYPRESS(space_pressed)) {
            int lvl = extract_file_level_number(g_map_list[select_menu_current_level], 0);
            g_scene.current_level_counter = lvl - 1;
            current_menu_state = MENU_MAIN;
            load_next_map();
        }
        
        // render
        glClearColor( 0.0, 0.0,  0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        draw_extra_stuff();
        gl_background_draw();
        
        int i = 0;
        while (i < g_map_list.size() && (i < 12)) {
            
            
            if (select_menu_current_level == i) {
                draw_text(g_map_list[i], i + 2, 2 + 1, true, 64.f, NRGBA{sr,sg,sb,1});
            } else {
                draw_text(g_map_list[i], i + 2, 2, true, 64.f);
            }
            
            i++;
        }
        
    }
    
}