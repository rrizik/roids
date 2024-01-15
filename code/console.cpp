#ifndef CONSOLE_C
#define CONSOLE_C

static void
init_console(Arena* arena){ //note: everything is positioned relative to the output_rect
    console.state = CLOSED;

    // init and load fonts
    bool succeed;
    succeed = load_font_ttf(arena, str8_literal("fonts/GolosText-Regular.ttf"), &console.input_font, 24);
    assert(succeed);
    succeed = load_font_ttf(arena, str8_literal("fonts/GolosText-Regular.ttf"), &console.output_font, 24);
    assert(succeed);

    // some size constraints
    input_height = 28;
    input_pos_x  = 25;
    output_pos_x = 10;
    cursor_height = 24;
    cursor_vertical_padding = 2;

    // how much/fast to open
    console_speed = 0.5f;
    console_t  = 0.0f;
    y_closed   = 1.0f;
    y_open     = .7f;
    y_open_big = .2f;

    // rect initialization
    f32 x0 = 0;
    f32 x1 = (f32)resolution.w;
    f32 y0 = (f32)resolution.h;
    f32 y1 = (f32)resolution.h;
    console.output_rect = make_rect(x0, y0, x1, y1);
    console.input_rect  = make_rect(x0, y0, x1, y1 + input_height);
    console.cursor_rect = make_rect(x0 + input_pos_x, y0 + cursor_vertical_padding, 0, y0 + cursor_height);
    console_cursor_update_width();
    console.history_pos = make_v2(output_pos_x, y0 + 40);

    // input prefix
    input_prefix_char = GREATER_THAN;

    // some colors
    console.output_background_color = CONSOLE_OUTPUT_BACKGROUND_COLOR;
    console.input_background_color = CONSOLE_INPUT_BACKGROUND_COLOR;
    console.input_color = CONSOLE_TEXT_INPUT_COLOR;
    console.output_color = CONSOLE_TEXT_OUTPUT_COLOR;
    console.cursor_color = CONSOLE_CURSOR_COLOR;
}

static bool
console_is_open(){
    return(console.state != CLOSED);
}

static bool
console_is_visible(){
    return(console.output_rect.y0 < (f32)resolution.h);
}

static void
console_cursor_update_pos(u8 c, s32 dir){
    if(c == 0){ // reset chursor position to default
        console.cursor_rect.x0 = console.output_rect.x0 + input_pos_x;
    }
    else{
        s32 advance_width, lsb;
        stbtt_GetCodepointHMetrics(&console.input_font.info, c, &advance_width, &lsb);
        console.cursor_rect.x0 += (f32)dir * ((f32)advance_width * console.input_font.scale);
    }
}

static void
console_cursor_update_width(){
    s32 advance_width, lsb;
    u8 curr_c;
    if(console.cursor_index == console.input.count){
        curr_c = 'a';
    }
    else{
        curr_c = console.input.array[console.cursor_index];
    }
    stbtt_GetCodepointHMetrics(&console.input_font.info, curr_c, &advance_width, &lsb);
    console.cursor_rect.x1 = console.cursor_rect.x0 + ((f32)advance_width * console.input_font.scale);
}

static void
console_clear_input(){
    console.input.count = 0;
}

static void
input_add_char(u8 c){
    if(console.input.count < INPUT_MAX_COUNT){
        if(console.cursor_index < console.input.count){
            ScratchArena scratch = begin_scratch(0);

            String8 left = {
                .str = push_array(scratch.arena, u8, console.cursor_index),
                .size = console.cursor_index,
            };
            mem_copy(left.str, console.input.array, left.size);
            String8 right = {
                .str = push_array(scratch.arena, u8, console.input.count - console.cursor_index),
                .size = console.input.count - console.cursor_index,
            };
            mem_copy(right.str, console.input.array + console.cursor_index, right.size);

            u32 index = 0;
            for(u32 i=0; i < left.size; ++i){
                console.input.array[index++] = left.str[i];
            }
            console.input.array[index++] = c;
            for(u32 i=0; i < right.size; ++i){
                console.input.array[index++] = right.str[i];
            }

            console.input.count++;

            end_scratch(scratch);
        }
        else{
            console.input.array[console.input.count++] = c;
        }
    }
}

static u8
input_remove_char(){
    u8 c = 0;
    if(console.input.count > 0 && console.cursor_index > 0){
        if(console.cursor_index < console.input.count){
            ScratchArena scratch = begin_scratch(0);

            String8 left = {
                .str = push_array(scratch.arena, u8, console.cursor_index-1),
                .size = console.cursor_index-1,
            };
            mem_copy(left.str, console.input.array, left.size);
            String8 right = {
                .str = push_array(scratch.arena, u8, console.input.count - console.cursor_index),
                .size = console.input.count - console.cursor_index,
            };
            mem_copy(right.str, console.input.array + console.cursor_index, right.size);

            console.input.count--;
            c = console.input.array[console.cursor_index - 1];
            u32 index = 0;
            for(u32 i=0; i < left.size; ++i){
                console.input.array[index++] = left.str[i];
            }
            for(u32 i=0; i < right.size; ++i){
                console.input.array[index++] = right.str[i];
            }

            end_scratch(scratch);
        }
        else{
            c = console.input.array[--console.input.count];
        }
    }
    return(c);
}

static void
draw_console(){
    // draw console background, input, cursor
    d3d_draw_quad(console.output_rect, console.output_background_color);
    d3d_draw_quad(console.input_rect, console.input_background_color);
    d3d_draw_quad(console.cursor_rect, console.cursor_color);

    // draw input
    d3d_draw_text(console.input_font, output_pos_x, (f32)resolution.h - (console.input_rect.y0 + 6), console.input_color, str8_literal(">"));
    if(console.input.count > 0){
        String8 input_str = str8(console.input.array, console.input.count);
        d3d_draw_text(console.input_font, input_pos_x, (f32)resolution.h - (console.input_rect.y0 + 6), console.input_color, input_str);
    }

    // draw history in reverse order, but only if its on screen
    f32 unscaled_y_offset = 0.0f;
    for(u64 i=console.output_history.count-1; i < console.output_history.count; --i){
        if(console.history_pos.y + (unscaled_y_offset * console.output_font.scale) < (f32)resolution.h){
            String8 next_string = console.output_history.array[i];

            d3d_draw_text(console.output_font, console.history_pos.x, (f32)resolution.h - (console.history_pos.y + (unscaled_y_offset * console.output_font.scale)), console.output_color, next_string);

            unscaled_y_offset += (f32)console.output_font.vertical_offset;
        }
    }

};

static void
update_console(){
    f32 output_rect_bottom = 0;
    switch(console.state){
        case CLOSED:{
            output_rect_bottom = y_closed * (f32)resolution.h;
            break;
        }
        case OPEN:{
            output_rect_bottom = y_open * (f32)resolution.h;
            break;
        }
        case OPEN_BIG:{
            output_rect_bottom = y_open_big * (f32)resolution.h;
            break;
        }
    }

    // lerp to appropriate position based on state. Everything is positioned based on output_rect.
    f32 lerp_speed =  console_speed * (f32)clock.dt;
    if(console_t < 1) {
        console_t += lerp_speed;

        console.output_rect.y0 = lerp(console.output_rect.y0, output_rect_bottom, (console_t));

        console.input_rect.y0 = console.output_rect.y0;
        console.input_rect.y1 = console.output_rect.y0 + input_height;

        console.cursor_rect.y0 = console.output_rect.y0 + cursor_vertical_padding;
        console.cursor_rect.y1 = console.output_rect.y0 + cursor_height;

        console.history_pos.y = console.output_rect.y0 + 40;
    }
}

static bool
handle_console_events(Event event){
    if(event.type == TEXT_INPUT){
        if(event.keycode != '`' && event.keycode != '~'){
            u8 c = (u8)event.keycode;
            input_add_char(c);
            console.cursor_index++;
            console_cursor_update_pos(c, 1);
            console_cursor_update_width();
            return(true);
        }
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == HOME){
                console.cursor_index = 0;
                console_cursor_update_pos(0, 0);
                console_cursor_update_width();
            }
            if(event.keycode == END){
                for(u32 i=console.cursor_index; i < console.input.count; ++i){
                    u8 c = console.input.array[i];
                    console_cursor_update_pos(c, 1);
                    console.cursor_index++;
                    console_cursor_update_width();
                }
            }
            if(event.keycode == ARROW_RIGHT){
                if(console.cursor_index < console.input.count){
                    u8 c = console.input.array[console.cursor_index];
                    console_cursor_update_pos(c, 1);
                    console.cursor_index++;
                    console_cursor_update_width();
                }
            }
            if(event.keycode == ARROW_LEFT){
                if(console.cursor_index > 0){
                    console.cursor_index--;
                    u8 c = console.input.array[console.cursor_index];
                    console_cursor_update_pos(c, -1);
                    console_cursor_update_width();
                }
            }
            if(event.keycode == BACKSPACE){
                u8 c = input_remove_char();

                console.cursor_index--;
                console_cursor_update_pos(c, -1);
                console_cursor_update_width();
                return(true);
            }
            if(event.keycode == ARROW_UP){
                if(console.input_history_index < console.input_history.count){
                    console.cursor_index = 0;
                    console_cursor_update_pos(0, 0);
                    console_cursor_update_width();

                    console_clear_input();
                    console.input_history_index++;
                    String8 command = console.input_history.array[console.input_history.count - console.input_history_index];
                    for(u32 i=0; i < command.size; ++i){
                        u8 c = command.str[i];
                        input_add_char(c);
                        console.cursor_index++;
                        console_cursor_update_pos(c, 1);
                        console_cursor_update_width();
                    }
                    console.input.count = (u32)command.size;
                }
            }
            if(event.keycode == ARROW_DOWN){
                if(console.input_history_index > 0){
                    console.cursor_index = 0;
                    console_cursor_update_pos(0, 0);
                    console_cursor_update_width();

                    console_clear_input();
                    console.input_history_index--;
                    String8 command = console.input_history.array[console.input_history.count - console.input_history_index];
                    for(u32 i=0; i < command.size; ++i){
                        u8 c = command.str[i];
                        input_add_char(c);
                        console.cursor_index++;
                        console_cursor_update_pos(c, 1);
                        console_cursor_update_width();
                    }
                    console.input.count = (u32)command.size;
                }
            }
            if(event.keycode == ENTER){
                u8* line_u8 = (u8*)push_array(global_arena, u8, console.input.count + 1);
                mem_copy(line_u8, console.input.array, console.input.count);

                String8 line_str8 = {line_u8, console.input.count};
                line_str8 = str8_eat_whitespace(line_str8);

                u64 args_count = parse_line_args(line_str8);
                if(!args_count){ return(false); }

                array_add(&console.input_history, line_str8);
                run_command(line_str8);

                console.cursor_index = 0;
                console_cursor_update_pos(0, 0);
                console_cursor_update_width();

                console_clear_input();
                console.input_history_index = 0;

                return(true);
            }
        }
    }
    return(false);
}

#endif
