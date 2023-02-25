#include "base_inc.h"
#include "win32_base_inc.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

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

typedef enum KeyCode{
    UNKOWN,

    L_MOUSE_BUTTON,
    R_MOUSE_BUTTON,
    M_MOUSE_BUTTON = 4,

    BACKSPACE = 8,
    TAB = 9,
    ENTER = 13,
    ESCAPE = 27,
    SPACEBAR = 32,

	EXCLAMATION = 33,
	QUOTATION = 34,
	OCTOTHORP = 35,
	DOLLAR = 36,
	PERCENT = 37,
	AMPERSAND = 38,
	APOSTROPHE = 39,
	OPEN_PARENTHESIS = 40,
	CLOSE_PARENTHESIS = 4,
	ASTERISK = 42,
	PLUS_SIGN = 43,
	COMMA = 44,
	HYPHEN = 45,
	PERIOD = 46,
	SLASH = 47,
	ZERO = 48,
	ONE = 49,
	TWO = 50,
	THREE = 51,
	FOUR = 52,
	FIVE = 53,
	SIX = 54,
	SEVEN = 55,
	EIGHT = 56,
	NINE = 57,
	COLON   = 58,
	SEMICOLON = 59,
	LESS_THAN = 60,
	EQUALS_TO = 61,
	GREATER_THAN = 62,
	QUESTION_MARK = 63,
	AT_SIGN = 64,
	A_UPPER = 65,
	B_UPPER = 66,
	C_UPPER = 67,
	D_UPPER = 68,
	E_UPPER = 69,
	F_UPPER = 70,
	G_UPPER = 71,
	H_UPPER = 72,
	I_UPPER = 73,
	J_UPPER = 74,
	K_UPPER = 75,
	L_UPPER = 76,
	M_UPPER = 77,
	N_UPPER = 78,
	O_UPPER = 79,
	P_UPPER = 80,
	Q_UPPER = 81,
	R_UPPER = 82,
	S_UPPER = 83,
	T_UPPER = 84,
	U_UPPER = 85,
	V_UPPER = 86,
	W_UPPER = 87,
	X_UPPER = 88,
	Y_UPPER = 89,
	Z_UPPER = 90,
	LEFT_SQUARE_BRACKET = 91,
	BACKSLASH = 92,
	RIGHT_SQUARE_BRACKET = 93,
	CARET = 94,
	UNDERSCORE = 95,
	GRAVE_ACCENT = 96,
	LOWERCASE_A = 97,
	LOWERCASE_B = 98,
	LOWERCASE_C = 99,
 	LOWERCASE_D = 100,
 	LOWERCASE_E = 101,
 	LOWERCASE_F = 102,
 	LOWERCASE_G = 103,
 	LOWERCASE_H = 104,
 	LOWERCASE_I = 105,
 	LOWERCASE_J = 106,
 	LOWERCASE_K = 107,
 	LOWERCASE_L = 108,
 	LOWERCASE_M = 119,
 	LOWERCASE_N = 110,
 	LOWERCASE_O = 111,
 	LOWERCASE_P = 112,
 	LOWERCASE_Q = 113,
 	LOWERCASE_R = 114,
 	LOWERCASE_S = 115,
 	LOWERCASE_T = 116,
 	LOWERCASE_U = 117,
 	LOWERCASE_V = 118,
 	LOWERCASE_W = 119,
 	LOWERCASE_X = 120,
 	LOWERCASE_Y = 121,
 	LOWERCASE_Z = 122,
 	LEFT_CURLY_BRACE = 123,
 	VERTICAL_BAR = 124,
 	RIGHT_CURLY_BRACE = 125,
 	TILDE = 126,
 	DEL = 127,
} KeyCode;

typedef enum EventType{
    KEYBOARD,
    TEXT_INPUT,
} EventType;

typedef struct Event{
    EventType type;
    u64 keycode;

    bool key_pressed;
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool repeat;
} Event;

typedef struct Events{
    Event e[256];
    s32 count;
} Events;

global bool global_running;
global RenderBuffer render_buffer;
global Controller controller;
global Memory memory;
global Clock clock;
global Events events;

bool alt_pressed;
bool shift_pressed;
bool ctrl_pressed;

static void
events_add(Events* events, Event event){
    events->e[++events->count] = event;
}

static void
events_flush(Events* events){
	events->count = -1;
}


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
    render_buffer->width = width;
    render_buffer->height = height;
    render_buffer->padding = 10;

    render_buffer->bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    render_buffer->bitmap_info.bmiHeader.biWidth = width;
    render_buffer->bitmap_info.bmiHeader.biHeight = -height;
    render_buffer->bitmap_info.bmiHeader.biPlanes = 1;
    render_buffer->bitmap_info.bmiHeader.biBitCount = 32;
    render_buffer->bitmap_info.bmiHeader.biCompression = BI_RGB;

    s32 bytes_per_pixel = 4;
    render_buffer->bytes_per_pixel = bytes_per_pixel;
    render_buffer->stride = width * bytes_per_pixel;
    render_buffer->size = width * height * bytes_per_pixel;
    render_buffer->base = os_virtual_alloc(render_buffer->size);

    render_buffer->render_command_arena = alloc_arena(MB(16));
    render_buffer->arena = alloc_arena(MB(4));
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
            OutputDebugStringA("quiting\n");
            global_running = false;
        } break;
        case WM_MOUSEMOVE:{
            //controller.mouse_pos.x = (s32)(l_param & 0xFFFF);
            controller.mouse_pos.y = render_buffer.height - (s32)(l_param >> 16);
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
            event.keycode = w_param; // TODO figure out how to use this to get the right keycode
            event.repeat = ((s32)l_param) & 0x40000000;

            event.key_pressed = 1;
            event.alt_pressed =   alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed =  ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
            if(w_param == VK_ESCAPE){
                OutputDebugStringA("quiting\n");
                print("%i - %i\n", VK_ESCAPE, ESCAPE);
                //global_running = false;
            } break;
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

Arena* global_arena = os_alloc_arena(KB(1));

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

    global_running = true;

    f64 FPS = 0;
    f64 MSPF = 0;
    u64 frame_count = 0;

    clock.dt =  1.0/60.0;
    f64 accumulator = 0.0;
    s64 last_ticks = clock.get_ticks();
    f64 second_marker = clock.get_ticks();
	u32 simulations = 0;

    render_buffer.device_context = GetDC(window);
    while(global_running){
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
            simulations++;
            //TODO: put in a function
            controller.up.pressed = false;
            controller.down.pressed = false;
            controller.left.pressed = false;
            controller.right.pressed = false;
            controller.one.pressed = false;
            controller.two.pressed = false;
            controller.three.pressed = false;
            controller.four.pressed = false;
            controller.m_right.pressed = false;
            controller.m_left.pressed = false;
            controller.m_middle.pressed = false;
        }

        frame_count++;
		simulations = 0;
        f64 time_elapsed = clock.get_seconds_elapsed(clock.get_ticks(), second_marker);
        if(time_elapsed > 1){
            FPS = (frame_count / time_elapsed);
            second_marker = clock.get_ticks();
            frame_count = 0;
        }
        //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - time_elapsed: %f\n", FPS, MSPF, clock.dt, accumulator, frame_time, time_elapsed);

        draw_commands(&render_buffer, render_buffer.render_command_arena);
        update_window(window, render_buffer);
        events_flush(&events);

        if(simulations){
            //handle_debug_counters(simulations);
        }
    }
    ReleaseDC(window, render_buffer.device_context);

    return(0);
}
