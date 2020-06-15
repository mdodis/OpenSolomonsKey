
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#define OSK_PLATFORM_SDL
#include "OpenSolomonsKey.h"
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>

#include <alsa/asoundlib.h>
#include <pthread.h>
#include <semaphore.h>

timespec timespec_diff(timespec start, timespec end) {
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

struct Timer {
    struct timespec last;
    
    double get_elapsed_secs() {
        struct timespec now;
        struct timespec delta_timespec;
        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_timespec = timespec_diff(this->last, now);
        double delta = (double)delta_timespec.tv_nsec /(double) 1000000000.0;
        delta += delta_timespec.tv_sec;
        return delta;
    }
    
    void reset() {
        clock_gettime(CLOCK_MONOTONIC, &last);
    }
};

void cb_init();
void cb_render(InputState istate, u64 audio_sample_count, float dt);
void cb_resize();

global snd_pcm_t* g_alsapcm;
global sem_t g_audiosem;
global pthread_t g_audiothread;

internal void audio_update(const InputState* const istate, u64 samples_to_write);
internal void audio_update_all_sounds();

void* alsa_cb_audio(void* unused) {
    
    i16 *buffer = (i16*)g_audio.buffer;
    for(;;)  {
        /* 1 frame = [L16|R16] in our case */
        snd_pcm_sframes_t avail = snd_pcm_avail_update(g_alsapcm);
        if (avail > 0) {
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
            
            snd_pcm_sframes_t written = snd_pcm_writei(g_alsapcm, buffer, n_to_write);
            
            if (written == -EPIPE) {
                fprintf(stderr, "EPIPE\n");
                assert(!snd_pcm_prepare(g_alsapcm));
            }
            
        }
        
    }
    return 0;
}

internal void
alsa_init() {
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

global bool g_running = true;
void sdl_exit(int num) {
    fprintf(stderr, "Exit called with %d\n", num);
    g_running = false;
}

#define exit(num) sdl_exit(num)

global const Uint8 *g_keyboard = 0;

internal inline b32 sdl_get_key_state(i32 key) {
    return (g_keyboard[key]);
}

internal void 
sdl_update_all_keys()
{
#define KEYDOWN(name, _u0, _u1, keysym) g_input_state.name = sdl_get_key_state(keysym);
#define KEYPRESS(name, _u0, _u1, keysym) { \
b32 now = sdl_get_key_state(keysym); \
g_input_state.name[1] = (now && !g_input_state.name[0]); \
g_input_state.name[0] = g_input_state.name[1] || now; \
    \
} \
    
    KEYMAP
        
#undef KEYDOWN
#undef KEYPRESS
}

int main(int argc, char **argv) {
    
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    g_wind_width = 640u;
    g_wind_height = 480u;
    
    SDL_Window *window;
    
    if ((argc > 1) && strcmp(argv[1], "-fullscreen") == 0) {
        SDL_DisplayMode mode;
        assert(SDL_GetDesktopDisplayMode(0, &mode) == 0);
        
        g_wind_width = mode.w;
        g_wind_height = mode.h;
        
        window = SDL_CreateWindow("Open Solomon's Key", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, g_wind_width, g_wind_height, SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN);
    } else {
        window = SDL_CreateWindow("Open Solomon's Key", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, g_wind_width, g_wind_height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    }
    
    fail_unless(window, "Could not create an SDL window");
    
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    // NOTE: using KMSDRM backend has really bad tearing without this,
    // but the only machine I tested is an Ubuntu laptop so...
    assert(SDL_GL_SetSwapInterval(1) == 0);
    fail_unless(glcontext != NULL, "Failed to create an gles context");
    
    g_keyboard = SDL_GetKeyboardState(0);
    cb_init();
    alsa_init();
    cb_resize();
    
    Timer timer;
    timer.reset();
    
    while (g_running) {
        SDL_Event event;
        
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
                case SDL_QUIT: {
                    g_running = false;
                }break;
                
                case SDL_WINDOWEVENT: {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        g_wind_width = event.window.data1;
                        g_wind_height = event.window.data2;
                        cb_resize();
                    }
                }break;
            }
        }
        
        sdl_update_all_keys();
        float delta = timer.get_elapsed_secs();
        timer.reset();
        // Render code here
        cb_render(g_input_state, 0, delta);
        
        {
            snd_pcm_sframes_t avail = snd_pcm_avail_update(g_alsapcm);
            if (g_alsapcm > 0)
                sem_post(&g_audiosem);
        }
        
        SDL_GL_SwapWindow(window);
        // TODO(miked): fullscreen toggle?
        
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
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

#include "OpenSolomonsKey.cpp"
