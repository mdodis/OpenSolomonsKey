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

#include "OpenSolomonsKey.h"

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
