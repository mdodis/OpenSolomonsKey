/*OSK input.h
Input is defined like everything in resources.h; a #define
where you choose to sepcify a KEYPRESS or KEYDOWN action.
Implementation is left up to the corresponding platform, and
as always configuration is done at compile time.*/
#define KEYMAP \
/*       NAME        X11 key   Win32 key */\
KEYDOWN (move_right, XK_Right, VK_RIGHT  ) \
KEYDOWN (move_left , XK_Left , VK_LEFT   ) \
KEYDOWN (move_down , XK_Down , VK_DOWN   ) \
KEYPRESS(move_up   , XK_Up   , VK_UP     ) \
KEYPRESS(m_pressed , XK_M    , 'M'       ) \
KEYPRESS(cast      , XK_C    , 'C'       ) \


#define KEYDOWN(key, ...) b32 key = false;
#define KEYPRESS(key, ...) b32 key[2] = {};
struct InputState
{
    KEYMAP
};
#undef KEYDOWN
#undef KEYPRESS

#define GET_KEYPRESS(name) (b32)(g_input_state.name[1])
#define GET_KEYDOWN(name) (b32)(g_input_state.name)

#if defined(OSK_PLATFORM_X11)

b32 x11_get_key_state(i32 key);

#elif defined(OSK_PLATFORM_WIN32)

b32 win32_get_key_state(i32 key);

#else
#error "No get key state function for compatible platform"
#endif

global InputState g_input_state;
