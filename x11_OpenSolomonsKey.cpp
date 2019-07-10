/*
x11_OpenSolomonsKey.cpp
Windoing for the platform layer

Tilesize: 64x64 (px)
Left+Right: 32 + 32 (so an extra tile_x)
Top+Bottom: 64 + 64 (so 2 extra tiles on y)
Total: 1024x896
Total_PlayArea: 960x768
AspectRatio: 0.875 (height/width, height = width * 0.875)

15x12 play area

TODO(mdodis):
* Start win32 layer
*/

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

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

#define W_2_H 1.1428571428571428
#define HEIGHT_2_WIDTH_SCALE 0.875

#define IS_POW2(x) x && !(x & (x - 1))

Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;

global u32 g_wind_width;
global u32 g_wind_height;

global u32 g_view_width;
global u32 g_view_height;

global double       g_tile_scale;

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

internal void
x11_init()
{
    
    dpy = XOpenDisplay(NULL);
    
    if(dpy == NULL) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }
    
    root = DefaultRootWindow(dpy);
    
    vi = glXChooseVisual(dpy, 0, att);
    
    if(vi == NULL) {
        printf("\n\tno appropriate visual found\n\n");
        exit(0);
    }
    else {
        printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
    }
    
    
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;
    
    win = XCreateWindow(dpy, root, 0, 0, 1024, 896, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Open Solomon's Key");
    
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
    
    //glEnable(GL_DEPTH_TEST);
    
}

internal void
x11_kill()
{
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
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

int main(int argc, char *argv[])
{
    x11_init();
    
    while(1) {
        while (XCheckMaskEvent(dpy, KeyPressMask | ExposureMask, &xev) != False)
        {
            if(xev.type == Expose) {
                XGetWindowAttributes(dpy, win, &gwa);
                g_wind_width = gwa.width;
                g_wind_height = gwa.height;
                
                g_tile_scale = get_tilescale_and_dimensions(
                    g_wind_width, g_wind_height,
                    &g_view_width, &g_view_height);
                
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0,g_view_width , g_view_height, 0, -1.f, 1.f);
                
                printf("\tResize: W:%d|%d H:%d|%d\n\t 64px is now %f\n",
                       g_wind_width,
                       g_view_width,
                       g_wind_height,
                       g_view_height,
                       g_tile_scale);
                
            }
            
        }
        // Render code here
        
        DrawAQuad();
        
        glXSwapBuffers(dpy, win);
    }
}
