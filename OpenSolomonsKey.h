#ifndef OSK_HH
#define OSK_HH

#define internal static
#define global   static
#define persist  static

#include <stdint.h>

typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

global u32 g_wind_width;
global u32 g_wind_height;

global u32 g_view_width;
global u32 g_view_height;

global double       g_tile_scale;

#define W_2_H 1.1428571428571428
#define HEIGHT_2_WIDTH_SCALE 0.875

#define IS_POW2(x) x && !(x & (x - 1))

void DrawAQuad() {
    //glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBegin(GL_QUADS);
    
    glColor4f(1.0, 0.0, 0.0, 1.f);
    glVertex2f(0.0, g_view_height);
    glVertex2f(g_view_width, g_view_height);
    glVertex2f(g_view_width, 0.0);
    glVertex2f(0.0, 0.0);
    glEnd();
    
    glBegin(GL_QUADS);
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 14; j++)
        {
            glColor4f((float)i / 15.f, (float)(i + j) / 28.f,(float)j / 13.f, 1.f);
            glVertex2f(g_tile_scale * (i + 0), g_tile_scale * (j + 1));
            glVertex2f(g_tile_scale * (i + 1), g_tile_scale * (j + 1));
            glVertex2f(g_tile_scale * (i + 1), g_tile_scale * (j + 0));
            glVertex2f(g_tile_scale * (i + 0), g_tile_scale * (j + 0));
            
        }
        
    }
    
    glEnd();
    
    glFlush();
}

/* Calculate aspect ratio from current window dimensions.
 Returns the size of a tile in pixels. For example, a window
 of dimensions (1024x896) will get a tilescale of 64 pixels
*/
internal double
get_tilescale_and_dimensions(
u32 current_w, u32 current_h,
u32* out_w, u32* out_h)
{
    i32 vw, vh;
    int leftover;
    printf("CALC %d | %d\n\n", current_w, current_h);
    
    if (((double)current_h * HEIGHT_2_WIDTH_SCALE) == current_w)
    {
        vw = current_w;
        vh = current_h;
    } else{
        
        if (current_w > current_h)
        {
            vh = current_h;
            vw = (i32)((double)(vh) * W_2_H);
            
            leftover = (i32)current_w - (i32)vw;
            if (leftover < 0)
            {
                vh += leftover;
                vw = (i32)((double)(vh) * W_2_H);
            }
            leftover = (i32)current_w - (i32)vw;
            assert(leftover >= 0);
            
            glViewport(leftover / 2, 0, vw, vh);
        } else {
            vw = current_w;
            vh = (i32)((double)(vw) * HEIGHT_2_WIDTH_SCALE);
            
            leftover = (i32)current_h - (i32)vh;
            assert(leftover >= 0);
            
            glViewport(0, leftover / 2, vw, vh);
        }
        
    }
    
    *out_w = vw;
    *out_h = vh;
    
    return (int)vw * (0.0625);
}


#endif //! OSK_HH