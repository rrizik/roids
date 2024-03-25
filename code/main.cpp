#include "main.hpp"

// todo: move header includes here
#include "input.cpp"
#include "clock.cpp"
#include "wave.cpp"
#include "wasapi.cpp"
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
    build_path = os_application_path(global_arena);
    fonts_path = str8_path_append(global_arena, build_path, str8_literal("fonts"));
    shaders_path = str8_path_append(global_arena, build_path, str8_literal("shaders"));
    saves_path = str8_path_append(global_arena, build_path, str8_literal("saves"));
    sprites_path = str8_path_append(global_arena, build_path, str8_literal("sprites"));
    sounds_path = str8_path_append(global_arena, build_path, str8_literal("sounds"));
}

static void
memory_init(){
    memory.permanent_size = MB(500);
    memory.transient_size = GB(1);
    memory.size = memory.permanent_size + memory.transient_size;

    memory.base = os_virtual_alloc(memory.size);
    memory.permanent_base = memory.base;
    memory.transient_base = (u8*)memory.base + memory.permanent_size;
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

    DWORD style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    RECT rect = { 0, 0, width, height };
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
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            Event event;
            event.type = QUIT;
            events_add(&events, event);
        } break;

        case WM_MOUSEMOVE:{
            Event event;
            event.type = MOUSE; // TODO: maybe have this be a KEYBOARD event
            event.mouse_pos.x = (s32)(s16)(l_param & 0xFFFF);
            event.mouse_pos.y = (s32)(s16)(l_param >> 16);
            print("%i, %i\n", event.mouse_pos.x, event.mouse_pos.y);

            // todo: dx/y is probably wrong, not working as expected
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
        } break;
        case WM_SYSKEYUP:
        case WM_KEYUP:{
            Event event;
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

    init_paths(global_arena);
    random_seed(0, 1);

    d3d_init(window.handle, window.width, window.height);
#if DEBUG
    d3d_init_debug_stuff();
#endif

    memory_init();
    init_clock(&clock);
    HRESULT hr = audio_init(2, 48000, 32);
    assert_hr(hr);

    events_init(&events);

    f64 FPS = 0;
    f64 MSPF = 0;
    u64 total_frames = 0;
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
        begin_timed_scope("memory.initialized");
        // consider: maybe move this memory stuff to memory_init()
        init_arena(&pm->arena, (u8*)memory.permanent_base + sizeof(PermanentMemory), memory.permanent_size - sizeof(PermanentMemory));
        init_arena(&tm->arena, (u8*)memory.transient_base + sizeof(TransientMemory), memory.transient_size - sizeof(TransientMemory));

        tm->render_command_arena = push_arena(&tm->arena, MB(100));
        tm->frame_arena = push_arena(&tm->arena, MB(100));
        tm->assets_arena = push_arena(&tm->arena, MB(100));

        pm->game_mode = GameMode_Game;

        show_cursor(true);
        load_assets(tm->assets_arena, &tm->assets);

        init_camera();
        init_console(&pm->arena, FontAsset_Golos);
        init_console_commands();

        init_texture_resource(&tm->assets.bitmap[BitmapAsset_Ship],   &ship_shader_resource);
        init_texture_resource(&tm->assets.bitmap[BitmapAsset_Circle], &circle_shader_resource);
        init_texture_resource(&tm->assets.bitmap[BitmapAsset_Asteroid], &asteroid_shader_resource);

        current_font = FontAsset_Arial;

        // setup free entities array in reverse order
        entities_clear(pm);

        audio_play(WaveAsset_track1, 0.0f, true);
        audio_play(WaveAsset_track5, 0.0f, true);
        audio_play(WaveAsset_track4, 0.0f, true);

        pm->ship = add_ship(pm, &ship_shader_resource, make_v2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), make_v2(75, 75));
        pm->ship_loaded = true;
        pm->lives = 3;

        memory.initialized = true;
    }


    f32 s = 0;
    should_quit = false;
    while(!should_quit){
        begin_timed_scope("while(!should_quit)");
        MSG message;
        while(PeekMessageW(&message, window.handle, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        //audio_play(440);
        //audio_play_wave(&wav);
        u64 now_ticks = clock.get_os_timer();
        f64 frame_time = clock.get_seconds_elapsed(now_ticks, last_ticks);
        MSPF = 1000/1000/((f64)clock.frequency / (f64)(now_ticks - last_ticks));
        last_ticks = now_ticks;

        arena_free(tm->render_command_arena);
        // simulation
        accumulator += frame_time;
        while(accumulator >= clock.dt){
            begin_timed_scope("simulation");
            update_game(&window, &memory, &events);

            accumulator -= clock.dt;
            time_elapsed += clock.dt;
            simulations++;

            clear_controller_pressed();
        }
        audio_play_cursors();

        // command arena
        draw_clear_color(tm->render_command_arena, BACKGROUND_COLOR);
        // todo: also use flags here
        for(s32 index = 0; index < array_count(pm->entities); ++index){
            begin_timed_scope("command arena");
            Entity *e = pm->entities + index;

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

                    draw_quad(tm->render_command_arena, p0, p1, p2, p3, e->color);
                } break;
                case EntityType_Asteroid:
                case EntityType_Bullet:
                case EntityType_Texture:{
                    v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                    v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                    Rect e_rect = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                                            make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
                    //push_quad(tm->render_command_arena, e_rect.min, make_v2(e_rect.x1, e_rect.y0), e_rect.max, make_v2(e_rect.x0, e_rect.y1), ORANGE);

                    p0 = rotate_point_deg(p0, e->deg, e->pos);
                    p1 = rotate_point_deg(p1, e->deg, e->pos);
                    p2 = rotate_point_deg(p2, e->deg, e->pos);
                    p3 = rotate_point_deg(p3, e->deg, e->pos);

                    //push_line(tm->render_command_arena, p0, p1, 2, GREEN);
                    //push_line(tm->render_command_arena, p1, p2, 2, GREEN);
                    //push_line(tm->render_command_arena, p2, p3, 2, GREEN);
                    //push_line(tm->render_command_arena, p3, p0, 2, GREEN);

                    draw_texture(tm->render_command_arena, e->texture, p0, p1, p2, p3, e->color);
                    String8 text = str8_formatted(tm->frame_arena, "%i", e->index);
                    if(e->type == EntityType_Asteroid){
                        text = str8_formatted(tm->frame_arena, "%i", e->health);
                    }
                    //push_text(tm->render_command_arena, current_font, text, p0.x, p0.y, RED);
                } break;
                case EntityType_Ship:{
                    v2 p0 = make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p1 = make_v2(e->pos.x + e->dim.w/2, e->pos.y - e->dim.h/2);
                    v2 p2 = make_v2(e->pos.x + e->dim.w/2, e->pos.y + e->dim.h/2);
                    v2 p3 = make_v2(e->pos.x - e->dim.w/2, e->pos.y + e->dim.h/2);

                    Rect e_rect = make_rect(make_v2(e->pos.x - e->dim.w/2, e->pos.y - e->dim.h/2),
                                            make_v2(e->pos.x + e->dim.x/2, e->pos.y + e->dim.h/2));
                    //push_quad(tm->render_command_arena, e_rect.min, make_v2(e_rect.x1, e_rect.y0), e_rect.max, make_v2(e_rect.x0, e_rect.y1), ORANGE);

                    p0 = rotate_point_deg(p0, e->deg, e->pos);
                    p1 = rotate_point_deg(p1, e->deg, e->pos);
                    p2 = rotate_point_deg(p2, e->deg, e->pos);
                    p3 = rotate_point_deg(p3, e->deg, e->pos);

                    //push_line(tm->render_command_arena, p0, p1, 2, GREEN);
                    //push_line(tm->render_command_arena, p1, p2, 2, GREEN);
                    //push_line(tm->render_command_arena, p2, p3, 2, GREEN);
                    //push_line(tm->render_command_arena, p3, p0, 2, GREEN);

                    draw_texture(tm->render_command_arena, e->texture, p0, p1, p2, p3, e->color);
                    String8 text = str8_formatted(tm->frame_arena, "%i", e->index);
                    //push_text(tm->render_command_arena, current_font, text, p0.x, p0.y, RED);
                } break;
            }
        }


        if(!pm->lives){
            String8 text = str8_formatted(tm->frame_arena, "GAME OVER - Score: %i", pm->score);
            f32 width = font_string_width(current_font, text);
            f32 x = SCREEN_WIDTH/2 - width/2;
            draw_text(tm->render_command_arena, current_font, text, make_v2(x, SCREEN_HEIGHT/2), ORANGE);
        }
        if(pm->score >= WIN_SCORE){
            String8 text = str8_formatted(tm->frame_arena, "CHICKEN DINNER - Score: %i", pm->score);
            f32 width = font_string_width(current_font, text);
            f32 x = SCREEN_WIDTH/2 - width/2;
            draw_text(tm->render_command_arena, current_font, text, make_v2(x, SCREEN_HEIGHT/2), ORANGE);
        }
        String8 text = str8_formatted(tm->frame_arena, "SCORE: %i/%i", pm->score, WIN_SCORE);
        draw_text(tm->render_command_arena, current_font, text, make_v2(50, 50), ORANGE);
        String8 lives = str8_formatted(tm->frame_arena, "LIVES: %i", pm->lives);
        draw_text(tm->render_command_arena, current_font, lives, make_v2(50, 100), ORANGE);

        console_draw();


        frame_count++;
        f64 second_elapsed = clock.get_seconds_elapsed(clock.get_os_timer(), frame_tick_start);
        if(second_elapsed > 1){
            FPS = ((f64)frame_count / second_elapsed);
            frame_tick_start = clock.get_os_timer();
            frame_count = 0;
        }

        //print("frame: at/size %i/%i\nrender: at/size %i/%i\n------------------------------\n", tm->frame_arena->at, tm->frame_arena->size, tm->render_command_arena->at, tm->render_command_arena->size);
        //print("FPS: %f - MSPF: %f - time_dt: %f - accumulator: %lu -  frame_time: %f - second_elapsed: %f - simulations: %i\n", FPS, MSPF, clock.dt, accumulator, frame_time, second_elapsed, simulations);

        String8 fps = str8_formatted(tm->frame_arena, "FPS: %.2f", FPS);
        draw_text(tm->render_command_arena, current_font, fps, make_v2(SCREEN_WIDTH - 250, 50), ORANGE);





        u32 size1 = tm->assets.waves[wave_cursors[0].id].sample_count;
        u32 size2 = tm->assets.waves[wave_cursors[1].id].sample_count;
        u32 size3 = tm->assets.waves[wave_cursors[2].id].sample_count;
        u32 largest_width = size1 > size2 ? size1 : size2;
        largest_width = largest_width > size3 ? largest_width : size3;

        f32 width1 = ((f32)size1 / (f32)largest_width) * 700;
        f32 width2 = ((f32)size2 / (f32)largest_width) * 700;
        f32 width3 = ((f32)size3 / (f32)largest_width) * 700;

        v2 pos;
        v2 dim;
        Quad q = {0};

        // ---------------------------------------------------------
        pos = make_v2(100, 500);
        dim = make_v2(width1, 50);
        q = quad_from_pos(pos, dim);
        draw_quad(tm->render_command_arena, q.p0, q.p1, q.p2, q.p3, ORANGE);

        u32 at1 = (u32)(((f32)wave_cursors[0].at / (f32)size1) * dim.w);
        //print("at1(%i) - at2(%i) - at3(%i)\n", at1, at2, at3);
        //print("at1(%i) - at(%i) - size(%i) - div(%i)\n", at1, wave_cursors[0].at, size1, wave_cursors[0].at/size1);
        //u32 at1 = (u32)((wave_cursors[0].at * (u32)dim.w) / 700);
        q = quad_from_pos(make_v2((pos.x + (f32)at1), pos.y), make_v2(10, 50));
        draw_quad(tm->render_command_arena, q.p0, q.p1, q.p2, q.p3, TEAL);

        String8 vol1_str = str8_formatted(tm->frame_arena, "vol1: %f", wave_cursors[0].volume);
        draw_text(tm->render_command_arena, current_font, vol1_str, make_v2(SCREEN_WIDTH - 450, 525), RED);

        // ---------------------------------------------------------
        pos = make_v2(100, 575);
        dim = make_v2(width2, 50);
        q = quad_from_pos(pos, dim);
        draw_quad(tm->render_command_arena, q.p0, q.p1, q.p2, q.p3, ORANGE);

        u32 at2 = (u32)(((f32)wave_cursors[1].at / (f32)size2) * dim.w);
        q = quad_from_pos(make_v2((pos.x + (f32)at2), pos.y), make_v2(10, 50));
        draw_quad(tm->render_command_arena, q.p0, q.p1, q.p2, q.p3, TEAL);

        String8 vol2_str = str8_formatted(tm->frame_arena, "vol2: %f", wave_cursors[1].volume);
        draw_text(tm->render_command_arena, current_font, vol2_str, make_v2(SCREEN_WIDTH - 450, 600), RED);

        // ---------------------------------------------------------
        pos = make_v2(100, 650);
        dim = make_v2(width3, 50);
        q = quad_from_pos(pos, dim);
        draw_quad(tm->render_command_arena, q.p0, q.p1, q.p2, q.p3, ORANGE);

        u32 at3 = (u32)(((f32)wave_cursors[2].at / (f32)size3) * dim.w);
        q = quad_from_pos(make_v2((pos.x + (f32)at3), pos.y), make_v2(10, 50));
        draw_quad(tm->render_command_arena, q.p0, q.p1, q.p2, q.p3, TEAL);

        String8 vol3_str = str8_formatted(tm->frame_arena, "vol3: %f", wave_cursors[2].volume);
        draw_text(tm->render_command_arena, current_font, vol3_str, make_v2(SCREEN_WIDTH - 450, 675), RED);






        // draw everything
        draw_commands(tm->render_command_arena);
		simulations = 0;
        total_frames++;
        arena_free(tm->frame_arena);
    }
    d3d_release();
    end_profiler();
    audio_release();

    return(0);
}

