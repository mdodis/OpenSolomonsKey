#define OSK_PLATFORM_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/glew.h>
#include <GL/GL.h>

#include "OpenSolomonsKey.h"

#define OSK_CLASS_NAME "OSK Class"

global WNDCLASSA g_wc;
global HWND     g_wind;
global HDC      g_dc;
global bool     g_running = true;

u32 g_wind_width;
u32 g_wind_height;
u32 g_view_width;
u32 g_view_height;
double g_tile_scale;

internal LRESULT CALLBACK
win32_windproc(
_In_ HWND   hwnd,
_In_ UINT   msg,
_In_ WPARAM wparam,
_In_ LPARAM lparam)
{
    LRESULT result = 0;
    
    switch(msg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            g_running = false;
        }break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        } break;
        
        case WM_SIZE:
        {
            g_wind_width = LOWORD(lparam);
            g_wind_height =  HIWORD(lparam);
            glViewport(0,0,g_wind_width, g_wind_height);
            
            cb_resize();
            
            OutputDebugStringA("WM_SIZE\n");
            PostMessage(hwnd, WM_PAINT, 0, 0);
        } break;
        
        default:
        {
            result = DefWindowProc(hwnd, msg, wparam, lparam);
        } break;
    }
    return result;
}

internal void
win32_init(HINSTANCE hInstance)
{
    g_wc.hInstance = hInstance;
    g_wc.lpszClassName = OSK_CLASS_NAME;
    g_wc.lpfnWndProc   = win32_windproc;
    g_wc.style = CS_OWNDC;
    // g_wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    g_wc.hbrBackground = (HBRUSH)(0);
    
    if (!RegisterClassA(&g_wc))
    {
        OutputDebugStringA("failed to register window class.");
        exit(1);
    }
    
    g_wind = CreateWindowExA(
        0,
        OSK_CLASS_NAME,
        "OpenSolomon's Key",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,1024, 896,
        NULL, NULL, hInstance, NULL);
    
    if (!g_wind)
    {
        OutputDebugStringA("failed to create window.");
        exit(1);
    }
    
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    HDC dev_ctx = GetDC(g_wind);
    int chosen_pixel_fmt = ChoosePixelFormat(dev_ctx, &pfd);
    SetPixelFormat(dev_ctx, chosen_pixel_fmt, &pfd);
    
    HGLRC render_ctx = wglCreateContext(dev_ctx);
    wglMakeCurrent(dev_ctx, render_ctx);
    
    OutputDebugStringA("OPENGL: ");
    OutputDebugStringA((char*)glGetString(GL_VERSION));
    OutputDebugStringA("\n");
    
    ReleaseDC(g_wind, dev_ctx);
    
}


b32 win32_get_key_state(i32 key)
{
    return GetAsyncKeyState(key);
}

int WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR     lpCmdLine,
int       nShowCmd)
{
    MSG message;
    LARGE_INTEGER perf_last, perf_now = {};
    LARGE_INTEGER perf_freq;
    
    win32_init(hInstance);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        OutputDebugStringA("NOPE\n");
        return 10;
    }
    cb_init();
    ShowWindow(g_wind, SW_SHOW);
    
    g_dc = GetDC(g_wind);
    
    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&perf_last);
    
    while (g_running)
    {
        while (PeekMessage(&message, g_wind, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        
        ISTATE_KEYDOWN_ACTION(VK_SPACE, spacebar_pressed);
        
        
        QueryPerformanceCounter(&perf_now);
        i64 time_elapsed = perf_now.QuadPart - perf_last.QuadPart;
        float delta = (time_elapsed * 1000) / perf_freq.QuadPart;
        assert(delta >= 0);
        
        cb_render(g_input_state,delta);
        perf_last = perf_now;
        SwapBuffers(g_dc); 
    }
    
    return 0;
}

#undef OSK_CLASS_NAME

#include "OpenSolomonsKey.cpp"