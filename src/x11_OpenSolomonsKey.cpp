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
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "OpenSolomonsKey.h"
#include "gl_funcs.h"

#include <alsa/asoundlib.h>
#include <pthread.h>
#include <semaphore.h>


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

/* Window Context */
global Display                 *dpy;
global Window                  root;
global XVisualInfo             *vi;
global Colormap                cmap;
global XSetWindowAttributes    swa;
global Window                  win;
global GLXContext              glc;
global XWindowAttributes       gwa;
global XEvent                  xev;

global bool g_ctx_error = false;

global snd_pcm_t* g_alsapcm;
global sem_t g_audiosem;
global pthread_t g_audiothread;

internal void audio_update(const InputState* const istate, u64 samples_to_write);
internal void audio_update_all_sounds();

void* alsa_cb_audio(void* unused)
{
    
    i16 *buffer = (i16*)g_audio.buffer;
    for(;;) 
    {
        /* 1 frame = [L16|R16] in our case */
        snd_pcm_sframes_t avail = snd_pcm_avail_update(g_alsapcm);
        if (avail > 0)
        {
            u32 bytes_per_frame = 
                .0166f * 
                (float)(AUDIO_SAMPLERATE * AUDIO_BYTESPERSAMPLE);
            
            u32 n_to_write = avail * AUDIO_CHANNELS;
            
            if (avail * AUDIO_BYTESPERSAMPLE > bytes_per_frame)
                n_to_write = (bytes_per_frame / AUDIO_BYTESPERSAMPLE);
            
            audio_update_all_sounds();
            
            sem_wait(&g_audiosem);
            audio_update(0, n_to_write);
            sem_post(&g_audiosem);
            
            snd_pcm_sframes_t written = 
                snd_pcm_writei(g_alsapcm, buffer, n_to_write);
            
            if (written == -EPIPE)
            {
                fprintf(stderr, "EPIPE\n");
                assert(!snd_pcm_prepare(g_alsapcm));
            }
            
        }
        
    }
    return 0;
}

static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    g_ctx_error = true;
    return 0;
}

static bool x11_extension_supported(const char *extList, const char *extension)
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
            
            inform("Matching fbconfig %2d, visual ID 0x%03lx: SAMPLE_BUFFERS = %d,"
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
    inform("Chosen visual ID = 0x%lx", vi->visualid );
    
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    
    swa.colormap = cmap;
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
    
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
    if ( !x11_extension_supported( glxExts, "GLX_ARB_create_context" ) ||
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

internal void
alsa_init()
{
    snd_pcm_open(&g_alsapcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    snd_pcm_set_params(g_alsapcm,
                       SND_PCM_FORMAT_S16_LE,
                       SND_PCM_ACCESS_RW_INTERLEAVED,
                       AUDIO_CHANNELS,
                       AUDIO_SAMPLERATE,
                       1,
                       16667);
    sem_init(&g_audiosem, 0, 0);
    pthread_create(&g_audiothread, 0, alsa_cb_audio, 0);
    
}


#include <vector>
#include <dirent.h>
std::vector<char *> list_maps() {
    std::vector<char *> result;
    
    struct dirent **namelist;
    int n, i;
    
    n = scandir(".", &namelist, 0, alphasort);
    if (n >= 0) {
        for (i = 0; i < n; ++i) {
            if (strncmp(namelist[i]->d_name, "level_", 6) == 0) {
                result.push_back(strdup(namelist[i]->d_name));
            }
        }
        free(namelist);
    }
    return result;
}

global bool is_fullscreen = false;
void toggle_fullscreen(Display* dpy, Window win) {
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom fullscreen_state = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN",  False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = is_fullscreen;
    xev.xclient.data.l[1] = fullscreen_state;
    xev.xclient.data.l[2] = 0;
    
    XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XFlush(dpy);

    is_fullscreen = !is_fullscreen;

}

int main(int argc, char *argv[]) {
    x11_init();
    gl_load();
    
    b32 m_final = false;
    b32 m_prev = false;
    
    alsa_init();
    
    Timer timer;
    timer.reset();
    
    cb_init();
    while(1) {
        while (XCheckMaskEvent(dpy, KeyPressMask | KeyReleaseMask | ExposureMask, &xev) != False) {
            switch(xev.type) {
                case Expose: {
                    XGetWindowAttributes(dpy, win, &gwa);
                    g_wind_width = gwa.width;
                    g_wind_height = gwa.height;
                    cb_resize();
                } break;

            }
        }
        
        x11_update_all_keys();

        if (GET_KEYPRESS(go_fullscreen)) {
            toggle_fullscreen(dpy, win);
        }
        
        float delta = timer.get_elapsed_secs();
        timer.reset();
        // Render code here
        cb_render(g_input_state, 0, delta);
        
        {
            snd_pcm_sframes_t avail = snd_pcm_avail_update(g_alsapcm);
            if (g_alsapcm > 0)
                sem_post(&g_audiosem);
        }
        
        glXSwapBuffers(dpy, win);
    }
}

#include "OpenSolomonsKey.cpp"
