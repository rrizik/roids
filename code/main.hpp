#ifndef MAIN_H
#define MAIN_H

#pragma comment(lib, "user32")

#if DEBUG
#define ENABLE_ASSERT 1
#endif

#include "base_inc.h"
#include "win32_base_inc.h"
#define PROFILER 1
#include "profiler.h"

static String8 build_path;
static String8 fonts_path;
static String8 shaders_path;
static String8 saves_path;
static String8 sprites_path;
static String8 sounds_path;

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

static void show_cursor(bool show);
static void init_paths(Arena* arena);

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);

global f32 g_angle = 0;
global f32 g_angle_t = 0;

typedef enum WaveAsset{
    WaveAsset_rail_1,
    WaveAsset_rail_2,
    WaveAsset_rail_3,
    WaveAsset_rail_4,
    WaveAsset_rail_5,
    WaveAsset_rail_all,
    WaveAsset_blast_all,
    WaveAsset_track1,
    WaveAsset_track2,

    WaveAsset_Count,
} WaveAsset;
#include "input.hpp"
#include "clock.hpp"
#include "wave.h"
#include "audio.h"
#include "camera.hpp"
#include "rect.hpp"
#include "bitmap.hpp"
#include "d3d11_init.hpp"
#include "font.hpp"
#include "d3d11_render.hpp"
#include "entity.hpp"
#include "console.hpp"
#include "command.hpp"

typedef enum BitmapAsset{
    BitmapAsset_Image,
    BitmapAsset_Ship,
    BitmapAsset_Tree,
    BitmapAsset_Circle,
    BitmapAsset_Bullet,
    BitmapAsset_Test,
    BitmapAsset_Asteroid,

    BitmapAsset_Count,
} BitmapAsset;


typedef struct Assets{
    Bitmap bitmap[BitmapAsset_Count];
    Wave wave[WaveAsset_Count];
} Assets;

#define WIN_SCORE 3000
#define ENTITIES_MAX 4096
typedef struct PermanentMemory{
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

    f64 spawn_t;
} PermanentMemory, State;
global PermanentMemory* pm;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *render_command_arena;
    Arena *assets_arena;

    Assets assets;
} TransientMemory;
global TransientMemory* tm;

#include "game.hpp"
static Font global_font;

#endif
