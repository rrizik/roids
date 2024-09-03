#include "main.hpp"

WINDOWPLACEMENT window_info = { sizeof(WINDOWPLACEMENT) };
static void
os_resize_window(HWND hwnd){
    s32 style = GetWindowLong(hwnd, GWL_STYLE);

    if(style & WS_OVERLAPPEDWINDOW){ // is windows mode?
        MONITORINFO monitor_info = { sizeof(MONITORINFO) };

        if(GetWindowPlacement(hwnd, &window_info) &&
           GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &monitor_info)){
            SetWindowLong(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else{
        SetWindowLong(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &window_info);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

static void
change_resolution(HWND hwnd, f32 width, f32 height, bool is_fullscreen) {
    s32 style = GetWindowLong(hwnd, GWL_STYLE);

    if (is_fullscreen) {
        d3d_resize_window(width, height);
    } else {
        RECT rect = {0, 0, (s32)width, (s32)height};
        AdjustWindowRect(&rect, (DWORD)style, FALSE);

        SetWindowPos(hwnd, NULL, 0, 0,
                     rect.right - rect.left,
                     rect.bottom - rect.top,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        d3d_resize_window(width, height);
    }
}

static void
init_paths(Arena* arena){
    build_path = os_application_path(global_arena);
    fonts_path = str8_path_append(global_arena, build_path, str8_literal("fonts"));
    shaders_path = str8_path_append(global_arena, build_path, str8_literal("shaders"));
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
    sprites_path = str8_path_append(global_arena, build_path, str8_literal("sprites"));
    sounds_path = str8_path_append(global_arena, build_path, str8_literal("sounds"));
}

static void
init_memory(u64 permanent, u64 transient){
    memory.permanent_size = permanent;
    memory.transient_size = transient;
    memory.size = memory.permanent_size + memory.transient_size;

    memory.base = os_alloc(memory.size);
    memory.permanent_base = memory.base;
    memory.transient_base = (u8*)memory.base + memory.permanent_size;
}

static Window
win32_window_create(const wchar* window_name, u32 width, u32 height){
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

    result.width = (f32)width;
    result.height = (f32)height;

    // adjust window size to exclude client area
    DWORD style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    style = style & ~WS_MAXIMIZEBOX; // disable maximize button
    RECT rect = { 0, 0, (s32)width, (s32)height };
    AdjustWindowRect(&rect, style, FALSE);
    s32 adjusted_w = rect.right - rect.left;
    s32 adjusted_h = rect.bottom - rect.top;

    result.handle = CreateWindowW(L"window class", window_name, style, CW_USEDEFAULT, CW_USEDEFAULT, adjusted_w, adjusted_h, 0, 0, GetModuleHandle(0), 0);
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

static LRESULT win_message_handler_callback(HWND hwnd, u32 message, u64 w_param, s64 l_param){
    LRESULT result = 0;

    switch(message){
        //case WM_SIZE:{
            //resize_window(hwnd, 0, 0);
        //} break;
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event = {0};
            event.type = QUIT;
            events_add(&events, event);
        } break;

        case WM_NCHITTEST:{ // prevent resizing on edges
            LRESULT hit = DefWindowProcW(hwnd, message, w_param, l_param);
            if (hit == HTLEFT       || hit == HTRIGHT || // edges of window
                hit == HTTOP        || hit == HTBOTTOM ||
                hit == HTTOPLEFT    || hit == HTTOPRIGHT || // corners of window
                hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT){
                return HTCLIENT;
            }
            return hit;
        } break;

        case WM_CHAR:{
            u64 keycode = w_param;

            if(keycode > 31){
                Event event = {0};
                event.type = TEXT_INPUT;
                event.keycode = keycode;
                event.repeat = ((s32)l_param) & 0x40000000;

                event.shift_pressed = shift_pressed;

                events_add(&events, event);

                if(w_param == VK_SHIFT)   { shift_pressed = true; }
            }

        } break;

        case WM_MOUSEMOVE:{
            Event event = {0};
            event.type = MOUSE;
            event.mouse_x = (f32)((s16)(l_param & 0xFFFF));
            event.mouse_y = (f32)((s16)(l_param >> 16));

            // calc dx/dy and normalize from -1:1
            f32 dx = event.mouse_x - controller.mouse.x;
            f32 dy = event.mouse_y - controller.mouse.y;
            v2 delta_normalized = normalize_v2(make_v2(dx, dy));
            event.mouse_dx = delta_normalized.x;
            event.mouse_dy = delta_normalized.y;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;

        case WM_MOUSEWHEEL:{
            Event event = {0};
            event.type = KEYBOARD;
            event.mouse_wheel_dir = GET_WHEEL_DELTA_WPARAM(w_param) > 0? 1 : -1;
            if(event.mouse_wheel_dir > 0){
                event.keycode = MOUSE_WHEEL_UP;
            }
            else{
                event.keycode = MOUSE_WHEEL_DOWN;
            }

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;

        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        // TODO: IMPORTANT: These events likely pass in mouse positions, need to store them as part of the event
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_LEFT;

            bool pressed = false;
            if(message == WM_LBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_RIGHT;

            bool pressed = false;
            if(message == WM_RBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:{
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = MOUSE_BUTTON_MIDDLE;
            event.repeat = ((s32)l_param) & 0x40000000;

            bool pressed = false;
            if(message == WM_MBUTTONDOWN){ pressed = true; }
            event.key_pressed = pressed;

            event.alt_pressed   = alt_pressed;
            event.shift_pressed = shift_pressed;
            event.ctrl_pressed  = ctrl_pressed;

            events_add(&events, event);

            if(w_param == VK_MENU)    { alt_pressed   = true; }
            if(w_param == VK_SHIFT)   { shift_pressed = true; }
            if(w_param == VK_CONTROL) { ctrl_pressed  = true; }
        } break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:{
            Event event = {0};
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
            Event event = {0};
            event.type = KEYBOARD;
            event.keycode = w_param;

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

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    begin_profiler();

    window = win32_window_create(L"Roids", SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!window.handle){
        print("Error: Could not create window\n");
        return(0);
    }

    init_d3d(window.handle, (u32)window.width, (u32)window.height);
#if DEBUG
    d3d_init_debug_stuff();
#endif

    random_seed(0, 1);

    init_paths(global_arena);
    init_memory(MB(500), GB(1));
    init_clock(&clock);
    init_wasapi(2, 48000, 32);
    init_events(&events);


    // note: sim measurements
	u32 simulations = 0;
    f64 time_elapsed = 0;
    f64 accumulator = 0.0;

    clock.dt =  1.0/240.0;
    u64 last_ticks = clock.get_os_timer();

    // note: fps measurement
    f64 FPS = 0;
    f64 MSPF = 0;
    u64 frame_inc = 0;
    u64 frame_tick_start = clock.get_os_timer();

    assert(sizeof(PermanentMemory) < memory.permanent_size);
    assert(sizeof(TransientMemory) < memory.transient_size);
    state = (PermanentMemory*)memory.permanent_base;
    ts    = (TransientMemory*)memory.transient_base;

    if(!memory.initialized){
        // consider: maybe move this memory stuff to init_memory()
        init_arena(&state->arena, (u8*)memory.permanent_base + sizeof(PermanentMemory), memory.permanent_size - sizeof(PermanentMemory));
        init_arena(&ts->arena, (u8*)memory.transient_base + sizeof(TransientMemory), memory.transient_size - sizeof(TransientMemory));

        ts->frame_arena = push_arena(&ts->arena, MB(100));
        ts->asset_arena = push_arena(&ts->arena, MB(100));
        ts->ui_arena = push_arena(&ts->arena, MB(100));
        ts->batch_arena = push_arena(&ts->arena, MB(100));

        state->game_mode = GameMode_Game;

        show_cursor(true);
        load_assets(ts->asset_arena, &assets);

        init_camera();
        init_console(global_arena, &window, &assets);
        init_ui(&state->arena, &window, &controller, &assets);
        init_render_commands(ts->batch_arena, &assets);

        state->font = &assets.fonts[FontAsset_Arial];

        // setup free entities array in reverse order
        entities_clear();

        //wasapi_play(&assets.waves[WaveAsset_Track1], 0.1f, true);
        //wasapi_play(&assets.waves[WaveAsset_Track5], 0.0f, true);
        //wasapi_play(&assets.waves[WaveAsset_Track4], 0.0f, true);

        half_screen = make_v2(window.width/2, window.height/2);
        state->scale = get_scale(&window);
        state->ship = add_ship(TextureAsset_Ship, make_v2(0, 0), make_v2(50, 50));
        //state->ship = add_ship(TextureAsset_Ship, makmake_v2(controller->mouse.x,e_v2(window.width/2, window.height/2), make_v2(30, 30));
        state->ship_loaded = true;
        state->lives = 1;

        state->level_index = 0;
        init_levels();
        state->current_level = &state->levels[state->level_index];

        state->screen_top = -window.height/2;
        state->screen_bottom = window.height/2;
        state->screen_left = -window.width/2;
        state->screen_right = window.width/2;

        memory.initialized = true;
    }

    should_quit = false;
    while(!should_quit){
        begin_timed_scope("while(!should_quit)");

        v2 screen_pos1 = screen_from_world(state->ship->pos, half_screen, state->scale);
        v2 screen_pos2 = screen_from_world_space(state->ship->pos, half_screen, state->scale);
        print("new_scaled: (%f, %f) - new (%f, %f)\n", screen_pos1.x, screen_pos1.y, screen_pos1.x/state->scale, screen_pos1.y/state->scale);
        print("old_scaled: (%f, %f) - old (%f, %f)\n", screen_pos2.x * state->scale, screen_pos2.y * state->scale, screen_pos2.x, screen_pos2.y);
        print("------------------------------------\n");

        u64 now_ticks = clock.get_os_timer();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        // NOTE: process events.
        bool handled;
        while(!events_empty(&events)){
            Event event = events_next(&events);

            handled = handle_global_events(event);

            if(console_is_open()){
                handled = handle_console_events(event);
                continue;
            }
            //handled = handle_camera_events(event);
            handled = handle_controller_events(event);
            handled = handle_game_events(event);

            if(event.type == KEYBOARD){
                if(event.key_pressed){
                    if(event.keycode == KeyCode_ONE){
                        toggle = !toggle;
                        if(toggle){
                            change_resolution(window.handle, 1920, 1080, false);
                        }
                        else{
                            change_resolution(window.handle, SCREEN_WIDTH, SCREEN_HEIGHT, false);
                        }
                    }
                    if(event.keycode == KeyCode_Q){
                        //os_resize_window(window.handle);
                        //fullscreen = !fullscreen;
                        //if(fullscreen){
                        //    d3d_resize_window(1920, 1080);
                        //    window.width = 1920.0f;
                        //    window.height = 1080.0f;
                        //}
                        //else{
                        //    d3d_resize_window(SCREEN_WIDTH, SCREEN_HEIGHT);
                        //    window.width = SCREEN_WIDTH;
                        //    window.height = SCREEN_HEIGHT;
                        //}

                        //state->screen_top = -window.height/2;
                        //state->screen_bottom = window.height/2;
                        //state->screen_left = -window.width/2;
                        //state->screen_right = window.width/2;
                    }
                }
            }
        }

        //----constant buffer----
        D3D11_MAPPED_SUBRESOURCE mapped_subresource;
        d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
        ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
        constants->screen_res = make_v2s32((s32)window.width, (s32)window.height);
        d3d_context->Unmap(d3d_constant_buffer, 0);

        //draw_clear_color(BACKGROUND_COLOR);
        d3d_clear_color(BACKGROUND_COLOR);
        console_update();
        if(state->game_mode == GameMode_Game){

            begin_timed_scope("GameMode_Game");
            if(!pause){
                simulations = 0;
                accumulator += frame_time;
                while(accumulator >= clock.dt){
                    update_game();

                    accumulator -= clock.dt;
                    time_elapsed += clock.dt;
                    simulations++;

                }
            }

            // todo: also use flags here
            for(s32 index = 0; index < array_count(state->entities); ++index){
                Entity *e = state->entities + index;

                //v2 pos = screen_from_world(e->pos, half_screen, state->scale);
                //v2 pos = screen_from_world_space(e->pos, half_screen);
                v2 pos = screen_from_world_space(e->pos, half_screen, state->scale);
                v2 dim = scaled_dim(e->dim, state->scale);
                if(has_flags(e->flags, EntityFlag_Active)){

                    switch(e->type){
                        case EntityType_Quad:{
                            v2 p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
                            v2 p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
                            v2 p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
                            v2 p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

                            //f32 deg = deg_from_dir(e->dir);
                            p0 = rotate_point_deg(p0, e->deg, pos);
                            p1 = rotate_point_deg(p1, e->deg, pos);
                            p2 = rotate_point_deg(p2, e->deg, pos);
                            p3 = rotate_point_deg(p3, e->deg, pos);

                            //set_texture(&assets.textures[TextureAsset_White]);
                            draw_quad(p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Asteroid:{
                            v2 p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
                            v2 p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
                            v2 p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
                            v2 p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, pos);
                            p1 = rotate_point_deg(p1, e->deg, pos);
                            p2 = rotate_point_deg(p2, e->deg, pos);
                            p3 = rotate_point_deg(p3, e->deg, pos);

                            //push_line(p0, p1, 2, GREEN);
                            //push_line(p1, p2, 2, GREEN);
                            //push_line(p2, p3, 2, GREEN);
                            //push_line(p3, p0, 2, GREEN);

                            set_texture(&r_assets->textures[TextureAsset_Asteroid]);
                            draw_texture(e->texture, p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Bullet:{
                            v2 p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
                            v2 p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
                            v2 p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
                            v2 p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, pos);
                            p1 = rotate_point_deg(p1, e->deg, pos);
                            p2 = rotate_point_deg(p2, e->deg, pos);
                            p3 = rotate_point_deg(p3, e->deg, pos);

                            //push_line(p0, p1, 2, GREEN);
                            //push_line(p1, p2, 2, GREEN);
                            //push_line(p2, p3, 2, GREEN);
                            //push_line(p3, p0, 2, GREEN);

                            set_texture(&r_assets->textures[TextureAsset_Bullet]);
                            draw_texture(e->texture, p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Particle:{
                            v2 p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
                            v2 p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
                            v2 p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
                            v2 p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, pos);
                            p1 = rotate_point_deg(p1, e->deg, pos);
                            p2 = rotate_point_deg(p2, e->deg, pos);
                            p3 = rotate_point_deg(p3, e->deg, pos);

                            //push_line(p0, p1, 2, GREEN);
                            //push_line(p1, p2, 2, GREEN);
                            //push_line(p2, p3, 2, GREEN);
                            //push_line(p3, p0, 2, GREEN);

                            draw_texture(e->texture, p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Texture:{
                            v2 p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
                            v2 p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
                            v2 p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
                            v2 p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, pos);
                            p1 = rotate_point_deg(p1, e->deg, pos);
                            p2 = rotate_point_deg(p2, e->deg, pos);
                            p3 = rotate_point_deg(p3, e->deg, pos);

                            //push_line(p0, p1, 2, GREEN);
                            //push_line(p1, p2, 2, GREEN);
                            //push_line(p2, p3, 2, GREEN);
                            //push_line(p3, p0, 2, GREEN);

                            draw_texture(e->texture, p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Ship:{
                            v2 p0 = make_v2(pos.x - dim.w/2, pos.y - dim.h/2);
                            v2 p1 = make_v2(pos.x + dim.w/2, pos.y - dim.h/2);
                            v2 p2 = make_v2(pos.x + dim.w/2, pos.y + dim.h/2);
                            v2 p3 = make_v2(pos.x - dim.w/2, pos.y + dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, pos);
                            p1 = rotate_point_deg(p1, e->deg, pos);
                            p2 = rotate_point_deg(p2, e->deg, pos);
                            p3 = rotate_point_deg(p3, e->deg, pos);

                            //set_texture(&assets.textures[TextureAsset_White]);
                            draw_line(p0, p1, 5, GREEN);
                            draw_line(p1, p2, 5, GREEN);
                            draw_line(p2, p3, 5, GREEN);
                            draw_line(p3, p0, 5, GREEN);

                            set_texture(&r_assets->textures[e->texture]);
                            if(state->ship->immune){
                                draw_texture(e->texture, p0, p1, p2, p3, ORANGE);
                            }
                            else{
                                draw_texture(e->texture, p0, p1, p2, p3, e->color);
                            }

                            // todo: yuckiness for ship exhaust
                            if(state->ship->accelerating){
                                p0.x += ((55 * state->scale) * (-e->dir.x));
                                p0.y += ((55 * state->scale) * (-e->dir.y));
                                p1.x += ((55 * state->scale) * (-e->dir.x));
                                p1.y += ((55 * state->scale) * (-e->dir.y));
                                p2.x += ((55 * state->scale) * (-e->dir.x));
                                p2.y += ((55 * state->scale) * (-e->dir.y));
                                p3.x += ((55 * state->scale) * (-e->dir.x));
                                p3.y += ((55 * state->scale) * (-e->dir.y));
                                u32 random_flame = random_range_u32(5) + 4;
                                set_texture(&r_assets->textures[random_flame]);
                                draw_texture(random_flame, p0, p1, p2, p3, e->color);
                            }

                        } break;
                    }
                }
            }

            String8 score = str8_formatted(ts->frame_arena, "SCORE: %i", state->score);
            draw_text(state->font, score, make_v2(text_padding, text_padding + ((f32)state->font->ascent * state->font->scale)), ORANGE);

            String8 lives = str8_formatted(ts->frame_arena, "LIVES: %i", state->lives);
            f32 width = font_string_width(state->font, lives);
            draw_text(state->font, lives, make_v2(window.width - width - text_padding, ((f32)(state->font->ascent) * state->font->scale) + text_padding), ORANGE);

            String8 level_str = str8_formatted(ts->frame_arena, "LEVEL: %i", state->level_index + 1);
            draw_text(state->font, level_str, make_v2(text_padding, text_padding + ((f32)state->font->ascent * state->font->scale) + ((f32)state->font->vertical_offset)), ORANGE);


            // todo: revaluate where this should be
            // todo: revaluate where this should be
            // todo: revaluate where this should be
            // todo: revaluate where this should be
            // todo: revaluate where this should be
            // todo: revaluate where this should be

            Level* level = state->current_level;
            String8 info_str = str8_formatted(ts->frame_arena, "level: %i\ntotal: %i\nspawned: %i\ndestroyed:%i", state->level_index, level->asteroid_count_max, level->asteroid_spawned, level->asteroid_destroyed);
            //draw_text(state->font, info_str, make_v2(50, window.height/2), TEAL);

            s32 found_count = 0;
            for(s32 i=0; i < array_count(state->entities); i++){
                Entity* e = state->entities + i;
                if(e->type == EntityType_Asteroid){
                    if(has_flags(e->flags, EntityFlag_Active)){
                        String8 str = str8_formatted(ts->frame_arena, "Asteroids - (%i)", e->health);
                        f32 str_width = font_string_width(state->font, str);
                        //draw_text(state->font, str, make_v2(window.width - str_width, (f32)(100 + (found_count * state->font->vertical_offset))), TEAL);
                        found_count++;
                    }
                }
            }

            if(game_won()){
                String8 text = str8_formatted(ts->frame_arena, "CHICKEN DINNER - Score: %i", state->score);
                f32 font_width = font_string_width(state->font, text);
                f32 x = window.width/2 - font_width/2;
                draw_text(state->font, text, make_v2(x, window.height/2 - 200), ORANGE);

                ui_begin(ts->ui_arena);

                ui_push_pos_x(window.width/2 - 50);
                ui_push_pos_y(window.height/2 - 100);
                ui_push_size_w(ui_size_children(0));
                ui_push_size_h(ui_size_children(0));

                UI_Box* box1 = ui_box(str8_literal("box1"));
                ui_push_parent(box1);
                ui_pop_pos_x();
                ui_pop_pos_y();

                ui_push_size_w(ui_size_pixel(100, 0));
                ui_push_size_h(ui_size_pixel(50, 0));
                ui_push_background_color(BLUE);
                if(ui_button(str8_literal("Restart")).pressed_left){
                    reset_game();
                    ui_close();
                }
                ui_spacer(10);
                if(ui_button(str8_literal("Exit")).pressed_left){
                    events_quit(&events);
                    ui_close();
                }
                ui_layout();
                ui_draw(ui_root());
                ui_end();
            }
            else if(game_over()){
                String8 text = str8_formatted(ts->frame_arena, "GAME OVER - Score: %i", state->score);
                f32 font_width = font_string_width(state->font, text);
                f32 x = window.width/2 - font_width/2;
                draw_text(state->font, text, make_v2(x, window.height/2 - 200), ORANGE);

                ui_begin(ts->ui_arena);

                ui_push_pos_x(window.width/2 - 50);
                ui_push_pos_y(window.height/2 - 100);
                ui_push_size_w(ui_size_children(0));
                ui_push_size_h(ui_size_children(0));

                UI_Box* box1 = ui_box(str8_literal("box1"));
                ui_push_parent(box1);
                ui_pop_pos_x();
                ui_pop_pos_y();

                ui_push_size_w(ui_size_pixel(100, 0));
                ui_push_size_h(ui_size_pixel(50, 0));
                ui_push_background_color(BLUE);
                if(ui_button(str8_literal("Restart")).pressed_left){
                    reset_game();
                    ui_close();
                }
                ui_spacer(10);
                if(ui_button(str8_literal("Exit")).pressed_left){
                    events_quit(&events);
                    ui_close();
                }
                ui_layout();
                ui_draw(ui_root());
                ui_end();
            }
            else if(pause && !game_won() && !game_over()){
                ui_begin(ts->ui_arena);

                ui_push_pos_x(window.width/2 - 50);
                ui_push_pos_y(window.height/2 - 100);
                ui_push_size_w(ui_size_children(0));
                ui_push_size_h(ui_size_children(0));

                UI_Box* box1 = ui_box(str8_literal("box1"));
                ui_push_parent(box1);
                ui_pop_pos_x();
                ui_pop_pos_y();

                ui_push_size_w(ui_size_pixel(100, 0));
                ui_push_size_h(ui_size_pixel(50, 0));
                ui_push_background_color(BLUE);
                if(ui_button(str8_literal("Resume")).pressed_left){
                    pause = false;
                    ui_close();
                }
                ui_spacer(10);
                if(ui_button(str8_literal("Exit")).pressed_left){
                    events_quit(&events);
                    ui_close();
                }
                ui_layout();
                ui_draw(ui_root());
                ui_end();
            }

            ui_begin(ts->ui_arena);

            ui_push_pos_x(100);
            ui_push_pos_y(300);
            ui_push_size_w(ui_size_children(0));
            ui_push_size_h(ui_size_children(0));

            ui_push_border_thickness(10);
            ui_push_background_color(ORANGE);
            UI_Box* box1 = ui_box(str8_literal("box1##4"),
                    UI_BoxFlag_DrawBackground|UI_BoxFlag_Draggable|UI_BoxFlag_Clickable);
            ui_push_parent(box1);
            ui_pop_pos_x();
            ui_pop_pos_y();

            ui_push_size_w(ui_size_pixel(100, 0));
            ui_push_size_h(ui_size_pixel(50, 0));
            ui_push_background_color(BLUE);
            if(ui_button(str8_literal("1920x1080")).pressed_left){
                if(window.width != 1920){
                    window.width = 1920.0f;
                    window.height = 1080.0f;
                    os_resize_window(window.handle);
                    d3d_resize_window(window.width, window.height);
                    state->scale = get_scale(&window);
                    half_screen = make_v2(window.width/2, window.height/2);
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("1280x720")).pressed_left){
                if(window.width != 1280){
                    window.width = 1280.0f;
                    window.height = 720.0f;
                    os_resize_window(window.handle);
                    d3d_resize_window(window.width, window.height);
                    state->scale = get_scale(&window);
                    half_screen = make_v2(window.width/2, window.height/2);
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("plus scale")).pressed_left){
                state->scale += 0.1f;
            }
            ui_spacer(10);
            if(ui_button(str8_literal("minus scale")).pressed_left){
                state->scale -= 0.1f;
                if(state->scale < 0.0f){
                    state->scale = 0.0f;
                }
            }

            ui_layout();
            ui_draw(ui_root());
            ui_end();
        }

        if(state->game_mode == GameMode_Menu){
            ui_begin(ts->ui_arena);

            ui_push_pos_x(window.width/2 - 50);
            ui_push_pos_y(window.height/2 - 100);
            ui_push_size_w(ui_size_children(0));
            ui_push_size_h(ui_size_children(0));

            UI_Box* box1 = ui_box(str8_literal("box1##1"));
            ui_push_parent(box1);
            ui_pop_pos_x();
            ui_pop_pos_y();

            ui_push_size_w(ui_size_pixel(100, 0));
            ui_push_size_h(ui_size_pixel(50, 0));
            ui_push_background_color(BLUE);
            if(ui_button(str8_literal("Play##1")).pressed_left){
                state->game_mode = GameMode_Game;
                reset_game();
                ui_close();
            }
            ui_spacer(10);
            if(ui_button(str8_literal("Exit##1")).pressed_left){
                events_quit(&events);
                ui_close();
            }
            ui_layout();
            ui_draw(ui_root());
            ui_end();
        }

        String8 fps = str8_formatted(ts->frame_arena, "FPS: %.2f", FPS);
        draw_text(state->font, fps, make_v2(window.width - text_padding - font_string_width(state->font, fps), window.height - text_padding), ORANGE);

        wasapi_play_cursors();

        console_draw();
        // draw everything
        draw_render_batches();
        d3d_present();

        clear_controller_pressed();
        //print("batch_arena_size: %i, batch_arena_at: %i\n",  ts->batch_arena->size, ts->batch_arena->at);
        //for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
        //    print("id: %i, total batches: %i, batch_count: %i, batch_cap: %i\n",
        //            batch->id, render_batches.count, batch->count, batch->cap);
        //}
        //print("last_id: %i - total: %i\n", render_batches.last->id, render_batches.count);
        //print("-----------------------------\n");
        render_batches_reset();
        arena_free(ts->batch_arena);
        arena_free(ts->frame_arena);
        arena_free(ts->ui_arena);

        // todo: why is this here?
        //end_profiler();

        frame_inc++;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_inc / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_inc = 0;
        }

    }

    d3d_release();
    end_profiler();
    wasapi_release();

    return(0);
}

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
