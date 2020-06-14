
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#define OSK_PLATFORM_SDL
#include "OpenSolomonsKey.h"
#include <SDL2/SDL.h>
#include <GLES3/gl3.h>

extern void cb_init();

global bool g_running = true;

int main(int argc, char **argv) {
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    
    SDL_Window *window = SDL_CreateWindow("Open Solomon's Key", SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    fail_unless(window, "Could not create an SDL window");
    
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    fail_unless(glcontext != NULL, "Failed to create an gles context");
    
    cb_init();
    while (g_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
                case SDL_QUIT: {
                    g_running = false;
                }break;
            }
        }
    }
    
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