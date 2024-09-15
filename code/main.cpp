#include "main.hpp"

WINDOWPLACEMENT window_info = { sizeof(WINDOWPLACEMENT) };
static void
os_fullscreen_mode(Window* window){
    window->type = WindowType_Fullscreen;
    s32 style = GetWindowLong(window->handle, GWL_STYLE);

    if(style & WS_OVERLAPPEDWINDOW){ // is windows mode?
        MONITORINFO monitor_info = { sizeof(MONITORINFO) };

        u32 flags = SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
        if(GetWindowPlacement(window->handle, &window_info) &&
           GetMonitorInfo(MonitorFromWindow(window->handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info)){
            SetWindowLong(window->handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window->handle, HWND_TOP,
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         flags);
        }
    }
}

static void
os_windowed_mode(Window* window){
    window->type = WindowType_Windowed;
    s32 style = GetWindowLong(window->handle, GWL_STYLE);

    u32 flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    SetWindowLong(window->handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(window->handle, &window_info);
    SetWindowPos(window->handle, 0,
                 0, 0, 0, 0, flags);
}

static void
change_resolution(Window* window, f32 width, f32 height) {
    window->width = width;
    window->height = height;
    window->aspect_ratio = window->width/window->height;

    s32 style = GetWindowLong(window->handle, GWL_STYLE);
    RECT rect = {0, 0, (s32)width, (s32)height};
    AdjustWindowRect(&rect, (DWORD)style, FALSE);

    u32 flags = 0;
    if(window->type == WindowType_Fullscreen){
        flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    }
    if(window->type == WindowType_Windowed){
        flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
    }
    SetWindowPos(window->handle, 0,
                 0, 0, rect.right - rect.left, rect.bottom - rect.top, flags);

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
    result.type = WindowType_Windowed;

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
    result.aspect_ratio = result.width/result.height;

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
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event = {0};
            event.type = QUIT;
            events_add(&events, event);
        } break;

        case WM_NCHITTEST:{ // note: prevent resizing on edges
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
            event.key_pressed = true;

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

            event.key_pressed = true;
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

            event.key_pressed = false;
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
        ts->hash_arena = push_arena(&ts->arena, MB(100));
        ts->batch_arena = push_arena(&ts->arena, MB(100));

        state->scene_state = SceneState_Menu;
        state->game_state = GameState_None;

        show_cursor(true);
        load_assets(ts->asset_arena, &assets);

        //init_camera(&camera);
        init_camera_2d(&camera, make_v2(0, 0), 1000);
        init_console(global_arena, &camera, &window, &assets);
        init_ui(ts->hash_arena, &window, &controller, &assets);
        init_render_commands(ts->batch_arena, &assets);

        state->font = &assets.fonts[FontAsset_Arial];

        // setup free entities array in reverse order
        entities_clear();

        //wasapi_play(&assets.waves[WaveAsset_Track1], 0.1f, true);
        //wasapi_play(&assets.waves[WaveAsset_Track5], 0.0f, true);
        //wasapi_play(&assets.waves[WaveAsset_Track4], 0.0f, true);

        state->ship = add_ship(TextureAsset_Ship, make_v2(0, 0), make_v2(150, 150));
        state->ship_loaded = true;
        state->lives = MAX_LIVES;

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
        }

        //----constant buffer----
        D3D11_MAPPED_SUBRESOURCE mapped_subresource;
        d3d_context->Map(d3d_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
        ConstantBuffer2D* constants = (ConstantBuffer2D*)mapped_subresource.pData;
        constants->screen_res = make_v2s32((s32)window.width, (s32)window.height);
        d3d_context->Unmap(d3d_constant_buffer, 0);

        //draw_clear_color(BACKGROUND_COLOR);

        console_update();
        camera_2d_update(&camera, window.aspect_ratio);

        d3d_clear_color(BACKGROUND_COLOR);
        //if(state->editor)
        //    if(controller.button[MOUSE_WHEEL_UP].pressed){
        //        camera.size -= 50;
        //    }
        //    if(controller.button[MOUSE_WHEEL_DOWN].pressed){
        //        camera.size += 50;
        //    }
        //}

        //if(state->scene_state == SceneState_Game &&
        //   state->game_state == GameState_Finished &&
        //   (state->game_result == GameResult_Won || state->game_result == GameResult_Lost)){
        //    if(controller.button[KeyCode_R].pressed){
        //        game_reset();
        //    }
        //}
        if(game_lost()){
            if(state->game_state != GameState_Finished){
                state->game_state = GameState_Finished;
                state->game_result = GameResult_Lost;
                wasapi_play(&assets.waves[WaveAsset_GameLost], 0.1f, false);
            }
        }

        if(game_won()){
            if(state->game_state != GameState_Finished){
                state->game_state = GameState_Finished;
                state->game_result = GameResult_Won;
                wasapi_play(&assets.waves[WaveAsset_GameWon], 0.1f, false);
            }
        }


        if(state->scene_state == SceneState_Menu){
            show_cursor(true);

            {
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
                    wasapi_play(&assets.waves[WaveAsset_Music], 0.5f, true);
                    state->scene_state = SceneState_Game;
                    state->game_state = GameState_Running;
                    game_reset();
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
        }
        else if(state->scene_state == SceneState_Game){
            if(state->game_state != GameState_Paused){
                simulations = 0;
                accumulator += frame_time;
                while(accumulator >= clock.dt){
                    update_game();

                    accumulator -= clock.dt;
                    time_elapsed += clock.dt;
                    simulations++;

                }
            }

            draw_entities(state);
            draw_level_info();
            if(state->game_state == GameState_Running){
                show_cursor(false);
            }
            else if(state->game_state == GameState_Paused){
                show_cursor(true);
                game_paused_ui();
            }
            else if(state->game_state == GameState_Finished){
                show_cursor(true);
                if(controller.button[KeyCode_R].pressed){
                    game_reset();
                }

                if(state->game_result == GameResult_Won){
                    String8 text = str8_formatted(ts->frame_arena, "CHICKEN DINNER - Score: %i", state->score);
                    f32 font_width = font_string_width(state->font, text);
                    f32 x = window.width/2 - font_width/2;
                    draw_text(state->font, text, make_v2(x, window.height/2 - 200), ORANGE);

                    game_finished_ui();
                }
                if(state->game_result == GameResult_Lost){
                    String8 text = str8_formatted(ts->frame_arena, "GAME OVER - Score: %i", state->score);
                    f32 font_width = font_string_width(state->font, text);
                    f32 x = window.width/2 - font_width/2;
                    draw_text(state->font, text, make_v2(x, window.height/2 - 200), ORANGE);

                    game_finished_ui();
                }
            }
        }

        // todo: remove this
        if(state->scene_state == SceneState_Game){

            begin_timed_scope("GameMode_Game");


            ui_begin(ts->ui_arena);

            ui_push_pos_x(100);
            ui_push_pos_y(300);
            ui_push_size_w(ui_size_children(0));
            ui_push_size_h(ui_size_children(0));

            ui_push_border_thickness(10);
            ui_push_background_color(ORANGE);
            UI_Box* box1 = ui_box(str8_literal("box1##4"),
                                  UI_BoxFlag_DrawBackground|
                                  UI_BoxFlag_Draggable|
                                  UI_BoxFlag_Clickable);
            ui_push_parent(box1);
            ui_pop_pos_x();
            ui_pop_pos_y();

            ui_push_size_w(ui_size_pixel(100, 0));
            ui_push_size_h(ui_size_pixel(50, 0));
            ui_push_background_color(BLUE);
            if(ui_button(str8_literal("fullscreen")).pressed_left){
                if(window.type != WindowType_Fullscreen){
                    os_fullscreen_mode(&window);
                    change_resolution(&window, window.width, window.height);
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("windowed")).pressed_left){
                if(window.type != WindowType_Windowed){
                    os_windowed_mode(&window);
                    change_resolution(&window, window.width, window.height);
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("1920x1080")).pressed_left){
                if(window.width != 1920){
                    change_resolution(&window, 1920, 1080);
                    d3d_resize_window(window.width, window.height);
                }
            }
            ui_spacer(10);
            if(ui_button(str8_literal("1280x720")).pressed_left){
                if(window.width != 640){
                    change_resolution(&window, 640, 360);
                    d3d_resize_window(window.width, window.height);
                }
            }

            ui_layout();
            ui_draw(ui_root());
            ui_end();
        }

        String8 fps = str8_formatted(ts->frame_arena, "FPS: %.2f", FPS);
        draw_text(state->font, fps, make_v2(window.width - text_padding - font_string_width(state->font, fps), window.height - text_padding), ORANGE);

        String8 count = str8_formatted(ts->frame_arena, "count: %i", wave_cursors_count);
        draw_text(state->font, count, make_v2(window.width - text_padding - font_string_width(state->font, fps), window.height - text_padding - 200), ORANGE);

        wasapi_play_cursors();
        console_draw();
        //String8 stuff = str8_formatted(ts->frame_arena, "(%f, %f)", camera.pos.x, camera.pos.y);
        //draw_text(state->font, stuff, make_v2(200, 200), ORANGE);

        // draw everything
        //v2 p0 = pos_screen_from_world(make_v2(camera.left_border, camera.top_border), &camera, &window);
        //v2 p1 = pos_screen_from_world(make_v2(camera.right_border, camera.top_border), &camera, &window);
        //v2 p2 = pos_screen_from_world(make_v2(camera.right_border, camera.bottom_border), &camera, &window);
        //v2 p3 = pos_screen_from_world(make_v2(camera.left_border, camera.bottom_border), &camera, &window);
        //draw_line(p0, p1, 10, GREEN);
        //draw_line(p1, p2, 10, GREEN);
        //draw_line(p2, p3, 10, GREEN);
        //draw_line(p3, p0, 10, GREEN);


        clear_controller_pressed();
        print("batch_arena_size: %i, batch_arena_at: %i\n",  ts->batch_arena->size, ts->batch_arena->at);
        for(RenderBatch* batch = render_batches.first; batch != 0; batch = batch->next){
            print("total_batches: %i, batches_id: %i, batch_count: %i, batch_cap: %i\n",
                    render_batches.count, batch->id, batch->count, batch->cap);
            if(render_batches.count > bcount){
                bcount = render_batches.count;
            }
        }
        print("last_id: %i - total: %i - highest_count: %i\n", render_batches.last->id, render_batches.count, bcount);
        print("-----------------------------\n");

        {
            draw_render_batches();
            d3d_present();

            render_batches_reset();
            arena_free(ts->batch_arena);
            arena_free(ts->frame_arena);
            arena_free(ts->ui_arena);

            frame_inc++;
            f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
            if(second_elapsed > 1){
                FPS = ((f64)frame_inc / second_elapsed);
                frame_tick_start = clock.get_os_timer();
                frame_inc = 0;
            }
        }
        // todo: why is this here?
        //end_profiler();
    }

    d3d_release();
    end_profiler();
    wasapi_release();

    return(0);
}

