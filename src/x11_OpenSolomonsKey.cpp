/* x11_OpenSolomonsKey.cpp

This is the initial Xlib + Alsa layer for OpenSolomonsKey.

Tilesize: 64x64 (px)
Left+Right: 32 + 32 (so an extra tile_x)
Top+Bottom: 64 + 64 (so 2 extra tiles on y)
Total: 1024x896
Total_PlayArea: 960x768
AspectRatio: 0.875 (height/width, height = width * 0.875)

15x12 play area
*/

#define OSK_PLATFORM_X11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include "OpenSolomonsKey.h"
#include "gl_funcs.h"

#include <portaudio.h>

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

struct Timer
{
    struct timespec last;
    
    double get_elapsed_secs()
    {
        struct timespec now;
        struct timespec delta_timespec;
        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_timespec = timespec_diff(this->last, now);
        double delta = (double)delta_timespec.tv_nsec /(double) 1000000000.0;
        delta += delta_timespec.tv_sec;
        return delta;
    }
    
    void reset()
    {
        clock_gettime(CLOCK_MONOTONIC, &last);
    }
};

Display                 *dpy;
Window                  root;
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  xev;

global bool g_ctx_error = false;

static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    g_ctx_error = true;
    return 0;
}

static bool isExtensionSupported(const char *extList, const char *extension)
{
    const char *start;
    const char *where, *terminator;
    
    /* Extension names should not have spaces. */
    where = strchr(extension, ' ');
    if (where || *extension == '\0')
        return false;
    
    /* It takes a bit of care to be fool-proof about parsing the
       OpenGL extensions string. Don't be fooled by sub-strings,
       etc. */
    for (start=extList;;) {
        where = strstr(start, extension);
        
        if (!where)
            break;
        
        terminator = where + strlen(extension);
        
        if ( where == start || *(where - 1) == ' ' )
            if ( *terminator == ' ' || *terminator == '\0' )
            return true;
        
        start = terminator;
    }
    
    return false;
}

/*
Oh dear god...
*/
internal void
x11_init()
{
    
    GLint                   att[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };
    int glx_major, glx_minor;
    
    dpy = XOpenDisplay(NULL);
    fail_unless(dpy, "Cannot connect to X server");
    
    int screen = DefaultScreen(dpy);
    root = DefaultRootWindow(dpy);
    
    fail_unless(
        glXQueryVersion(dpy, &glx_major, &glx_minor) && 
        (glx_major != 1  || glx_minor >= 3) &&
        (glx_major >= 1),
        "Invalid GLX version");
    
    int elemc;
    GLXFBConfig *fbcfg = glXChooseFBConfig(dpy, screen, att, &elemc);
    
    fail_unless(fbcfg, "Could not get FB configs");
    inform("Got %d FB configs", elemc);
    
    // get visual info
    
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
    
    for (int i=0; i<elemc; ++i)
    {
        XVisualInfo *t_vi = glXGetVisualFromFBConfig( dpy, fbcfg[i] );
        if ( t_vi )
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib( dpy, fbcfg[i], GLX_SAMPLE_BUFFERS, &samp_buf );
            glXGetFBConfigAttrib( dpy, fbcfg[i], GLX_SAMPLES       , &samples  );
            
            inform("Matching fbconfig %2d, visual ID 0x%03x: SAMPLE_BUFFERS = %d,"
                   " samples = %d",
                   i, t_vi->visualid, samp_buf, samples );
            
            if ( best_fbc < 0 || samp_buf && samples > best_num_samp )
                best_fbc = i, best_num_samp = samples;
            if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
                worst_fbc = i, worst_num_samp = samples;
        }
        XFree( t_vi );
    }
    
    GLXFBConfig bestFbc = fbcfg[ best_fbc ];
    XFree(fbcfg);
    
    vi = glXGetVisualFromFBConfig(dpy, bestFbc);
    inform("Chosen visual ID = 0x%x", vi->visualid );
    
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    
    swa.colormap = cmap;
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = ExposureMask | KeyPressMask;
    
    win = XCreateWindow(dpy, root, 0, 0, 700, 700, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    
    XClassHint h = {"solomons key", "popup"};
    XSetClassHint(dpy, win, &h);
    
    XFree(vi);
    
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Open Solomon's Key");
    
    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString( dpy, DefaultScreen( dpy ) );
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
    
    int (*oldHandler)(Display*, XErrorEvent*) =
        XSetErrorHandler(&ctxErrorHandler);
    
    
    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
        !glXCreateContextAttribsARB )
    {
        warn("%s", "glXCreateContextAttribsARB() not found"
             " ... using old-style GLX context" );
        glc = glXCreateNewContext( dpy, bestFbc, GLX_RGBA_TYPE, 0, True );
    }
    else
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };
        
        inform("%s", "Creating context");
        glc = glXCreateContextAttribsARB( dpy, bestFbc, 0, True, context_attribs );
        // Sync to ensure any errors generated are processed.
        XSync( dpy, False );
        
        if ( !g_ctx_error && glc )
            inform("%s", "Created GL 3.0 context");
        else
        {
            // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
            // When a context version below 3.0 is requested, implementations will
            // return the newest context version compatible with OpenGL versions less
            // than version 3.0.
            // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
            context_attribs[1] = 1;
            // GLX_CONTEXT_MINOR_VERSION_ARB = 0
            context_attribs[3] = 0;
            
            g_ctx_error = false;
            
            warn("%s", "Failed to create GL 3.0 context"
                 " ... using old-style GLX context" );
            glc = glXCreateContextAttribsARB( dpy, bestFbc, 0,
                                             True, context_attribs );
        }
    }
    
    // Sync to ensure any errors generated are processed.
    XSync( dpy, False );
    
    // Restore the original error handler
    XSetErrorHandler( oldHandler );
    
    fail_unless(!g_ctx_error && glc, "Failed to create an OpenGL context");
    
    // Verifying that context is a direct context
    if ( ! glXIsDirect ( dpy, glc ) )
    {
        inform("%s", "Indirect GLX rendering context obtained");
    }
    else
    {
        inform("%s", "Direct GLX rendering context obtained");
    }
    
    inform("%s", "Making context current");
    glXMakeCurrent( dpy, win, glc );
    
    // NOTE(miked): if this ever fails, switch to a windowing lib.
    // I'm not doing this again
}

internal void
x11_kill()
{
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

internal char _x11_internal_keys[32];

b32 x11_get_key_state(i32 key)
{
    int keycode = XKeysymToKeycode(dpy, key);
    if (_x11_internal_keys[keycode / 8] & (0x1 << ( keycode % 8 )))
        return true;
    else return false;
    
}

internal void 
x11_update_all_keys()
{
    XQueryKeymap(dpy, _x11_internal_keys);
    
#define KEYDOWN(name, keysym, ...) g_input_state.name = x11_get_key_state(keysym);
#define KEYPRESS(name, keysym, ...) { \
        b32 now = x11_get_key_state(keysym); \
        g_input_state.name[1] = (now && !g_input_state.name[0]); \
        g_input_state.name[0] = g_input_state.name[1] || now; \
        \
    } \
    
    KEYMAP
    
#undef KEYDOWN
#undef KEYPRESS
}

global PaStream* portaudio_stream;
internal void
portaudio_init()
{
    PaError err;
    err = Pa_Initialize();
    fail_unless(err == paNoError, "failed to init portaudio");
    
    err = Pa_OpenDefaultStream(
        &portaudio_stream,
        0,          /* no input channels */
        AUDIO_CHANNELS,
        paInt16,  /* 32 bit floating point output */
        AUDIO_SAMPLERATE,
        AUDIO_FRAMES,        /* frames per buffer, i.e. the number
        of sample frames that PortAudio will
        request from the callback. Many apps
        may want to use
        paFramesPerBufferUnspecified, which
        tells PortAudio to pick the best,
        possibly changing, buffer size.*/
        0, /* this is your callback function */
        0);
    
    fail_unless(err == paNoError, "failed to open stream");
    err = Pa_StartStream(portaudio_stream);
    fail_unless(err == paNoError, "failed to start stream");
}

internal i64
portaudio_get_samples_to_write()
{
    i64 result = Pa_GetStreamWriteAvailable(portaudio_stream);
    
    fail_unless(result >= 0, "");
    
    return result;
}

internal void
portaudio_update_buffer(i64 frames_to_write)
{
    PaError err = Pa_WriteStream(
        portaudio_stream,
        g_audio.buffer,
        frames_to_write);
    
    if (err != paNoError)
    {
        
        fprintf(stderr, "Pa_WriteStream returned: %s", Pa_GetErrorText(err));
    }
}

int main(int argc, char *argv[])
{
    x11_init();
    gl_load();
    
    b32 m_final = false;
    b32 m_prev = false;
    
    portaudio_init();
    
    Timer timer;
    timer.reset();
    
    cb_init();
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
        
        x11_update_all_keys();
        
        float delta = timer.get_elapsed_secs();
        timer.reset();
        i64 samples_to_write = portaudio_get_samples_to_write();
        // Render code here
        cb_render(g_input_state, samples_to_write, delta);
        portaudio_update_buffer(samples_to_write);
        
        glXSwapBuffers(dpy, win);
    }
}

#include "OpenSolomonsKey.cpp"
