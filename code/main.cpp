#include "base_inc.h"
#include "win32_base_inc.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

v2s32 resolution = {
    SCREEN_WIDTH,
    SCREEN_HEIGHT
};

typedef s64 GetTicks(void);
typedef f64 GetSecondsElapsed(s64 start, s64 end);
typedef f64 GetMsElapsed(s64 start, s64 end);

typedef struct Clock{
    f64 dt;
    s64 frequency;
    GetTicks* get_ticks;
    GetSecondsElapsed* get_seconds_elapsed;
    GetMsElapsed* get_ms_elapsed;
} Clock;

typedef struct Button{
    bool pressed;
    bool held;
} Button;

typedef struct Mouse{
    v2 pos;
    v2 last_pos;
    f32 dx;
    f32 dy;
    f32 wheel_direction;
    bool tracking_leave;
} Mouse;

typedef struct Controller{
    Button up;
    Button down;
    Button left;
    Button right;
    Button one;
    Button two;
    Button three;
    Button four;
    Button m_left;
    Button m_right;
    Button m_middle;
    v2 mouse_pos;
} Controller;

typedef struct RenderBuffer{
    void* base;
    size_t size;

    s32 padding;
    s32 width;
    s32 height;
    s32 bytes_per_pixel;
    s32 stride;

    BITMAPINFO bitmap_info;

    Arena* render_command_arena;
    Arena* arena;
    HDC device_context;
} RenderBuffer;

typedef struct Memory{
    void* base;
    size_t size;

    void* permanent_base;
    size_t permanent_size;
    void* transient_base;
    size_t transient_size;

    bool initialized;
} Memory;

global bool should_quit;
global RenderBuffer render_buffer;
global Controller controller;
global Memory memory;
global Clock clock;
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
init_clock(Clock* clock){
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock->frequency = frequency.QuadPart;
    clock->get_ticks = get_ticks;
    clock->get_seconds_elapsed = get_seconds_elapsed;
    clock->get_ms_elapsed = get_ms_elapsed;
}

static void
init_memory(Memory* memory){
    memory->permanent_size = MB(500);
    memory->transient_size = GB(1);
    memory->size = memory->permanent_size + memory->transient_size;
    memory->base = os_virtual_alloc(memory->size);
    memory->permanent_base = memory->base;
    memory->transient_base = (u8*)memory->base + memory->permanent_size;
}

static void
init_render_buffer(RenderBuffer* render_buffer, s32 width, s32 height){
    render_buffer->width   = width;
    render_buffer->height  = height;
    render_buffer->padding = 10;

    render_buffer->bitmap_info.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    render_buffer->bitmap_info.bmiHeader.biWidth    = width;
    render_buffer->bitmap_info.bmiHeader.biHeight   = -height;
    render_buffer->bitmap_info.bmiHeader.biPlanes   = 1;
    render_buffer->bitmap_info.bmiHeader.biBitCount = 32;
    render_buffer->bitmap_info.bmiHeader.biCompression = BI_RGB;

    s32 bytes_per_pixel = 4;
    render_buffer->bytes_per_pixel = bytes_per_pixel;
    render_buffer->stride = width * bytes_per_pixel;
    render_buffer->size   = width * height * bytes_per_pixel;
    render_buffer->base   = os_virtual_alloc(render_buffer->size);

    render_buffer->render_command_arena = make_arena(MB(16));
    render_buffer->arena = make_arena(MB(4));
}

static void
update_window(HWND window, RenderBuffer render_buffer){
    s32 padding = render_buffer.padding;

    PatBlt(render_buffer.device_context, 0, 0, render_buffer.width + padding, padding, WHITENESS);
    PatBlt(render_buffer.device_context, 0, padding, padding, render_buffer.height + padding, WHITENESS);
    PatBlt(render_buffer.device_context, render_buffer.width + padding, 0, padding, render_buffer.height + padding, WHITENESS);
    PatBlt(render_buffer.device_context, padding, render_buffer.height + padding, render_buffer.width + padding, padding, WHITENESS);

    if(StretchDIBits(render_buffer.device_context,
                     padding, padding, render_buffer.width, render_buffer.height,
                     0, 0, render_buffer.width, render_buffer.height,
                     render_buffer.base, &render_buffer.bitmap_info, DIB_RGB_COLORS, SRCCOPY))
    {
    }
    else{
        OutputDebugStringA("StrechDIBits failed\n");
    }
}

#include "game.h"

LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
    LRESULT result = 0;

    switch(message){
        case WM_PAINT:{
            PAINTSTRUCT paint;
            BeginPaint(hwnd, &paint);
            update_window(hwnd, render_buffer);
            EndPaint(hwnd, &paint);
        } break;
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            print("quiting\n");
            should_quit = true;
        } break;
        case WM_MOUSEMOVE:{
            //controller.mouse_pos.x = (s32)(l_param & 0xFFFF);
            //controller.mouse_pos.y = render_buffer.height - (s32)(l_param >> 16);
            break;
        } break;

        //case WM_LBUTTONUP:
        //case WM_RBUTTONUP:
        //case WM_MBUTTONUP:
        //case WM_LBUTTONDOWN:
        //case WM_RBUTTONDOWN:
        //case WM_MBUTTONDOWN:{
        //    print("VK_CODE: %llu\n", w_param);
        //} break;

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
        case WM_KEYDOWN: {
            Event event;
            event.type = KEYBOARD;
            event.keycode = w_param;
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = 1;
            event.alt_pressed =   alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed =  ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event;
            event.type = KEYBOARD;
            event.keycode = w_param; // TODO figure out how to use this to get the right keycode

            event.key_pressed = 0;
            event.alt_pressed =   alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed =  ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = false; }
            if(w_param == VK_SHIFT)   { shift_pressed = false; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = false; }
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

Arena* global_arena = os_make_arena(KB(1));

static HWND
win32_window_create(wchar* window_name, u32 width, u32 height){
    HWND handle = CreateWindowW(L"window class", window_name, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, GetModuleHandle(0), 0);
    assert(IsWindow(handle));

    return(handle);
}

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){

    assert(win32_init());
    HWND window = win32_window_create(L"flux", SCREEN_WIDTH + 30, SCREEN_HEIGHT + 50);

    init_memory(&memory);
    init_clock(&clock);
    init_render_buffer(&render_buffer, SCREEN_WIDTH, SCREEN_HEIGHT);
    init_events(&events);

    should_quit = false;

    f64 FPS = 0;
    f64 MSPF = 0;
    u64 frame_count = 0;

    clock.dt =  1.0/240.0;
    f64 accumulator = 0.0;
    s64 last_ticks = clock.get_ticks();
    f64 second_marker = clock.get_ticks();
	u32 simulations = 0;

    render_buffer.device_context = GetDC(window);
    f64 time_elapsed = 0;
    while(!should_quit){
        MSG message;
        while(PeekMessageW(&message, window, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        s64 now_ticks = clock.get_ticks();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        accumulator += frame_time;
        while(accumulator >= clock.dt){
            update_game(&memory, &render_buffer, &events, &controller, &clock);
            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;
        }

        frame_count++;
		simulations = 0;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_ticks(), second_marker);
        if(second_elapsed > 1){
            FPS = (frame_count / second_elapsed);
            second_marker = clock.get_ticks();
            frame_count = 0;
        }
        //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed);

        draw_commands(&render_buffer, render_buffer.render_command_arena);
        update_window(window, render_buffer);

        if(simulations){
            //handle_debug_counters(simulations);
        }
    }
    ReleaseDC(window, render_buffer.device_context);

    return(0);
}
