#ifndef MAIN_H
#define MAIN_H

#if DEBUG
#define ENABLE_ASSERT 1
#endif

#pragma comment(lib, "user32")
#include "base_inc.h"
#include "win32_base_inc.h"

#define PROFILER 1
#include "profiler.h"

#include "input.hpp"
#include "clock.hpp"
#include "camera.hpp"
#include "bitmap.hpp"
#include "d3d11_init.hpp"
#include "d3d11_render.hpp"
#include "font.hpp"
#include "wave.hpp"
#include "wasapi.hpp"
#include "assets.hpp"
#include "console.hpp"
#include "command.hpp"
#include "draw.hpp"
#include "ui.hpp"
#include "entity.hpp"
#include "game.hpp"

#include "input.cpp"
#include "clock.cpp"
#include "camera.cpp"
#include "bitmap.cpp"
#include "d3d11_init.cpp"
#include "font.cpp"
#include "wave.cpp"
#include "wasapi.cpp"
#include "assets.cpp"
#include "console.cpp"
#include "command.cpp"
#include "draw.cpp"
#include "ui.cpp"
#include "entity.cpp"


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);
global Window window;
static Window win32_window_create(const wchar* window_name, u32 width, u32 height);

static String8 build_path;
static String8 fonts_path;
static String8 shaders_path;
static String8 saves_path;
static String8 sprites_path;
static String8 sounds_path;
static void init_paths(Arena* arena);

typedef struct Memory{
    void* base;
    u64 size;

    void* permanent_base;
    u64 permanent_size;
    void* transient_base;
    u64 transient_size;

    bool initialized;
} Memory;
global Memory memory;
static void init_memory(u64 permanent, u64 transient);


static u64 frame_count;
global bool pause;
global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));


static void show_cursor(bool show);


#define MAX_LEVELS 3
#define MAX_LIVES 1
#define WIN_SCORE 3000
#define ENTITIES_MAX 4096
typedef struct State{
    Arena arena;
    u32 game_mode; // GameMode

    Entity entities[ENTITIES_MAX];
    u32 entities_count;

    u32 generation[ENTITIES_MAX];
    u32 free_entities[ENTITIES_MAX];
    u32 free_entities_at;

    Entity* ship;
    bool ship_loaded;
    s32 score;
    s32 lives;

    Level levels[MAX_LEVELS];
    s32 level_index;
    Level* current_level;
    Font* font;

    f32 scale;

    f64 spawn_t;

    f32 screen_top;
    f32 screen_bottom;
    f32 screen_left;
    f32 screen_right;

    //RenderBatch* render_batch;
} State, PermanentMemory;
global State* state;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;
    Arena *asset_arena;
    Arena *ui_arena;
    Arena *batch_arena;

    // todo: make this global, make more globals
    // todo: make this global, make more globals
    // todo: make this global, make more globals
    Assets assets;
} TransientMemory, TState;
global TState* ts;

// todo: once I fix rendering pipeline, this can move up
#include "d3d11_render.cpp"
#include "game.cpp"

//todo: get rid of this
global f32 text_padding = 20;
global bool fullscreen = false;
global bool toggle = false;

#endif
