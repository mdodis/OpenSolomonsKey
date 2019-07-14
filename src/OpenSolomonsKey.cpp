#include <stdio.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


inline i32 ftrunc(float n) { return (i32)(n + 0.5f); }

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
    
    if (((double)current_w / (double)current_h) == W_2_H)
    {
        vw = current_w;
        vh = current_h;
        glViewport(0,0,current_w, current_h);
        
    } else {
        
        if (current_w > current_h)
        {
            vh = current_h;
            vw = (i32)((double)(vh) * W_2_H);
            vw = ftrunc(vw);
            
            leftover = (i32)current_w - (i32)vw;
            if (leftover < 0)
            {
                // if the new one is bigger than what we show
                vh += leftover;
                vw = (i32)((double)(vh) * W_2_H);
                vw = ftrunc(vw);
            }
            leftover = (i32)current_w - (i32)vw;
            assert(leftover >= 0);
            
            glViewport(leftover / 2, 0, vw, vh);
        } else {
            vw = current_w;
            vh = (i32)((double)(vw) * HEIGHT_2_WIDTH_SCALE);
            vh = ftrunc(vh);
            
            leftover = (i32)current_h - (i32)vh;
            assert(leftover >= 0);
            
            glViewport(0, leftover / 2, vw, vh);
        }
        
    }
    
    *out_w = vw;
    *out_h = vh;
    
    return (int)vw * (0.0625);
}

internal u32 
gl_load_rgba_texture(u8* data, i32 width, i32 height)
{
    u32 result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width, height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);
    
    assert(glGetError() == GL_NO_ERROR);
    
    return result;
}

internal u8*
load_image_as_rgba_pixels(
const char* const name,
i32* out_width,
i32* out_height,
i32* out_n)
{
    int i_w, i_h, i_n;
    unsigned char* data = stbi_load(name, out_width, out_height, out_n, 4);
    
    return data;
}

/// test
global u32 g_text_id;
global u32 g_horiz_l0_id;
global u32 g_horiz_l1_id;

global u32 g_atlas;

void
cb_init()
{
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    //
    int w, h, n;
    //u8* data = load_image_as_rgba_pixels("glassesboy.png",&w,&h,&n);
    //g_text_id = gl_load_rgba_texture(data, w, h);
    //stbi_image_free(data);
    //
    //data = load_image_as_rgba_pixels("horiz_l0.png",&w,&h,&n);
    //assert(data);
    //g_horiz_l0_id = gl_load_rgba_texture(data, w, h);
    //stbi_image_free(data);
    //
    //data = load_image_as_rgba_pixels("horiz_l1.png",&w,&h,&n);
    //assert(data);
    //g_horiz_l1_id = gl_load_rgba_texture(data, w, h);
    //stbi_image_free(data);
    //
    
    
    u8* data = load_image_as_rgba_pixels("test_tilemap.png",&w,&h,&n);
    assert(data);
    g_atlas = gl_load_rgba_texture(data, w, h);
    
    return;
    stbi_image_free(data);
}

#include <math.h>
int highestPowerof2(int n) 
{ 
    int p = (int)log2(n); 
    return (int)pow(2, p);  
} 

void
cb_resize()
{
    g_tile_scale = get_tilescale_and_dimensions(
        g_wind_width, g_wind_height,
        &g_view_width, &g_view_height);
    
#ifdef OSK_ROUND_TO_POW_64
    g_tile_scale = highestPowerof2((u64)g_tile_scale);
    
    g_view_width  = g_tile_scale * 16;
    g_view_height = (i32)((double)(g_view_width) * HEIGHT_2_WIDTH_SCALE);
    
    int lx = (i32)g_wind_width - g_view_width;
    int ly = (i32)g_wind_height - g_view_height;
    glViewport(lx / 2, ly / 2, g_view_width, g_view_height);
#endif
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_view_width , g_view_height, 0, -1.f, 1.f);
    
    printf("LEFT PAD: %f\n", HORIZ_SCREEN_PAD);
    
    printf("\tResize: W:%d|%d H:%d|%d\n\t 64px is now %f\n",
           g_wind_width,
           g_view_width,
           g_wind_height,
           g_view_height,
           g_tile_scale);
    
    g_pixel_scale = (float)g_tile_scale / 64.0f;
}

/*
NOTE(miked): Has to be preceded by a call to glBegin(GL_QUADS)
*/
internal void
gl_draw_rect_textured(
i32 x, i32 y,
i32 w, i32 h,
u32 texture_id)
{
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glBegin(GL_QUADS);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glTexCoord2f(0.f, 0.f);
    glVertex2f(x, y + h);
    glTexCoord2f(1.f, 0.f);
    glVertex2f( x + w, y + h);
    glTexCoord2f(1.f, -1.f);
    glVertex2f(x + w, y);
    glTexCoord2f(0.f, -1.f);
    glVertex2f(x, y);
    glEnd();
    
}

internal void
gl_draw_rect_untextured(
float x, float y,
float w,  float h,
float r, float g, float b, float a)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBegin(GL_QUADS);
    glColor4f(r, g, b, a);
    glVertex2f(x, y + h);
    glVertex2f( x + w, y + h);
    glVertex2f(x + w, y);
    glVertex2f(x, y);
    glEnd();
    
}

int x = 0;
i32 y = 0;

int tilex = 0;
int tiley = 0;

void
cb_render(InputState istate, float dt)
{
    glClearColor( 0.156, 0.156,  0.156, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 14; j++)
        {
            gl_draw_rect_untextured(
                g_tile_scale * (i + 0), g_tile_scale * (j + 0),
                g_tile_scale, g_tile_scale,
                (float)i / 15.f, (float)(i + j) / 28.f,(float)j / 13.f, 1.f);
        }
        
    }
    
    if (istate.spacebar_pressed)
    {
        y += dt * 0.5f;
        x+= dt * 0.5f;
    }
    
    glBindTexture(GL_TEXTURE_2D, g_atlas);
    
    
    int w = g_tile_scale;
    int h = g_tile_scale;
    
    double incx = 1.0 / 5.0;
    double incy = 1.0 / 5.0;
    
    glPushMatrix();
    
    
    glBegin(GL_QUADS);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    glTexCoord2d(incx * tilex, incy * tiley); // upper left
    glVertex2f(x, y);
    
    
    glTexCoord2d(incx *  (tilex + 1), incy * tiley);
    glVertex2f(x + w, y); // upper right
    
    
    glTexCoord2d(incx * (tilex + 1), incy * (tiley + 1) );
    glVertex2f( x + w, y + h); // lower right
    
    
    glTexCoord2d(incx * tilex, incy * (tiley + 1) );
    glVertex2f(x, y + h); // lower left
    
    glEnd();
    
    glPopMatrix();
    
    
    //gl_draw_rect_textured(0, y * g_pixel_scale, g_tile_scale, g_tile_scale, g_text_id);
    
    
    //glFlush();
}

