#ifndef CONSOLE_C
#define CONSOLE_C

static void
init_console(Arena* arena){ //note: everything is positioned relative to the output_rect
    console.state = CLOSED;

    // load fonts
    bool succeed;
    //succeed = load_font_ttf(arena, str8_literal("fonts/consola.ttf"), &console.font, 24);
    succeed = load_font_ttf(arena, str8_literal("fonts/GolosText-Regular.ttf"), &console.font, 24);
    if(!succeed) { } // incomplete: replace assert with baked default font
    assert(succeed);

    // some size constraints
    console.text_left_pad = 10;

    // how much/fast to open
    console.open_dt = 2.0f;
    console.open_t  = 1.0f;
    console.open_t_target = 1.0f;
    console.y_closed   = 1.0f;
    console.y_open     = .7f;
    console.y_open_big = .2f;

    // some colors
    console.output_background_color = CONSOLE_OUTPUT_BACKGROUND_COLOR;
    console.input_background_color  = CONSOLE_INPUT_BACKGROUND_COLOR;
    console.input_color  = CONSOLE_TEXT_INPUT_COLOR;
    console.output_color = CONSOLE_TEXT_OUTPUT_COLOR;
    console.cursor_color = CONSOLE_CURSOR_COLOR;

    console.output_history_count = 0;
    console.input_history_count = 0;
    console.input_history_index = 0;
    console.input_count = 0;
    console.cursor_index = 0;
}

static bool
console_is_open(){
    return(console.state != CLOSED);
}

static bool
console_is_visible(){
    return(console.open_t < 1);
}

static void
input_add_char(u8 c){
    if(console.input_count < INPUT_COUNT_MAX){
        if(console.cursor_index < console.input_count){
            ScratchArena scratch = begin_scratch(0);

            String8 left = {
                .str = push_array(scratch.arena, u8, console.cursor_index),
                .size = (u64)console.cursor_index,
            };
            mem_copy(left.str, console.input, left.size);
            String8 right = {
                .str = push_array(scratch.arena, u8, console.input_count - console.cursor_index),
                .size = (u64)console.input_count - console.cursor_index,
            };
            mem_copy(right.str, console.input + console.cursor_index, right.size);

            u32 index = 0;
            for(u32 i=0; i < left.size; ++i){
                console.input[index++] = left.str[i];
            }
            console.input[index++] = c;
            for(u32 i=0; i < right.size; ++i){
                console.input[index++] = right.str[i];
            }

            console.input_count++;

            end_scratch(scratch);
        }
        else{
            console.input[console.input_count++] = c;
        }
    }
}

static u8
input_remove_char(){
    u8 c = 0;
    if(console.input_count > 0 && console.cursor_index > 0){
        if(console.cursor_index < console.input_count){
            ScratchArena scratch = begin_scratch(0);

            String8 left = {
                .str = push_array(scratch.arena, u8, console.cursor_index-1),
                .size = (u64)console.cursor_index-1,
            };
            mem_copy(left.str, console.input, left.size);
            String8 right = {
                .str = push_array(scratch.arena, u8, console.input_count - console.cursor_index),
                .size = (u64)console.input_count - console.cursor_index,
            };
            mem_copy(right.str, console.input + console.cursor_index, right.size);

            console.input_count--;
            c = console.input[console.cursor_index - 1];
            u32 index = 0;
            for(u32 i=0; i < left.size; ++i){
                console.input[index++] = left.str[i];
            }
            for(u32 i=0; i < right.size; ++i){
                console.input[index++] = right.str[i];
            }

            end_scratch(scratch);
        }
        else{
            c = console.input[--console.input_count];
        }
    }
    return(c);
}

static void
console_set_state(ConsoleState state){
    console.state = state;
    switch(state){
        case CLOSED:{
            console.open_t_target = console.y_closed;
            break;
        }
        case OPEN:{
            console.open_t_target = console.y_open;
            break;
        }
        case OPEN_BIG:{
            console.open_t_target = console.y_open_big;
            break;
        }
    }

    console_update_openess();
}

static bool
handle_console_events(Event event){
    if(event.type == TEXT_INPUT){
        if(event.keycode != '`' && event.keycode != '~'){
            u8 c = (u8)event.keycode;
            input_add_char(c);
            console.cursor_index++;
            return(true);
        }
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == HOME){
                console.cursor_index = 0;
            }
            if(event.keycode == END){
                for(s32 i=console.cursor_index; i < console.input_count; ++i){
                    u8 c = console.input[i];
                    console.cursor_index++;
                }
            }
            if(event.keycode == ARROW_RIGHT){
                if(console.cursor_index < console.input_count){
                    u8 c = console.input[console.cursor_index];
                    console.cursor_index++;
                }
            }
            if(event.keycode == ARROW_LEFT){
                if(console.cursor_index > 0){
                    console.cursor_index--;
                    u8 c = console.input[console.cursor_index];
                }
            }
            if(event.keycode == BACKSPACE){
                if(console.cursor_index > 0){
                    u8 c = input_remove_char();

                    console.cursor_index--;
                }
                return(true);
            }
            if(event.keycode == ARROW_UP){
                if(console.input_history_index < console.input_history_count){
                    // clear input
                    console.cursor_index = 0;
                    console.input_count = 0;
                    console.input_history_index++;
                    String8 command = console.input_history[console.input_history_count - console.input_history_index];
                    for(u32 i=0; i < command.size; ++i){
                        u8 c = command.str[i];
                        input_add_char(c);
                        console.cursor_index++;
                    }
                    console.input_count = (s32)command.size;
                }
            }
            if(event.keycode == ARROW_DOWN){
                if(console.input_history_index > 0){
                    // clear input
                    console.cursor_index = 0;
                    console.input_count = 0;
                    console.input_history_index--;
                    String8 command = console.input_history[console.input_history_count - console.input_history_index];
                    for(u32 i=0; i < command.size; ++i){
                        u8 c = command.str[i];
                        input_add_char(c);
                        console.cursor_index++;
                    }
                    console.input_count = (s32)command.size;
                }
            }
            if(event.keycode == ENTER){
                u8* line_u8 = (u8*)push_array(global_arena, u8, console.input_count + 1);
                mem_copy(line_u8, console.input, (size_t)console.input_count);

                String8 line_str8 = {line_u8, (u64)console.input_count};
                line_str8 = str8_eat_whitespace(line_str8);

                u64 args_count = parse_line_args(line_str8);
                if(!args_count){ return(false); }

                console.input_history[console.input_history_count++] = line_str8;
                run_command(line_str8);

                // clear_input
                console.cursor_index = 0;
                console.input_count = 0;
                console.input_history_index = 0;

                return(true);
            }
        }
    }
    return(false);
}

static void
console_update_openess(){
    f32 open_d = console.open_dt * (f32)clock.dt;
    if(console.open_t < console.open_t_target){
        console.open_t += open_d;
        if(console.open_t > console.open_t_target){
            console.open_t = console.open_t_target;
        }
    }
    else if(console.open_t > console.open_t_target){
        console.open_t -= open_d;
        if(console.open_t < 0){
            console.open_t = 0;
        }
    }
}

static void
console_draw(){
    // rect setup
    Rect output_rect = {0};
    output_rect.x0 = 0;
    output_rect.y0 = console.open_t * (f32)window.height;
    output_rect.x1 = (f32)window.width;
    output_rect.y1 = (f32)window.height;

    Rect input_rect = {0};
    input_rect.x0 = 0;
    input_rect.y0 = output_rect.y0 - (f32)console.font.vertical_offset * console.font.scale;
    input_rect.x1 = (f32)window.width;
    input_rect.y1 = output_rect.y0;

    Rect cursor_rect = {0};
    u8 char_at_cursor = 0; // note: defaults to 'a' when nothing is located at cursor_index
    if(console.input_count == console.cursor_index){
        char_at_cursor = 'a';
    }
    else{
        char_at_cursor = console.input[console.cursor_index];
    }
    String8 str = str8(console.input, (u64)console.cursor_index);
    cursor_rect.x0 = console.text_left_pad + font_char_width(console.font, '>') + font_string_width(console.font, str);
    cursor_rect.y0 = input_rect.y0;
    cursor_rect.x1 = cursor_rect.x0 + font_char_width(console.font, char_at_cursor);
    cursor_rect.y1 = input_rect.y1;

    // draw regions
    push_quad(render_command_arena, output_rect, console.output_background_color);
    push_quad(render_command_arena, input_rect, console.input_background_color);
    push_quad(render_command_arena, cursor_rect, console.cursor_color);

    // draw input
    f32 input_pos_y = (f32)window.height - (input_rect.y0 - ((f32)console.font.descent * console.font.scale));
    push_text(render_command_arena, console.font, str8_literal(">"), console.text_left_pad, input_pos_y, console.input_color);
    if(console.input_count > 0){
        String8 input_str = str8(console.input, (u64)console.input_count);
        push_text(render_command_arena, console.font, input_str, console.text_left_pad + font_char_width(console.font, '>'), input_pos_y, console.input_color);
    }

    // draw history in reverse order, but only if its on screen
    if(console.output_history_count > 0){
        f32 output_pos_y = (f32)window.height - (output_rect.y0 - ((f32)console.font.descent * console.font.scale));
        for(s32 i=console.output_history_count-1; i >= 0; --i){
            if(output_pos_y < (f32)window.height){
                String8 next_string = console.output_history[i];
                push_text(render_command_arena, console.font, next_string, console.text_left_pad, output_pos_y, console.output_color);
                output_pos_y -= (f32)console.font.vertical_offset * console.font.scale;
            }
        }
    }
};

#endif
