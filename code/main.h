#pragma comment(lib, "user32")
//#pragma comment(lib, "gdi32")
//#pragma comment(lib, "winmm")

#include "base_inc.h"
#include "win32_base_inc.h"
#define PROFILER 1
#include "profiler.h"

static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);

typedef struct Memory{
    void* base;
    size_t size;

    void* permanent_base;
    size_t permanent_size;
    void* transient_base;
    size_t transient_size;

    bool initialized;
} Memory;
global Memory memory;

// todo: Maybe this should be in a window file?
// todo: maybe this should be in win32?
struct Window{
    s32 width;
    s32 height;
    f32 aspect_ratio;
    HWND handle;
};
global Window window;

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));
global v2s32 resolution;
static String8 path_data;
