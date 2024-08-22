#include "main.hpp"

WINDOWPLACEMENT window_info = { sizeof(WINDOWPLACEMENT) };
static void os_resize_window(HWND hwnd){
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
change_resolution(HWND hwnd, u32 width, u32 height, bool is_fullscreen) {
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
u32_buffer_from_u8_buffer(String8* u8_buffer, String8* u32_buffer){
    u32* base_rgba = (u32*)u32_buffer->str;
    u8* base_a = (u8*)u8_buffer->str;

    for(s32 i=0; i < u8_buffer->size; ++i){
        *base_rgba = (u32)(*base_a << 24 |
                               255 << 16 |
                               255 << 8  |
                               255 << 0);
        base_rgba++;
        base_a++;
    }
}

static void
load_assets(Arena* arena){

    ScratchArena scratch = begin_scratch();

    Bitmap bm;
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/ship2.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Ship].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/circle.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Bullet].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/asteroid.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Asteroid].view, &bm);

    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/flame1.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Flame1].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/flame2.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Flame2].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/flame3.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Flame3].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/flame4.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Flame4].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/flame5.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Flame5].view, &bm);

    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/explosion1.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Explosion1].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/explosion2.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Explosion2].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/explosion3.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Explosion3].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/explosion4.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Explosion4].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/explosion5.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Explosion5].view, &bm);
    bm = load_bitmap(scratch.arena, build_path, str8_literal("sprites/explosion6.bmp"));
    init_texture_resource(&ts->assets.textures[TextureAsset_Explosion6].view, &bm);

    end_scratch(scratch);

    ts->assets.waves[WaveAsset_Track1] = load_wave(arena, build_path, str8_literal("sounds/track1.wav"));
    ts->assets.waves[WaveAsset_Track2] = load_wave(arena, build_path, str8_literal("sounds/track2.wav"));
    ts->assets.waves[WaveAsset_Track3] = load_wave(arena, build_path, str8_literal("sounds/track3.wav"));
    ts->assets.waves[WaveAsset_Track4] = load_wave(arena, build_path, str8_literal("sounds/track4.wav"));
    ts->assets.waves[WaveAsset_Track5] = load_wave(arena, build_path, str8_literal("sounds/track5.wav"));
    ts->assets.waves[WaveAsset_Rail1] =  load_wave(arena, build_path, str8_literal("sounds/rail1.wav"));
    ts->assets.waves[WaveAsset_Rail2] =  load_wave(arena, build_path, str8_literal("sounds/rail2.wav"));
    ts->assets.waves[WaveAsset_Rail3] =  load_wave(arena, build_path, str8_literal("sounds/rail3.wav"));
    ts->assets.waves[WaveAsset_Rail4] =  load_wave(arena, build_path, str8_literal("sounds/rail4.wav"));
    ts->assets.waves[WaveAsset_Rail5] =  load_wave(arena, build_path, str8_literal("sounds/rail5.wav"));

    ts->assets.fonts[FontAsset_Arial] =    load_font_ttf(arena, str8_literal("fonts/arial.ttf"), 16);
    ts->assets.fonts[FontAsset_Golos] =    load_font_ttf(arena, str8_literal("fonts/GolosText-Regular.ttf"), 36);
    ts->assets.fonts[FontAsset_Consolas] = load_font_ttf(arena, str8_literal("fonts/consola.ttf"), 36);
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
init_memory(){
    memory.permanent_size = MB(500);
    memory.transient_size = GB(1);
    memory.size = memory.permanent_size + memory.transient_size;

    memory.base = os_virtual_alloc(memory.size);
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
    begin_timed_function();
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
            event.mouse_x = (s32)(s16)(l_param & 0xFFFF);
            event.mouse_y = (s32)(s16)(l_param >> 16);

            // calc dx/dy and normalize from -1:1
            s32 dx = event.mouse_x - controller.mouse.x;
            s32 dy = event.mouse_y - controller.mouse.y;
            v2 delta_normalized = normalize_v2(make_v2((f32)dx, (f32)dy));
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
    init_memory();
    init_clock(&clock);
    HRESULT hr = init_audio(2, 48000, 32);
    assert_hr(hr);
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

        ts->render_command_arena = push_arena(&ts->arena, MB(100));
        ts->frame_arena = push_arena(&ts->arena, MB(100));
        ts->asset_arena = push_arena(&ts->arena, MB(100));
        ts->ui_arena = push_arena(&ts->arena, MB(100));

        state->game_mode = GameMode_Game;

        show_cursor(true);
        load_assets(ts->asset_arena);

        init_camera();
        init_console(&state->arena, FontAsset_Arial);
        init_console_commands();
        init_ui(&state->arena, &window, &controller);
        init_render_commands(ts->render_command_arena);

        state->current_font = FontAsset_Arial;
        state->font = &ts->assets.fonts[FontAsset_Arial];

        // setup free entities array in reverse order
        entities_clear();

        //audio_play(WaveAsset_Track1, 0.1f, true);
        //audio_play(WaveAsset_Track5, 0.0f, true);
        //audio_play(WaveAsset_Track4, 0.0f, true);

        state->scale = 1;
        state->ship = add_ship(TextureAsset_Ship, make_v2(0, 0), make_v2(30, 30));
        //state->ship = add_ship(TextureAsset_Ship, make_v2(window.width/2, window.height/2), make_v2(30, 30));
        state->ship_loaded = true;
        state->lives = 1;

        state->level_index = 0;
        init_levels();
        state->current_level = &state->levels[state->level_index];

        memory.initialized = true;

    }

    should_quit = false;
    while(!should_quit){
        begin_timed_scope("while(!should_quit)");

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
                            change_resolution(window.handle, 640, 360, false);
                        }
                    }
                    if(event.keycode == KeyCode_Q){
                        os_resize_window(window.handle);
                        fullscreen = !fullscreen;
                        if(fullscreen){
                            d3d_resize_window(1920, 1080);
                            window.width = 1920.0f;
                            window.height = 1080.0f;
                        }
                        else{
                            d3d_resize_window(640, 360);
                            window.width = 640.0f;
                            window.height = 360.0f;
                        }
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

        draw_clear_color(BACKGROUND_COLOR);
        if(state->game_mode == GameMode_Menu){
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
            if(ui_button(str8_literal("Play")).pressed_left){
                state->game_mode = GameMode_Game;
                reset_game();
                ui_close();
            }
            ui_spacer(10);
            if(ui_button(str8_literal("Exit")).pressed_left){
                should_quit = true;
                ui_close();
            }
        }

        if(state->game_mode == GameMode_Game){
            Font* font = &ts->assets.fonts[state->current_font];

            //if(controller.button[KeyCode_ESCAPE].pressed){
            //    pause = !pause;
            //}

            if(!state->lives){
                String8 text = str8_formatted(ts->frame_arena, "GAME OVER - Score: %i", state->score);
                f32 width = font_string_width(state->current_font, text);
                f32 x = window.width/2 - width/2;
                draw_text(state->current_font, text, make_v2(x, window.height/2 - 200), ORANGE);

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
                    should_quit = true;
                    ui_close();
                }
            }
            else if(game_won()){
                String8 text = str8_formatted(ts->frame_arena, "CHICKEN DINNER - Score: %i", state->score);
                f32 width = font_string_width(state->current_font, text);
                f32 x = window.width/2 - width/2;
                draw_text(state->current_font, text, make_v2(x, window.height/2 - 200), ORANGE);

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
                    should_quit = true;
                    ui_close();
                }
            }
            if(pause && !game_won() && state->lives){
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
                    should_quit = true;
                    ui_close();
                }
            }
            else{
                simulations = 0;
                accumulator += frame_time;
                while(accumulator >= clock.dt){
                    begin_timed_scope("simulation");
                    update_game();

                    accumulator -= clock.dt;
                    time_elapsed += clock.dt;
                    simulations++;

                    clear_controller_pressed();
                }
            }

            // todo: also use flags here
            for(s32 index = 0; index < array_count(state->entities); ++index){
                begin_timed_scope("build command arena");
                Entity *e = state->entities + index;
                if(has_flags(e->flags, EntityFlag_Active)){

                    switch(e->type){
                        case EntityType_Quad:{
                            v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                            v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                            v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                            v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                            //f32 deg = deg_from_dir(e->dir);
                            p0 = rotate_point_deg(p0, e->deg, e->pos);
                            p1 = rotate_point_deg(p1, e->deg, e->pos);
                            p2 = rotate_point_deg(p2, e->deg, e->pos);
                            p3 = rotate_point_deg(p3, e->deg, e->pos);

                            draw_quad(p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Asteroid:
                        case EntityType_Bullet:
                        case EntityType_Particle:
                        case EntityType_Texture:{
                            v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                            v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                            v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                            v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, e->pos);
                            p1 = rotate_point_deg(p1, e->deg, e->pos);
                            p2 = rotate_point_deg(p2, e->deg, e->pos);
                            p3 = rotate_point_deg(p3, e->deg, e->pos);

                            //push_line(p0, p1, 2, GREEN);
                            //push_line(p1, p2, 2, GREEN);
                            //push_line(p2, p3, 2, GREEN);
                            //push_line(p3, p0, 2, GREEN);

                            draw_texture(e->texture, p0, p1, p2, p3, e->color);
                        } break;
                        case EntityType_Ship:{
                            v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                            v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                            v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                            v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                            p0 = rotate_point_deg(p0, e->deg, e->pos);
                            p1 = rotate_point_deg(p1, e->deg, e->pos);
                            p2 = rotate_point_deg(p2, e->deg, e->pos);
                            p3 = rotate_point_deg(p3, e->deg, e->pos);

                            //push_line(p0, p1, 2, GREEN);
                            //push_line(p1, p2, 2, GREEN);
                            //push_line(p2, p3, 2, GREEN);
                            //push_line(p3, p0, 2, GREEN);

                            if(state->ship->immune){
                                draw_texture(e->texture, p0, p1, p2, p3, ORANGE);
                            }
                            else{
                                draw_texture(e->texture, p0, p1, p2, p3, e->color);
                            }

                            // todo: yuckiness for ship exhaust
                            if(state->ship->accelerating){
                                p0.x += (55 * (-e->dir.x));
                                p0.y += (55 * (-e->dir.y));
                                p1.x += (55 * (-e->dir.x));
                                p1.y += (55 * (-e->dir.y));
                                p2.x += (55 * (-e->dir.x));
                                p2.y += (55 * (-e->dir.y));
                                p3.x += (55 * (-e->dir.x));
                                p3.y += (55 * (-e->dir.y));
                                u32 random_flame = random_range_u32(5) + 3;
                                draw_texture(random_flame, p0, p1, p2, p3, e->color);
                            }

                        } break;
                    }
                }
            }

            String8 score = str8_formatted(ts->frame_arena, "SCORE: %i", state->score);
            draw_text(state->current_font, score, make_v2(text_padding, text_padding + ((f32)font->ascent * font->scale)), ORANGE);

            String8 lives = str8_formatted(ts->frame_arena, "LIVES: %i", state->lives);
            f32 width = font_string_width(state->current_font, lives);
            draw_text(state->current_font, lives, make_v2(window.width - width - text_padding, ((f32)(font->ascent) * font->scale) + text_padding), ORANGE);

            String8 level_str = str8_formatted(ts->frame_arena, "LEVEL: %i", state->level_index + 1);
            draw_text(state->current_font, level_str, make_v2(text_padding, text_padding + ((f32)font->ascent * font->scale) + ((f32)font->vertical_offset)), ORANGE);


            // todo: revaluate where this should be
            frame_inc++;
            f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
            if(second_elapsed > 1){
                FPS = ((f64)frame_inc / second_elapsed);
                frame_tick_start = clock.get_os_timer();
                frame_inc = 0;
            }
            //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);
            String8 fps = str8_formatted(ts->frame_arena, "FPS: %.2f", FPS);
            draw_text(state->current_font, fps, make_v2(window.width - text_padding - font_string_width(state->current_font, fps), window.height - text_padding), ORANGE);

            Level* level = state->current_level;
            String8 info_str = str8_formatted(ts->frame_arena, "level: %i\ntotal: %i\nspawned: %i\ndestroyed:%i", state->level_index, level->asteroid_count_max, level->asteroid_spawned, level->asteroid_destroyed);
            //draw_text(state->current_font, info_str, make_v2(50, window.height/2), TEAL);

            s32 found_count = 0;
            for(s32 i=0; i < array_count(state->entities); i++){
                Entity* e = state->entities + i;
                if(e->type == EntityType_Asteroid){
                    if(has_flags(e->flags, EntityFlag_Active)){
                        String8 str = str8_formatted(ts->frame_arena, "Asteroids - (%i)", e->health);
                        f32 str_width = font_string_width(state->current_font, str);
                        //draw_text(state->current_font, str, make_v2(window.width - str_width, (f32)(100 + (found_count * state->font->vertical_offset))), TEAL);
                        found_count++;
                    }
                }
            }
        }
        console_update();

        ui_layout();
        ui_draw(ui_root());
        ui_end();

        // draw everything
        console_draw();
        draw_commands();

        audio_play_cursors();

        arena_free(ts->frame_arena);
        arena_free(ts->ui_arena);
        arena_free(ts->render_command_arena);

        frame_count++;

        // todo: why is this here?
        //end_profiler();
    }

    d3d_release();
    end_profiler();
    audio_release();

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
        //    audio_play(WaveAsset_Rail1, 0.1f, false);
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
