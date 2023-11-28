#pragma comment(lib, "user32")
//#pragma comment(lib, "gdi32")
//#pragma comment(lib, "winmm")

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720


#include "base_inc.h"
#include "win32_base_inc.h"
struct Window{ // todo: maybe this should be in win32
    s32 width;
    s32 height;
    f32 aspect_ratio;
    HWND handle;
};
global Window window;

global HRESULT hr; //todo: add this in win32_base_inc?
#define PROFILER 1
#include "profiler.h"
#include "camera.h"
#include "math.h"
#include "rect.h"
#include "bitmap.h"
#include "font.h"
#include "entity.h"
global Arena* global_arena = os_make_arena(MB(100));

static String8 path_exe;
static String8 path_cwd;
static String8 path_data;
static String8 path_sprites;
static String8 path_fonts;
static String8 path_saves;
static String8 path_shaders;
static void
init_paths(Arena* arena){
    path_exe = os_get_exe_path(global_arena);

    ScratchArena scratch = begin_scratch(0);
    String8Node split_exe_path = str8_split(scratch.arena, path_exe, '\\');
    dll_pop_back(&split_exe_path);
    dll_pop_back(&split_exe_path);
    String8Join opts = { .mid = str8_literal("\\"), };
    path_cwd = str8_join(global_arena, &split_exe_path, opts);
    end_scratch(scratch);

    path_data    = str8_path_append(arena, path_cwd, str8_literal("data"));
    path_sprites = str8_path_append(arena, path_data, str8_literal("sprites"));
    path_fonts   = str8_path_append(arena, path_data, str8_literal("fonts"));
    path_saves   = str8_path_append(arena, path_data, str8_literal("saves"));
    path_shaders = str8_path_append(arena, path_data, str8_literal("shaders"));
}

#include "d3d11_init.h"
#include "d3d11_render.h"

global v2s32 resolution = {
    .x = SCREEN_WIDTH,
    .y = SCREEN_HEIGHT
};

typedef s64 GetTicks(void);
typedef f64 GetSecondsElapsed(s64 start, s64 end);
typedef f64 GetMSElapsed(s64 start, s64 end);

typedef struct Clock{
    f64 dt;
    s64 frequency;
    GetTicks* get_ticks;
    GetSecondsElapsed* get_seconds_elapsed;
    GetMSElapsed* get_ms_elapsed;
} Clock;
global Clock clock;

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


global bool should_quit;
#include "input.h"
global Events events;

static s64 get_ticks(){
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return(result.QuadPart);
}

static f64 get_seconds_elapsed(s64 end, s64 start){
    f64 result;
    result = ((f64)(end - start) / ((f64)clock.frequency));
    return(result);
}

static f64 get_ms_elapsed(s64 start, s64 end){
    f64 result;
    result = (1000 * ((f64)(end - start) / ((f64)clock.frequency)));
    return(result);
}

static void
init_clock(Clock* c){
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    c->frequency = frequency.QuadPart;
    c->get_ticks = get_ticks;
    c->get_seconds_elapsed = get_seconds_elapsed;
    c->get_ms_elapsed = get_ms_elapsed;
}

static void
init_memory(Memory* m){
    m->permanent_size = MB(500);
    m->transient_size = GB(1);
    m->size = m->permanent_size + m->transient_size;
    m->base = os_virtual_alloc(m->size);
    m->permanent_base = m->base;
    m->transient_base = (u8*)m->base + m->permanent_size;
}


static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
    LRESULT result = 0;

    switch(message){
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event;
            event.type = QUIT;
            events_add(&events, event);
        } break;

        // TODO: mouse_pos now is probably wrong, fix it
        case WM_MOUSEMOVE:{
            Event event;
            event.type = MOUSE; // TODO: maybe have this be a KEYBOARD event
            event.mouse_pos.x = (s32)(l_param & 0xFFFF);
            event.mouse_pos.y = (SCREEN_HEIGHT - (s32)(l_param >> 16));

            s32 dx = event.mouse_pos.x - (SCREEN_WIDTH/2);
            s32 dy = event.mouse_pos.y - (SCREEN_HEIGHT/2);
            event.mouse_dx = (f32)dx / (f32)(SCREEN_WIDTH/2);
            event.mouse_dy = (f32)dy / (f32)(SCREEN_HEIGHT/2);

            events_add(&events, event);
        } break;

        case WM_MOUSEWHEEL:{
            Event event;
            event.type = KEYBOARD;
            event.mouse_wheel_dir = GET_WHEEL_DELTA_WPARAM(w_param) > 0? 1 : -1;
            events_add(&events, event);
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_LEFT;

            bool pressed = false;
            if(message == WM_LBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            events_add(&events, event);
        } break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_RIGHT;

            bool pressed = false;
            if(message == WM_RBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            events_add(&events, event);
        } break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_MIDDLE;

            bool pressed = false;
            if(message == WM_MBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            events_add(&events, event);
        } break;

        case WM_CHAR:{
            u64 keycode = w_param;

            if(keycode > 31){
                Event event;
                event.type = TEXT_INPUT;
                event.keycode = keycode;
                events_add(&events, event);
            }

        } break;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = w_param;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = 1;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            //print("keycode: %i, repeat: %i, pressed: %i, s: %i, c: %i, a: %i\n", event.keycode, event.repeat, event.key_pressed, event.shift_pressed, event.ctrl_pressed, event.alt_pressed);
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = w_param; // TODO figure out how to use this to get the right keycode

            event.key_pressed = 0;
            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = false; }
            if(w_param == VK_SHIFT)   { shift_pressed = false; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = false; }
            //print("keycode: %i, repeat: %i, pressed: %i, s: %i, c: %i, a: %i\n", event.keycode, event.repeat, event.key_pressed, event.shift_pressed, event.ctrl_pressed, event.alt_pressed);
        } break;
        default:{
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        } break;
    }
    return(result);
}

static bool
win32_init(){
    WNDCLASSW window_class = {
        .style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC,
        .lpfnWndProc = win_message_handler_callback,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = L"window class",
    };

    if(RegisterClassW(&window_class)){
        return(true);
    }

    return(false);
}

static Window
win32_window_create(wchar* window_name, s32 width, s32 height){
    Window result = {0};
    result.width = width;
    result.height = height;
    result.aspect_ratio = (f32)width / (f32)height;
    result.handle = CreateWindowW(L"window class", window_name, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, (s32)result.width, (s32)result.height, 0, 0, GetModuleHandle(0), 0);
    assert(IsWindow(result.handle));

    return(result);
}

static void
show_cursor(bool show){
    if(show){
        while(ShowCursor(1) < 0);
    }
    else{
        while(ShowCursor(0) >= 0);
    }
}

static Entity* first;
static Entity* second;
static Entity* third;
#include "game.h"
static f32 cangle = 0;
static HRESULT hresult;
s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){

    bool succeed = win32_init();
    assert(succeed);

    init_paths(global_arena);

    random_seed(0, 1);

    Window window = win32_window_create(L"Roids", SCREEN_WIDTH, SCREEN_HEIGHT);

    // D3D11 stuff
    d3d_init(window);
#if DEBUG
    d3d_init_debug_stuff();
#endif

    init_memory(&memory);
    init_clock(&clock);
    init_events(&events);

    f64 FPS = 0;
    f64 MSPF = 0;
    u64 frame_count = 0;

    clock.dt =  1.0/240.0;
    f64 accumulator = 0.0;
    s64 last_ticks = clock.get_ticks();
    s64 second_marker = clock.get_ticks();

	u32 simulations = 0;
    f64 time_elapsed = 0;

    should_quit = false;
    begin_profiler();
    while(!should_quit){
        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }


        s64 now_ticks = clock.get_ticks();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        // simulation
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            begin_timed_scope("simulation");

            update_game(&window, &memory, &events, &clock);
            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;
            clear_controller_pressed(&controller);
        }

        // draw everything
        if(memory.initialized){
            begin_timed_scope("draw");
            //draw_everything();
            d3d_clear_color(BACKGROUND_COLOR);

            d3d_draw_cube_texture_instanced(&second->texture);

            d3d_draw_quad(-0.0f, -0.0f, 0.5f, 0.5f, BLUE);

            {
                begin_timed_scope("draw_textured_quad_shader_loading");
                d3d_load_shader(str8_literal("2d_texture_shader.hlsl"), d3d_2dui_texture_input_layout, 3);
            }
            if(console_is_visible()){
                begin_timed_scope("draw_console");
                draw_console();
            }

            String8 ship_str   = str8_literal("ship_simple.bmp");
            Bitmap ship_image = load_bitmap(&tm->arena, path_sprites, ship_str);
            d3d_draw_textured_quad(-0.5f, -0.5f, 0.0f, 0.0f, &ship_image);
            //print("%i\n", simulations);
            simulations = 0;
        }

        d3d_present();

        frame_count++;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_ticks(), second_marker);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            second_marker = clock.get_ticks();
            frame_count = 0;
        }
        print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed);
    }
    end_profiler();

    return(0);
}

