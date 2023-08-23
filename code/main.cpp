#pragma comment(lib, "user32")
//#pragma comment(lib, "gdi32")
//#pragma comment(lib, "winmm")

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#include "base_inc.h"
#include "win32_base_inc.h"
#include "directx.h"

global v2s32 resolution = {
    .x = SCREEN_WIDTH,
    .y = SCREEN_HEIGHT
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

typedef struct Memory{
    void* base;
    size_t size;

    void* permanent_base;
    size_t permanent_size;
    void* transient_base;
    size_t transient_size;

    bool initialized;
} Memory;


global Arena* global_arena = os_make_arena(MB(1));
global bool should_quit;
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
            event.mouse_pos.x = (l_param & 0xFFFF); //- render_buffer.padding;
            event.mouse_pos.y = (SCREEN_HEIGHT - (s32)(l_param >> 16)); //+ render_buffer.padding; // (0, 0) bottom left

            event.mouse_dx = event.mouse_pos.x - last_mouse_x;
            event.mouse_dy = event.mouse_pos.y - last_mouse_y;
            //print("x: %i, y: %i, dx: %i, dy: %i\n", event.mouse_pos.x, event.mouse_pos.y, event.mouse_dx, event.mouse_dy);

            last_mouse_x = event.mouse_pos.x;
            last_mouse_y = event.mouse_pos.y;
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

struct Window{
    u32 width;
    u32 height;
    HWND handle;
};

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
win32_window_create(wchar* window_name, u32 width, u32 height){
    Window result = {0};
    result.width = width;
    result.height = height;
    result.handle = CreateWindowW(L"window class", window_name, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, result.width, result.height, 0, 0, GetModuleHandle(0), 0);
    assert(IsWindow(result.handle));

    return(result);
}

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
    HRESULT result;
    d3d_create_device_and_context();
    d3d_create_swapchain(window.handle);

    d3d_create_framebuffer();
    d3d_create_depthbuffer();
    d3d_create_depthstencil();

    d3d_create_rasterizer_state();
    d3d_create_sampler_state();
    d3d_set_viewport();

    d3d_load_vertex_shader(path_shaders);
    d3d_load_pixel_shader(path_shaders);

    d3d_create_vertex_buffer();
    d3d_create_index_buffer();
    //d3d_set_vertex_buffer(cube, array_count(cube));
    //d3d_set_index_buffer(indicies, array_count(indicies));
    d3d_create_constant_buffer();


    d3d_context->VSSetShader(vertex_shader, 0, 0);
    d3d_context->PSSetShader(pixel_shader, 0, 0);
    d3d_context->PSSetConstantBuffers(0, 1, &ps_constant_buffer);
    d3d_context->IASetVertexBuffers(0, 1, &vertex_buffer, &vertex_stride, &vertex_offset);
    d3d_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

    d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context->OMSetRenderTargets(1, &d3d_framebuffer_view, d3d_depthbuffer_view);
    d3d_context->OMSetBlendState(0, 0, 0xffffffff);


    init_memory(&memory);
    init_clock(&clock);
    init_events(&events);


    //f64 FPS = 0;
    //f64 MSPF = 0;
    //u64 frame_count = 0;

    clock.dt =  1.0/240.0;
    f64 accumulator = 0.0;
    s64 last_ticks = clock.get_ticks();
    s64 second_marker = clock.get_ticks();

	u32 simulations = 0;
    f64 time_elapsed = 0;

    should_quit = false;
    while(!should_quit){
        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        s64 now_ticks = clock.get_ticks();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        //MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        accumulator += frame_time;
        while(accumulator >= clock.dt){
            update_game(&memory, &events, &clock);
            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;
        }

        f32 aspect_ratio = (f32)SCREEN_HEIGHT / (f32)SCREEN_WIDTH;
        //f32 aspect_ratio = (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT;

        d3d_create_constant_buffer();

        f32 background_color[4] = {0.2f, 0.29f, 0.29f, 1.0f};
        d3d_context->ClearRenderTargetView(d3d_framebuffer_view, background_color);
        d3d_context->ClearDepthStencilView(d3d_depthbuffer_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
        if(memory.initialized){
        for(u32 entity_index = (u32)pm->free_entities_at; entity_index < array_count(pm->entities); ++entity_index){
            Entity *e = pm->entities + pm->free_entities[entity_index];

            switch(e->type){
                case EntityType_Cube:{
                    print("index: %i\n", e->index);
                    e->angle += (f32)clock.dt;

                    Mesh mesh = pm->meshes[EntityType_Cube];
                    d3d_context->IASetVertexBuffers(0, 1, &mesh.vertex_buffer, &vertex_stride, &vertex_offset);
                    d3d_context->IASetIndexBuffer(mesh.index_buffer, DXGI_FORMAT_R32_UINT, 0);
                    print("z mouse: %f\n", (f32)controller.mouse_pos.y/SCREEN_HEIGHT * 10);
                    print("x: %f - y: %f - z: %f\n", e->pos.x, e->pos.y, e->pos.y);

                    if(e->index == 120){
                        vs_cb.transform = XMMatrixTranspose(
                            XMMatrixRotationZ(e->angle) *
                            XMMatrixRotationY(e->angle) *
                            XMMatrixScaling(e->scale.x, e->scale.y, e->scale.z) *
                            XMMatrixTranslation(e->pos.x, e->pos.y, e->pos.z) *
                            XMMatrixPerspectiveLH(1.0f, aspect_ratio, 0.5f, 20.0f)
                        );
                        d3d_context->VSSetConstantBuffers(0, 1, &vs_constant_buffer);
                    }
                    if(e->index == 121){
                        vs_cb2.transform = XMMatrixTranspose(
                            XMMatrixRotationZ(e->angle) *
                            XMMatrixRotationX(e->angle) *
                            XMMatrixScaling(e->scale.x, e->scale.y, e->scale.z) *
                            XMMatrixTranslation(e->pos.x, e->pos.y, (f32)controller.mouse_pos.y/SCREEN_HEIGHT * 10) *
                            XMMatrixPerspectiveLH(1.0f, aspect_ratio, 0.5f, 20.0f)
                        );
                        d3d_context->VSSetConstantBuffers(0, 1, &vs_constant_buffer2);
                    }

                    d3d_context->DrawIndexed(index_count, 0, 0);
                } break;
            }
        }
        }
        d3d_swapchain->Present(1, 0);

        //POINT p = {controller.mouse_pos.x, controller.mouse_pos.y};
        //ScreenToClient(window.handle, &p);
        //vs_cb.transform = XMMatrixTranspose(
        //        XMMatrixRotationZ(angle) *
        //        XMMatrixRotationX(angle) *
        //        XMMatrixTranslation(0.0f, 0.0f, (f32)controller.mouse_pos.y/SCREEN_HEIGHT * 10) *
        //        //XMMatrixTranslation(0.0f, 0.0f, 8.0f) *
        //        XMMatrixPerspectiveLH(1.0f, aspect_ratio, 0.5f, 20.0f)
        //);
        //vs_cb2.transform = XMMatrixTranspose(
        //        XMMatrixRotationZ(angle) *
        //        XMMatrixRotationY(angle) *
        //        XMMatrixScaling(0.5f, 0.5f, 0.5f) *
        //        //XMMatrixTranslation((f32)p.x/SCREEN_WIDTH, (f32)p.y/SCREEN_HEIGHT, 10.0f) *
        //        XMMatrixTranslation(0.0f, 0.0f, 4.0f) *
        //        XMMatrixPerspectiveLH(1.0f, aspect_ratio, 0.5f, 20.0f)
        //);
        //print("%i - %i\n", controller.mouse_pos.x, controller.mouse_pos.y);
        //print("n:%f - n:%f\n", (f32)controller.mouse_pos.x/SCREEN_WIDTH * 3, (f32)controller.mouse_pos.y/SCREEN_HEIGHT * 3);
        ////print("%i - %i\n", p.x, p.y);


        //// NOTE: Clear screen to color
        //f32 background_color[4] = {0.2f, 0.29f, 0.29f, 1.0f};
        //d3d_context->ClearRenderTargetView(d3d_framebuffer, background_color);
        //d3d_context->ClearDepthStencilView(d3d_depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);


        // QUESTION: Does this stuff have to happen every time? I feel like no

        //d3d_context->VSSetConstantBuffers(0, 1, &vs_constant_buffer);
        //d3d_context->DrawIndexed(index_count, 0, 0);

        //d3d_context->IASetVertexBuffers(0, 1, &vertex_buffer2, &vertex_stride, &vertex_offset);
        //d3d_context->IASetIndexBuffer(index_buffer2, DXGI_FORMAT_R32_UINT, 0);
        //d3d_context->VSSetConstantBuffers(0, 1, &vs_constant_buffer2);
        //d3d_context->DrawIndexed(index_count, 0, 0);
        //d3d_context->Draw(array_count(verticies), 0);


        //frame_count++;
		//simulations = 0;
        //f64 second_elapsed = clock.get_seconds_elapsed(clock.get_ticks(), second_marker);
        //if(second_elapsed > 1){
        //    FPS = ((f64)frame_count / second_elapsed);
        //    second_marker = clock.get_ticks();
        //    frame_count = 0;
        //}
        //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed);
    }

    return(0);
}

