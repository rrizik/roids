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
#define WORLD_UNITS_WIDTH 1000
#define WORLD_UNITS_HEIGHT 1000
//#define SCREEN_WIDTH 1920
//#define SCREEN_HEIGHT 1080
s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type);
static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param);
static Window win32_window_create(const wchar* window_name, u32 width, u32 height);

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
static void show_cursor(bool show);
static void init_paths(Arena* arena);

global String8 build_path;
global String8 fonts_path;
global String8 shaders_path;
global String8 saves_path;
global String8 sprites_path;
global String8 sounds_path;

global Window window;
global u64 frame_count;
global bool pause;
global bool should_quit;
global Arena* global_arena = os_make_arena(MB(100));
global Assets assets;


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

    f64 spawn_t;

} State, PermanentMemory;
global State* state;

typedef struct TransientMemory{
    Arena arena;
    Arena *frame_arena;
    Arena *asset_arena;
    Arena *ui_arena;
    Arena *hash_arena;
    Arena *batch_arena;
} TransientMemory, TState;
global TState* ts;

// todo: once I fix rendering pipeline, this can move up

//todo: get rid of this
global f32 text_padding = 20;
global bool fullscreen = false;
global bool toggle = false;

#include "game.cpp"
#endif
        //ui_begin(ts->ui_arena);

        //ui_push_background_color(ORANGE);
        //ui_push_pos_x(50);
        //ui_push_pos_y(50);
        //ui_push_size_w(ui_size_children(0));
        //ui_push_size_h(ui_size_children(0));
        //ui_push_border_thickness(10);

        //UI_Box* box1 = ui_box(str8_literal("box1"), UI_BoxFlag_DrawBackground|UI_BoxFlag_Draggable|UI_BoxFlag_Clickable);
        //ui_push_parent(box1);
        //ui_pop_border_thickness();
        //ui_pop_pos_x();
        //ui_pop_pos_y();

        //ui_push_size_w(ui_size_pixel(100, 0));
        //ui_push_size_h(ui_size_pixel(50, 0));
        //ui_push_background_color(BLUE);
        //ui_label(str8_literal("MY LAHBEL"));
        //if(ui_button(str8_literal("button 1")).pressed_left){
        //    print("button 1: PRESSED\n");
        //    wasapi_play(WaveAsset_Rail1, 0.1f, false);
        //}
        //ui_spacer(10);

        //ui_push_size_w(ui_size_pixel(50, 0));
        //ui_push_size_h(ui_size_pixel(50, 0));
        //ui_push_background_color(GREEN);
        //if(ui_button(str8_literal("button 2")).pressed_left){
        //    print("button 2: PRESSED\n");
        //}
        //ui_pop_background_color();
        //ui_pop_background_color();

        //ui_spacer(50);
        //ui_push_size_w(ui_size_children(0));
        //ui_push_size_h(ui_size_children(0));
        //ui_push_layout_axis(Axis_X);
        //ui_push_background_color(MAGENTA);
        //UI_Box* box2 = ui_box(str8_literal("box2"));
        //ui_push_parent(box2);
        //ui_pop_background_color();

        //ui_pop_size_w();
        //ui_pop_size_h();
        //ui_pop_size_w();
        //ui_pop_size_h();
        //ui_push_size_w(ui_size_pixel(100, 1));
        //ui_push_size_h(ui_size_pixel(50, 1));
        //ui_push_background_color(TEAL);
        //if(ui_button(str8_literal("button 3")).pressed_left){
        //    print("button 3: PRESSED\n");
        //}
        //ui_spacer(50);
        //ui_push_background_color(RED);
        //if(ui_button(str8_literal("button 4")).pressed_left){
        //    print("button 4: PRESSED\n");
        //}
        //ui_spacer(50);
        //ui_pop_background_color();
        //if(ui_button(str8_literal("button 5")).pressed_left){
        //    print("button 5: PRESSED\n");
        //}
        //ui_pop_parent();

        //ui_spacer(50);
        //ui_push_size_w(ui_size_children(0));
        //ui_push_size_h(ui_size_children(0));
        //ui_push_layout_axis(Axis_Y);
        //ui_push_background_color(MAGENTA);
        //UI_Box* box3 = ui_box(str8_literal("box3"));
        //ui_push_parent(box3);
        //ui_pop_background_color();

        //ui_push_size_w(ui_size_pixel(100, 0));
        //ui_push_size_h(ui_size_pixel(100, 0));
        //ui_push_background_color(YELLOW);
        //if(ui_button(str8_literal("button 6")).pressed_left){
        //    print("button 6: PRESSED\n");
        //}
        //ui_spacer(50);
        //ui_push_background_color(DARK_GRAY);
        //ui_push_size_w(ui_size_text(0));
        //ui_push_size_h(ui_size_text(0));
        //ui_push_text_padding(50);
        //if(ui_button(str8_literal("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz")).pressed_left){
        //    print("button 7: PRESSED\n");
        //}
        //ui_pop_text_padding();
        //ui_pop_parent();
