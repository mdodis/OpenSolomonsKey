/* NOTE(miked): 
Attempting to switch to a non palleted level format.
*/

#define IS_DIGIT(x) (x >= '0' && x <= '9')

internal const char*
string_trim(const char* c)
{
    while(*c == ' ' || *c == '\n') c++;
    return c;
}

internal const char* string_nextline(const char* c)
{
    while (*c != '\n') c++;
    return c + 1;
}

internal const char* string_parse(const char* c, const char* str)
{
    while (*str && *c && *c == *str)
    {
        c++;
        str++;
    }
    
    if (!(*str))
        return c;
    
    return 0;
}

internal const char*
string_parse_uint(const char* c, u64* out_i)
{
    u64 res = 0;
    while (*c && IS_DIGIT(*c))
    {
        res *= 10;
        res += *c - '0';
        
        c++;
    }
    *out_i = res;
    return c;
}


internal void level_load(char* data)
{
    const char* c = data;
#define OSK_LEVEL_FMT_VERSION "V0.2"
    fail_unless(string_parse(c, OSK_LEVEL_FMT_VERSION), "Version string does not match!");
    
    c += 5;
    inform("%s", "LOADING LEVEL...");
    
    u32 counter_x = 0;
    u32 counter_y = 0;
    
    while (*c)
    {
        switch(*c)
        {
            case '#':
            {
                c = string_nextline(c);
            }break;
            
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                u64 res;
                c = string_parse_uint(c, &res);
                // TODO(miked): add custom parameters with commas!
                
                fail_unless(res < EntityBaseType_Count,
                            "Entity index does not exist in version" 
                            OSK_LEVEL_FMT_VERSION);
                
                if (counter_x >= TILEMAP_COLS)
                {
                    counter_x = 0;
                    //printf("\t%d\n", counter_y);
                    counter_y++;
                }
                
                g_scene.tilemap[counter_x][counter_y] = res;
                
                counter_x++;
                
                
            } break;
            
            default:
            {
                c++;
            } break;
        }
    }
    
}

internal void scene_init(const char* level_path)
{
    //load_test_level(g_scene.palette, &g_scene.palette_size);
    char* lvl = platform_load_entire_file("lvl1.osk");
    level_load(lvl);
    free(lvl);
    
    //exit(0);
}

internal void 
scene_sprite_add(Sprite* sprite)
{
    fail_unless(sprite, "Passing null sprite to scene_add");
    ca_push_array(Sprite, &g_scene.spritelist, sprite, 1);
}

internal void
scene_draw_tilemap()
{
    for(int i = 0; i < 15; ++i )
    {
        for(int j = 0; j < 12; ++j )
        {
            u32 id;
            EntityBaseType type = (EntityBaseType)g_scene.tilemap[i][j];
            
            if (type == eEmptySpace) continue;
            if (type == eBlockSolid) id = 2;
            if (type == eBlockFrail) id = 1;
            
            gl_slow_tilemap_draw(
                &GET_TILEMAP_TEXTURE(test),
                {i * 64, j * 64},
                {64, 64},
                0.f,
                id);
        }
    }
}

internal void 
scene_update(InputState* istate, float dt)
{
    scene_draw_tilemap();
    
    List_Sprite l = g_scene.spritelist;
    
    for (int i = 0; i < l.sz; ++i)
    {
        Sprite* spref = l.data + i;
        Sprite_update_animation(spref, dt);
        
        switch(spref->entity.type)
        {
            case ePlayer:
            {
                ePlayer_update(spref, istate, dt);
            }break;
            
            
            default:
            break;
        }
        
        Sprite_draw_anim(l.data + i);
    }
}
