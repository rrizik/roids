#ifndef MAIN_H
#define MAIN_H

#pragma comment(lib, "user32")

#include "base_inc.h"
#include "win32_base_inc.h"
#define PROFILER 1
#include "profiler.h"

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
static void init_memory(Memory* m);

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
struct Window{
    s32 width;
    s32 height;
    f32 aspect_ratio;
    HWND handle;
};
global Window window;
static Window win32_window_create(const wchar* window_name, s32 width, s32 height);

global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));
static String8 path_data;

static void show_cursor(bool show);
static void init_paths(Arena* arena);

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);

#endif
