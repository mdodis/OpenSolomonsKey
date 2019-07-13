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
*/

#define OSK_PLATFORM_X11

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include <time.h>

#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

#include "OpenSolomonsKey.h"

u32 g_wind_width;
u32 g_wind_height;
u32 g_view_width;
u32 g_view_height;
double       g_tile_scale;


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
}

internal void
x11_kill()
{
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

timespec timespec_diff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

internal char _x11_internal_keys[32];

b32 x11_get_key_state(i32 key)
{
    int keycode = XKeysymToKeycode(dpy, key);
    if (_x11_internal_keys[keycode / 8] & (0x1 << ( keycode % 8 )))
        return true;
    else return false;
    
}

int main(int argc, char *argv[])
{
    x11_init();
    
    cb_init();
    
    struct timespec last, now, delta_timespec;
    clock_gettime(CLOCK_MONOTONIC, &last);
    while(1) {
        while (XCheckMaskEvent(dpy, KeyPressMask | ExposureMask, &xev) != False)
        {
            switch(xev.type)
            {
                case Expose:
                {
                    XGetWindowAttributes(dpy, win, &gwa);
                    g_wind_width = gwa.width;
                    g_wind_height = gwa.height;
                    cb_resize();
                } break;
                
            }
        }
        
        XQueryKeymap(dpy, _x11_internal_keys);
        ISTATE_KEYDOWN_ACTION(XK_space, spacebar_pressed);
        
        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_timespec = timespec_diff(last, now);
        float delta = delta_timespec.tv_nsec / 1000000.f;
        assert(delta > 0);
        // Render code here
        cb_render(g_input_state, delta);
        
        last = now;
        glXSwapBuffers(dpy, win);
    }
}


#include "OpenSolomonsKey.cpp"