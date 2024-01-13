#include "main.h"

static void
init_paths(Arena* arena){
    path_data = os_get_data_path(global_arena);
    String8 exe = os_get_exe_path(global_arena);
    u32 a = 1;
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

static Window
win32_window_create(const wchar* window_name, s32 width, s32 height){
    Window result = {0};

    WNDCLASSW window_class = {
        .style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC,
        .lpfnWndProc = win_message_handler_callback,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = L"window class",
    };

    if(!RegisterClassW(&window_class)){
        return(result);
    }

    resolution.w = width;
    resolution.h = height;


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

#include "input.h"
#include "clock.h"
#include "camera.h"
#include "math.h"
#include "rect.h"
#include "bitmap.h"
#include "entity.h"
// todo(rr): get rid of these once your done settings things up
static Entity* first;
static Entity* second;
static Entity* third;

static Bitmap image;
static Bitmap ship;
static Bitmap tree;
static Bitmap circle;
static Bitmap bullet;
static Bitmap test;

//#include "d3d11_init.h"
//#include "d3d11_render.h"
//#include "font.h"
#include "d3d11_init.h"
#include "font.h"
#include "d3d11_render.hpp"
#include "console.h"

#include "d3d11_init.cpp"
#include "d3d11_render.cpp"
#include "font.cpp"
#include "console.cpp"
#include "game.h"

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    begin_profiler();

    Window window = win32_window_create(L"Roids", SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!window.handle){ return(0); }

    init_paths(global_arena);
    random_seed(0, 1);

    // D3D11 stuff
    d3d_init(window);
#if DEBUG
    d3d_init_debug_stuff();
#endif

    image  = load_bitmap(global_arena, str8_literal("sprites\\image.bmp"));
    ship   = load_bitmap(global_arena, str8_literal("sprites\\ship.bmp"));
    tree   = load_bitmap(global_arena, str8_literal("sprites\\tree00.bmp"));
    circle = load_bitmap(global_arena, str8_literal("sprites\\circle.bmp"));
    bullet = load_bitmap(global_arena, str8_literal("sprites\\bullet4.bmp"));
    test   = load_bitmap(global_arena, str8_literal("sprites\\test.bmp"));

    init_texture_resource(&image, &image_shader_resource);
    init_texture_resource(&ship, &ship_shader_resource);
    init_texture_resource(&tree, &tree_shader_resource);
    init_texture_resource(&circle, &circle_shader_resource);
    init_texture_resource(&bullet, &bullet_shader_resource);
    init_texture_resource(&test, &test_shader_resource);

    Font font;
    load_font_ttf(global_arena, str8_literal("fonts/arial.ttf"), &font, 36);

    init_memory(&memory);
    init_clock(&clock);
    events_init(&events);

    f64 FPS = 0;
    f64 MSPF = 0;
    u64 frame_count = 0;
	u32 simulations = 0;
    f64 time_elapsed = 0;
    f64 accumulator = 0.0;

    clock.dt =  1.0/240.0;
    u64 last_ticks = clock.get_os_timer();
    u64 frame_tick_start = clock.get_os_timer();

    should_quit = false;
    while(!should_quit){
        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        u64 now_ticks = clock.get_os_timer();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        // simulation
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            update_game(&window, &memory, &events, &clock);

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

            clear_controller_pressed(&controller);
        }

        // draw everything
        if(memory.initialized){
            //draw_everything();
            d3d_clear_color(BACKGROUND_COLOR);


            String8 text = str8_literal("! \"#$%'()*+,-x/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghujklmnopqrstuvwxyz{|}~");
            f32 ypos = 0.2f * (f32)resolution.h;
            d3d_draw_text(font, 10.0f, ypos, ORANGE, text);

            d3d_draw_textured_cube_instanced(&image_shader_resource);


            d3d_draw_quad_textured(0.0f, 0.0f, 0.5f, 0.5f, RED, &white_shader_resource);
            d3d_draw_quad_textured(0.0f, 0.0f, 0.4f, 0.4f, BLUE, &ship_shader_resource);
            d3d_draw_quad_textured(0.0f, 0.0f, 0.2f, 0.2f, GREEN, &white_shader_resource);
            d3d_draw_quad_textured(-0.5f, -0.5f, 0.0f, 0.0f, RED, &ship_shader_resource);
            d3d_draw_quad_textured(-0.5f, -0.5f, -0.2f, -0.2f, BLUE, &ship_shader_resource);
            d3d_draw_quad_textured(-0.5f, -0.5f, -0.4f, -0.4f, GREEN, &ship_shader_resource);
            if(console_is_visible()){
                draw_console();
            }
        }
        d3d_present();

        frame_count++;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_count = 0;
        }
        print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);
		simulations = 0;
    }
    d3d_release();
    end_profiler();

    return(0);
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
