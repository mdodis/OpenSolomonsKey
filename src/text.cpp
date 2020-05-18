/* text.cpp
 Open Solomon's Key
*/

void draw_num(float num,
              int line = 0,
              int xoffset = 0,
              bool account_for_offset = true,
              float size = 16.f,
              bool trunc = false,
              NRGBA tint = {1,1,1,1})
{
    char buf[20] = "";
    
    sprintf(buf, trunc ? "%0.0f" : "%f", num);
    
    char* c = buf;
    float increment = 0;
    while(*c)
    {
        if (*c >= '0' && *c <= '9')
        {
            int c_to_font = (*c - 48) + (1 * 16);
            gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(font),
                                 fvec2{xoffset * size + increment, line * size},
                                 fvec2{size, size},
                                 0,
                                 c_to_font,
                                 false, false, tint,
                                 account_for_offset);
            
        } else if (*c == '.') {
            gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(font),
                                 fvec2{xoffset * size + increment, line * size},
                                 fvec2{size, size},
                                 0,
                                 14,
                                 false, false, tint,
                                 account_for_offset);
            
        }
        increment += size;
        
        c++;
    }
}


void draw_text(char* text, int line, int xoffset = 0, bool account_for_offset = true, float size = 16.f, NRGBA tint = {1,1,1,1}) {
    char* c = text;
    float increment = 0.f;
    
    while(*c) {
        int c_to_font = -1;
        
        if (*c >= 'a' && *c <= 'z')
            c_to_font = (*c - 65) + (2 * 16 + 1);
        else if (*c >= 'A' && *c <= 'Z')
            c_to_font = (*c - 97) + (4 * 16 + 1);
        else if (*c >= '0' && *c <= '?')
            c_to_font = (*c - 48) + (1 * 16);
        
        if (c_to_font != -1) {
            gl_slow_tilemap_draw(&GET_TILEMAP_TEXTURE(font),
                                 fvec2{xoffset * size + increment,line * size},
                                 fvec2{size, size},
                                 0,
                                 c_to_font,
                                 false, false,
                                 tint,
                                 account_for_offset);
        }
        
        increment += size;
        c++;
    }
}
