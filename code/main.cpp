#include "main.hpp"

#include "input.cpp"
#include "clock.cpp"
#include "camera.cpp"
#include "rect.cpp"
#include "bitmap.cpp"
#include "entity.cpp"
#include "d3d11_init.cpp"
#include "d3d11_render.cpp"
#include "font.cpp"
#include "console.cpp"
#include "command.cpp"
#include "game.cpp"

static void
init_paths(Arena* arena){
    build_dir = os_application_path(global_arena);
    fonts_dir = str8_path_append(global_arena, build_dir, str8_literal("fonts"));
    shaders_dir = str8_path_append(global_arena, build_dir, str8_literal("shaders"));
    saves_dir = str8_path_append(global_arena, build_dir, str8_literal("saves"));
    sprites_dir = str8_path_append(global_arena, build_dir, str8_literal("sprites"));
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

    result.width = width;
    result.height = height;
    result.aspect_ratio = (f32)width / (f32)height;

    DWORD style = WS_OVERLAPPEDWINDOW;
    style &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rect, style, FALSE);
    s32 adjusted_w = rect.right - rect.left;
    s32 adjusted_h = rect.bottom - rect.top;

    result.handle = CreateWindowW(L"window class", window_name, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, adjusted_w, adjusted_h, 0, 0, GetModuleHandle(0), 0);
    if(!IsWindow(result.handle)){
        // todo: log error
    }
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

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    begin_profiler();

    window = win32_window_create(L"Roids", SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!window.handle){
        print("Error: Could not create window\n");
        return(0);
    }

    init_paths(global_arena);
    random_seed(0, 1);

    // D3D11 stuff
    d3d_init(window);
#if DEBUG
    d3d_init_debug_stuff();
#endif


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

    assert(sizeof(PermanentMemory) < memory.permanent_size);
    assert(sizeof(TransientMemory) < memory.transient_size);
    pm = (PermanentMemory*)memory.permanent_base;
    tm = (TransientMemory*)memory.transient_base;

    if(!memory.initialized){
        pm->game_mode = GameMode_Editor;
        show_cursor(true);

        init_camera();

        // consider: maybe move this memory stuff to init_memory()
        init_arena(&pm->arena, (u8*)memory.permanent_base + sizeof(PermanentMemory), memory.permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory.transient_base + sizeof(TransientMemory), memory.transient_size - sizeof(TransientMemory));

        tm->render_command_arena = push_arena(&tm->arena, MB(16));
        tm->frame_arena = push_arena(&tm->arena, MB(100));
        tm->assets_arena = push_arena(&tm->arena, MB(100));

        // setup free entities array in reverse order
        entities_clear(pm);

        load_font_ttf(global_arena, str8_literal("fonts/arial.ttf"), &global_font, 36);
        load_assets(tm->assets_arena, &tm->assets);

        init_texture_resource(&tm->assets.bitmaps[AssetID_Image],  &image_shader_resource);
        init_texture_resource(&tm->assets.bitmaps[AssetID_Ship],   &ship_shader_resource);
        init_texture_resource(&tm->assets.bitmaps[AssetID_Tree],   &tree_shader_resource);
        init_texture_resource(&tm->assets.bitmaps[AssetID_Circle], &circle_shader_resource);
        init_texture_resource(&tm->assets.bitmaps[AssetID_Bullet], &bullet_shader_resource);
        init_texture_resource(&tm->assets.bitmaps[AssetID_Test],   &test_shader_resource);
        init_texture_resource(&tm->assets.bitmaps[AssetID_Asteroid], &asteroid_shader_resource);

        init_console(&pm->arena);
        init_console_commands();

        pm->ship = add_ship(pm, &ship_shader_resource, make_v2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), make_v2(75, 75));
        pm->ship_loaded = true;

        memory.initialized = true;
    }


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

        arena_free(render_command_arena);
        // simulation
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            update_game(&window, &memory, &events);

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

            clear_controller_pressed(&controller);
        }

        // command arena
        push_clear_color(render_command_arena, BACKGROUND_COLOR);
        for(s32 index = 0; index < array_count(pm->entities); ++index){
            Entity *e = pm->entities + index;

            switch(e->type){
                case EntityType_Quad:{
                    v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                    v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                    //f32 deg = deg_from_dir(e->dir);
                    p0 = rotate_point_deg(p0, e->deg, e->origin);
                    p1 = rotate_point_deg(p1, e->deg, e->origin);
                    p2 = rotate_point_deg(p2, e->deg, e->origin);
                    p3 = rotate_point_deg(p3, e->deg, e->origin);

                    push_quad(render_command_arena, p0, p1, p2, p3, e->color);
                } break;
                case EntityType_Asteroid:
                case EntityType_Bullet:
                case EntityType_Ship:
                case EntityType_Texture:{
                    v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                    v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                    Rect e_rect = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                                            make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
                    push_quad(render_command_arena, e_rect.min, make_v2(e_rect.x1, e_rect.y0), e_rect.max, make_v2(e_rect.x0, e_rect.y1), ORANGE);

                    p0 = rotate_point_deg(p0, e->deg, e->pos);
                    p1 = rotate_point_deg(p1, e->deg, e->pos);
                    p2 = rotate_point_deg(p2, e->deg, e->pos);
                    p3 = rotate_point_deg(p3, e->deg, e->pos);

                    push_line(render_command_arena, p0, p1, 2, GREEN);
                    push_line(render_command_arena, p1, p2, 2, GREEN);
                    push_line(render_command_arena, p2, p3, 2, GREEN);
                    push_line(render_command_arena, p3, p0, 2, GREEN);


                    push_texture(render_command_arena, e->texture, p0, p1, p2, p3, e->color);
                    String8 text = str8_formatted(tm->frame_arena, "%i", e->index);
                    push_text(render_command_arena, global_font, text, p0.x, p0.y, RED);
                } break;
                case EntityType_Text:{
                    //push_text(render_command_arena, e->font, e->text, e->x, e->y, e->color);
                } break;
            }
        }

        String8 text = str8_formatted(tm->frame_arena, "SCORE: %i", pm->score);
        push_text(render_command_arena, global_font, text, 50, 50, BLUE);
        push_line(render_command_arena, make_v2(100, 100), make_v2(200, 200), 5, GREEN);
        push_line(render_command_arena, make_v2(100, 100), make_v2(100, 200), 5, GREEN);
        push_line(render_command_arena, make_v2(100, 100), make_v2(200, 100), 5, GREEN);
        push_line(render_command_arena, make_v2(100, 200), make_v2(200, 200), 5, GREEN);
        push_line(render_command_arena, make_v2(200, 200), make_v2(200, 100), 5, GREEN);

        // draw everything
        console_draw();
        draw_commands(render_command_arena);

        arena_free(tm->frame_arena);

        frame_count++;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_count = 0;
        }


        //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);
		simulations = 0;
    }
    d3d_release();
    end_profiler();

    return(0);
}

#include <Windowsx.h>
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
            event.mouse_pos.x = (s32)(s16)(l_param & 0xFFFF);
            event.mouse_pos.y = (s32)(s16)(l_param >> 16);

            s32 dx = event.mouse_pos.x - (SCREEN_WIDTH/2);
            s32 dy = event.mouse_pos.y - (SCREEN_HEIGHT/2);
            event.mouse_dx = (f32)dx / (f32)(SCREEN_WIDTH/2);
            event.mouse_dy = (f32)dy / (f32)(SCREEN_HEIGHT/2);
            //print("(%f, %f) - (%i, %i)\n", event.mouse_dx, event.mouse_dy, event.mouse_pos.x, event.mouse_pos.y);

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
        } break;
        default:{
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        } break;
    }
    return(result);
}
