/* NOTE(miked):
Ok, so we're gonna have to do audio on a separate thread anyways
*/
#define OSK_PLATFORM_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/GL.h>
#include <portaudio.h>

#define OSK_PLATFORM_WIN32
#include "OpenSolomonsKey.h"
#define OSK_CLASS_NAME "OSK Class"

#include "gl_funcs.h"

LARGE_INTEGER perf_last, perf_now = {};
global LARGE_INTEGER perf_freq;

// NOTE(miked): Win32's DefWindowProc function takes the liberty of
// blocking the whole thread whilst moving the window (via the title bar),
// so we us these two bools to check for that, and NOT update.
// Done using WM_ENTERSIZEMOVE / WM_EXITSIZEMOVE
global b32 is_currently_moving_or_resizing = false;
global b32 was_previously_moving_or_resizing = false;

// NOTE(miked): Thank you https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
// Modern Opengl is a bitch to init on Windows
typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,const int *attribList);
wglCreateContextAttribsARB_type *wglCreateContextAttribsARB;

typedef const char* PFNwglGetExtensionsStringARB(HDC hdc);
PFNwglGetExtensionsStringARB* wglGetExtensionsStringARB;

typedef BOOL PFNwglSwapIntervalEXT(int interval);
PFNwglSwapIntervalEXT* wglSwapIntervalEXT;

b32 wgl_is_extension_supported(char* extname,  char* ext_string)
{
    char* csearch = extname;
    char* chay = ext_string;
    
    while(*csearch && *chay)
    {
        if (!(*chay))
        {
            if (!(*csearch))
                return false;
            else 
                return true;
        }
        
        if (*csearch == *chay)
        {
            csearch++;
        }
        else
            csearch = extname;
        
        chay++;
    }
    
    if (!(*csearch) && !(*csearch))
        return true;
    else
        return false;
    
}

// See https://www.opengl.org/registry/specs/ARB/wgl_create_context.txt for all values
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
                                                 const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
wglChoosePixelFormatARB_type *wglChoosePixelFormatARB;

// See https://www.opengl.org/registry/specs/ARB/wgl_pixel_format.txt for all values
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B


struct Timer
{
    LARGE_INTEGER time_last;
    
    double get_elapsed_secs(b32 should_reset = false)
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        i64 time_elapsed = now.QuadPart - time_last.QuadPart;
        float delta;
        if (perf_freq.QuadPart == 0) delta = 0;
        else delta = (time_elapsed * 1000.f) / perf_freq.QuadPart;
        assert((delta >= 0.f));
        
        if (should_reset) time_last = now;
        
        return (delta / 1000.f);
    }
    
    void reset()
    {
        QueryPerformanceCounter(&time_last);
    }
};

#include <mmsystem.h> // WIN32_LEAN_AND_MEAN probably doesn't load this
#include <dsound.h>

global HWND     g_wind;
global bool     g_running = true;
global LPDIRECTSOUNDBUFFER g_secondary_buffer;
/* Win32 AUDIO
Uses DSound.
*/

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef DIRECT_SOUND_CREATE(FNDirectSoundCreate);


internal void win32_dsound_init()
{
    // load dsound
    HMODULE dsound_lib = LoadLibraryA("dsound.dll");
    fail_unless(dsound_lib, "dsound failed to load");
    
    FNDirectSoundCreate* dsound_create = (FNDirectSoundCreate*)GetProcAddress(dsound_lib, "DirectSoundCreate");
    fail_unless(dsound_create, "");
    
    LPDIRECTSOUND dsound;
    if (!SUCCEEDED(dsound_create(0, &dsound, 0))) _exit_with_message("dsound_create");
    if (!SUCCEEDED(dsound->SetCooperativeLevel(g_wind, DSSCL_PRIORITY))) _exit_with_message("SetCooperativeLevel");
    
    WAVEFORMATEX wave_fmt = {};
    wave_fmt.wFormatTag = WAVE_FORMAT_PCM;
    wave_fmt.nChannels = AUDIO_CHANNELS;
    wave_fmt.nSamplesPerSec = AUDIO_SAMPLERATE;
    wave_fmt.wBitsPerSample = AUDIO_BPS;
    wave_fmt.nBlockAlign = (wave_fmt.nChannels * wave_fmt.wBitsPerSample) / 8;
    wave_fmt.nAvgBytesPerSec = wave_fmt.nSamplesPerSec * wave_fmt.nBlockAlign;
    wave_fmt.cbSize = 0;
    
    LPDIRECTSOUNDBUFFER primary_buffer;
    {
        DSBUFFERDESC buffer_desc = {};
        // create primary buffer
        buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        buffer_desc.dwSize = sizeof(DSBUFFERDESC);
        
        if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &primary_buffer, 0)))
            _exit_with_message("CreateSoundBuffer");
        
        if (!SUCCEEDED(primary_buffer->SetFormat(&wave_fmt))) _exit_with_message("SetFormat");
    }
    
    // create secondary buffer
    DSBUFFERDESC buffer_desc = {};
    buffer_desc.dwSize = sizeof(buffer_desc);
    buffer_desc.dwFlags = DSBCAPS_GLOBALFOCUS;
    buffer_desc.dwBufferBytes = AUDIO_BUFFER_SIZE;
    buffer_desc.lpwfxFormat = &wave_fmt;
    if (!SUCCEEDED(dsound->CreateSoundBuffer(&buffer_desc, &g_secondary_buffer, 0)))
        _exit_with_message("CreateSoundBuffer secondary");
    
    HRESULT err = g_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);
}

global u64 sample_counter = 0;

internal void
win32_dsound_get_bytes_to_output(DWORD* byte_to_lock, DWORD* bytes_to_write, float last_frame_delta)
{
    const float tdt = .0166f;
    DWORD play_cursor, write_cursor;
    if (!SUCCEEDED(g_secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
        _exit_with_message("GetCurrentPosition");
    
    DWORD expected_bytes_per_tick =(DWORD)( float(AUDIO_SAMPLERATE * AUDIO_BYTESPERSAMPLE) * tdt);
    expected_bytes_per_tick -= expected_bytes_per_tick % AUDIO_BYTESPERSAMPLE;
    
    // x extra frames
    DWORD safety_bytes = expected_bytes_per_tick * 5;
    //safety_bytes -= safety_bytes % AUDIO_BYTESPERSAMPLE;
    
    DWORD expected_boundary = play_cursor + expected_bytes_per_tick;
    
    
    DWORD safe_write_pos = write_cursor;
    if (safe_write_pos < play_cursor)
    {
        // wrap it around
        safe_write_pos += AUDIO_BUFFER_SIZE;
    }
    else safe_write_pos += safety_bytes;
    
    DWORD target_cursor;
    if (safe_write_pos < expected_boundary)
        target_cursor = expected_boundary + expected_bytes_per_tick;
    else
        target_cursor = write_cursor + expected_bytes_per_tick + safety_bytes;
    
    target_cursor %= AUDIO_BUFFER_SIZE;
    
    *byte_to_lock = (sample_counter * AUDIO_BYTESPERSAMPLE) % AUDIO_BUFFER_SIZE;
    
    if (*byte_to_lock > target_cursor)
        *bytes_to_write = (AUDIO_BUFFER_SIZE -  *byte_to_lock) + target_cursor;
    else
        *bytes_to_write = target_cursor - *byte_to_lock;
    
}

internal void
win32_dsound_copy_to_sound_buffer(DWORD byte_to_lock, DWORD bytes_to_write)
{
    
    void* region1; DWORD region1_size;
    void* region2; DWORD region2_size;
    
    if (bytes_to_write == 0) return;
    
    HRESULT error = g_secondary_buffer->Lock(
        byte_to_lock, bytes_to_write,
        &region1, &region1_size,
        &region2, &region2_size, 0);
    
    assert(region1_size + region2_size == bytes_to_write);
    assert(bytes_to_write <= AUDIO_BUFFER_SIZE);
    if (!SUCCEEDED(error))
    {
        printf(" \t%d %d\n %x\n", byte_to_lock, bytes_to_write,  error);
        _exit_with_message("Lock failed");
    }
    
    DWORD region1_samples = region1_size / AUDIO_BYTESPERSAMPLE;
    DWORD region2_samples = region2_size / AUDIO_BYTESPERSAMPLE;
    
    i16* sample_in = (i16*)g_audio.buffer;
    i16* sample_out = (i16*) region1;
    for (DWORD sample_idx = 0; sample_idx < region1_samples; sample_idx++)
    {
        *sample_out++ = *sample_in++;
        *sample_out++ = *sample_in++;
        sample_counter++;
    }
    
    sample_out = (i16*) region2;
    for (DWORD sample_idx = 0; sample_idx < region2_samples; sample_idx++)
    {
        *sample_out++ = *sample_in++;
        *sample_out++ = *sample_in++;
        sample_counter++;
    }
    
    fail_unless(SUCCEEDED(g_secondary_buffer->Unlock(
        region1, region1_size,
        region2, region2_size)), "");
}

internal void
win32_update_and_render(HDC dc)
{
    
    QueryPerformanceCounter(&perf_now);
    i64 time_elapsed = perf_now.QuadPart - perf_last.QuadPart;
    float delta;
    if (perf_freq.QuadPart == 0) delta = 0;
    else delta = ((float)time_elapsed) / (float)perf_freq.QuadPart;
    //assert(delta >= 0);
    
    if (was_previously_moving_or_resizing &&
        !is_currently_moving_or_resizing)
    {
        delta = 0.f;
        was_previously_moving_or_resizing = false;
    }
    //fprintf(stdout, "ms : %f\n", delta);
    //delta /= 1000.f;
    
    if (delta >= 0.9f) delta = 0.9f;
    
    DWORD byte_to_lock, bytes_to_write;
    win32_dsound_get_bytes_to_output(&byte_to_lock, &bytes_to_write, delta);
    u64 samples_to_write = bytes_to_write / AUDIO_BYTESPERSAMPLE;
    
    cb_render(g_input_state, samples_to_write, delta);
    
    win32_dsound_copy_to_sound_buffer(byte_to_lock, bytes_to_write);
    
    perf_last = perf_now;
    SwapBuffers(dc);
}


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
        
        // NOTE(miked): For the blocking of moving a window bug, does not
        // happen on X11
        case WM_ENTERSIZEMOVE:
        {
            was_previously_moving_or_resizing = is_currently_moving_or_resizing;
            is_currently_moving_or_resizing = true;
        } break;
        case WM_EXITSIZEMOVE:
        {
            was_previously_moving_or_resizing = is_currently_moving_or_resizing;
            is_currently_moving_or_resizing = false;
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
    // create the actual window
    WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = win32_windproc,
        .hInstance = hInstance,
        .hCursor = LoadCursor(0, IDC_ARROW),
        .hbrBackground = 0,
        .lpszClassName = OSK_CLASS_NAME,
    };
    fail_unless(RegisterClassA(&window_class), "Failed to register class");
    
    RECT rect = {
        .right = 1024,
        .bottom = 896
    };
    // DWORD window_style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rect, window_style, false);
    
    g_wind = CreateWindowExA(
        0,
        OSK_CLASS_NAME,
        "Solomon's Key",
        window_style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        0, 0, hInstance, 0);
    fail_unless(g_wind, "Failed to create window");
}

internal void
win32_init_gl_extensions()
{
    // Before we can load extensions, we need a dummy OpenGL context, created using a dummy window.
    // We use a dummy window because you can only set the pixel format for a window once. For the
    // real window, we want to use wglChoosePixelFormatARB (so we can potentially specify options
    // that aren't available in PIXELFORMATDESCRIPTOR), but we can't load and use that before we
    // have a context.
    WNDCLASSA window_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProcA,
        .hInstance = GetModuleHandle(0),
        .lpszClassName = "Dummy_WGL_djuasiodwa",
    };
    
    if (!RegisterClassA(&window_class)) {
        inform("Failed to register dummy OpenGL window.");
    }
    
    HWND dummy_window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Dummy OpenGL Window",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        window_class.hInstance,
        0);
    
    if (!dummy_window) {
        inform("Failed to create dummy OpenGL window.");
    }
    
    HDC dummy_dc = GetDC(dummy_window);
    
    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .cDepthBits = 24,
        .cStencilBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
    };
    
    int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format) {
        inform("Failed to find a suitable pixel format.");
    }
    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd)) {
        inform("Failed to set the pixel format.");
    }
    
    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context) {
        inform("Failed to create a dummy OpenGL rendering context.");
    }
    
    if (!wglMakeCurrent(dummy_dc, dummy_context)) {
        inform("Failed to activate dummy OpenGL rendering context.");
    }
    
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
        "wglChoosePixelFormatARB");
    wglGetExtensionsStringARB = (PFNwglGetExtensionsStringARB*)wglGetProcAddress(
        "wglGetExtensionsStringARB");
    
    if (wglGetExtensionsStringARB)
    {
        char* ext_string = (char*)wglGetExtensionsStringARB(dummy_dc);
        
        printf("WGL extensions: %s\n", ext_string);
        
        
        if (wgl_is_extension_supported("WGL_EXT_swap_control", ext_string))
        {
            inform("wglSwapInterval is supported!");
            
            wglSwapIntervalEXT = (PFNwglSwapIntervalEXT*)wglGetProcAddress(
                "wglSwapIntervalEXT");
            
        }
    }
    
    
    
    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    
    
    
    
    
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
    
    
    
}


internal void
win32_init_gl(HDC real_dc)
{
    win32_init_gl_extensions();
    // Now we can choose a pixel format the modern way, using wglChoosePixelFormatARB.
    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };
    
    int pixel_format;
    UINT num_formats;
    wglChoosePixelFormatARB(real_dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats) {
        inform("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(real_dc, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(real_dc, pixel_format, &pfd)) {
        inform("Failed to set the OpenGL 3.3 pixel format.");
    }
    
    // Specify that we want to create an OpenGL 3.3 core profile context
    int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    
    HGLRC mgl_ctx = wglCreateContextAttribsARB(real_dc, 0, gl33_attribs);
    if (!mgl_ctx) {
        inform("Failed to create OpenGL 3.3 context.");
    }
    
    if (!wglMakeCurrent(real_dc, mgl_ctx)) {
        inform("Failed to activate OpenGL 3.3 rendering context.");
    }
    
    
    fail_unless(wglSwapIntervalEXT, "wglSwapIntervalEXT");
    fail_unless(wglSwapIntervalEXT(0), "swap interval set failed!");
    
    gl_load();
}


b32 win32_get_key_state(i32 key)
{
    return GetAsyncKeyState(key);
}

internal void
win32_update_all_keys()
{
#define KEYDOWN(name, _X, keysym) g_input_state.name = win32_get_key_state(keysym);
#define KEYPRESS(name, _X, keysym) { \
        b32 now = win32_get_key_state(keysym); \
        g_input_state.name[1] = (now && !g_input_state.name[0]); \
        g_input_state.name[0] = g_input_state.name[1] || now; \
        \
    } \
    
    KEYMAP
    
#undef KEYDOWN
#undef KEYPRESS
}

// THREADING

const u32 MAX_THREADS = 3;
global HANDLE win32_threads[MAX_THREADS];

struct win32_ThreadInfo
{
    u32 thread_index;
};

struct win32_WorkQueueEntry
{
    char* string_to_print;
};


#include <intrin.h>

#define COMPLETE_PAST_WRITES _WriteBarrier(); _mm_sfence()
#define COMPLETE_PAST_READS  _ReadBarrier()

global u32 volatile next_entry;
global u32 volatile entry_count;
global win32_WorkQueueEntry entries[256];

internal void push_queue_entry(char* string)
{
    win32_WorkQueueEntry* entry = entries + entry_count;
    entry->string_to_print = string;
    
    COMPLETE_PAST_WRITES;
    
    entry_count++;
    
}

DWORD test_thread_proc(void* params)
{
    win32_ThreadInfo* info = (win32_ThreadInfo*)params;
    
    while (1)
    {
        if (next_entry < entry_count)
        {
            int next_entry_index =  InterlockedIncrement(&next_entry) - 1;
            
            COMPLETE_PAST_READS;
            
            win32_WorkQueueEntry* entry = entries + next_entry_index;
            
            inform("THREAD %d: %s", info->thread_index, entry->string_to_print);
        }
    }
    
}


int WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR     lpCmdLine,
int       nShowCmd)
{
    MSG message;
    
#if 1
    AllocConsole() ;
    AttachConsole( GetCurrentProcessId() ) ;
    freopen( "CON", "w", stdout ) ;
#endif
    
    
#if 0    
    win32_ThreadInfo info[MAX_THREADS] = {};
    for (i32 i = 0; i < MAX_THREADS; ++i)
    {
        info[i].thread_index = i;
        
        DWORD thread_id;
        HANDLE thread_handle = CreateThread(
            0,0, 
            test_thread_proc,
            info + i,
            0,
            &thread_id);
    }
    
    push_queue_entry("String 0\n");
    push_queue_entry("String 1\n");
    push_queue_entry("String 2\n");
    push_queue_entry("String 3\n");
    push_queue_entry("String 4\n");
    push_queue_entry("String 5\n");
    push_queue_entry("String 6\n");
    push_queue_entry("String 7\n");
    push_queue_entry("String 8\n");
    push_queue_entry("String 9\n");
#endif
    
    
    
    win32_init(hInstance);
    HDC dc = GetDC(g_wind);
    
    win32_init_gl(dc);
    cb_init();
    
    win32_dsound_init();
    
    OutputDebugStringA("Initialized Opengl");
    
    ShowWindow(g_wind, SW_SHOW);
    UpdateWindow(g_wind);
    
    QueryPerformanceFrequency(&perf_freq);
    QueryPerformanceCounter(&perf_last);
    
    while (g_running)
    {
        while (PeekMessage(&message, g_wind, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        win32_update_all_keys();
        
        win32_update_and_render(dc);
        
    }
    
    //system("pause");
    
    return 0;
}

#undef OSK_CLASS_NAME



#include "OpenSolomonsKey.cpp"