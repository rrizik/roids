#ifndef CONSOLE_C
#define CONSOLE_C

static void
init_console(Arena* arena, Window* window, Assets* assets){ //note: everything is positioned relative to the output_rect
    console.state = CLOSED;
    console.font = &assets->fonts[FontAsset_Consolas];
    console.window = window;
    console.arena = arena;

    // some size constraints
    console.text_left_pad = 10;

    // how much/fast to open
    console.open_dt = 60.0f;
    console.open_t  = 0.0f;
    console.open_t_target = 0.0f;

    console.y_closed   = 0.0f;
    console.y_open     = 0.4f;
    console.y_open_big = 0.7f;

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

    init_console_commands();
}

static bool
console_is_open(void){
    return(console.state != CLOSED);
}

static bool
console_is_visible(void){
    return(console.open_t > 0);
}

static u8
console_char_at_cursor(void){
    u8 result = 0;
    if(console.input_count == console.cursor_index){
        result = 'a';
    }
    else{
        result = console.input[console.cursor_index];
    }
    return(result);
}

static void
input_add_char(u8 c){
    if(console.input_count < INPUT_COUNT_MAX){
        if(console.cursor_index < console.input_count){
            for(s32 i=console.input_count; i > console.cursor_index; --i){
                console.input[i] = console.input[i - 1];
            }
            console.input_count++;
            console.input[console.cursor_index] = c;
        }
        else{
            console.input[console.input_count++] = c;
        }
    }
}

static void
input_remove_char(void){
    u8 c = 0;
    if(console.input_count > 0 && console.cursor_index > 0){
        if(console.cursor_index < console.input_count){
            for(s32 i=console.cursor_index; i < console.input_count; ++i){
                console.input[i - 1] = console.input[i];
            }
            --console.input_count;
        }
        else{
            --console.input_count;
        }
    }
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
            if(event.keycode == KeyCode_HOME){
                console.cursor_index = 0;
            }
            if(event.keycode == KeyCode_END){
                for(s32 i=console.cursor_index; i < console.input_count; ++i){
                    u8 c = console.input[i];
                    console.cursor_index++;
                }
            }
            if(event.keycode == KeyCode_RIGHT){
                if(console.cursor_index < console.input_count){
                    u8 c = console.input[console.cursor_index];
                    console.cursor_index++;
                }
            }
            if(event.keycode == KeyCode_LEFT){
                if(console.cursor_index > 0){
                    console.cursor_index--;
                    u8 c = console.input[console.cursor_index];
                }
            }
            if(event.keycode == KeyCode_BACKSPACE){
                if(console.cursor_index > 0){
                    input_remove_char();

                    console.cursor_index--;
                }
                return(true);
            }
            if(event.keycode == KeyCode_UP){
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
            if(event.keycode == KeyCode_DOWN){
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
            if(event.keycode == KeyCode_ENTER){
                u8* line_u8 = (u8*)push_array(console.arena, u8, console.input_count + 1);
                memcpy(line_u8, console.input, (size_t)console.input_count);

                String8 line_str8 = {line_u8, (u64)console.input_count};
                line_str8 = str8_eat_spaces(line_str8);

                u64 args_count = command_parse_args(line_str8);
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
console_update(void){
    f32 epsilon = 0.001f;
    f32 open_d = console.open_dt * (f32)clock.dt;
    if(console.open_t < console.open_t_target){
        console.open_t = lerp(console.open_t, console.open_t_target, smoothstep(open_d));
        if(console.open_t > console.open_t_target - epsilon){
            console.open_t = console.open_t_target;
        }
    }
    else if(console.open_t > console.open_t_target){
        console.open_t = lerp(console.open_t, console.open_t_target, smoothstep(open_d));
        if(console.open_t < epsilon){
            console.open_t = 0;
        }
    }
}

static void
console_draw(void){
    if(console_is_visible()){
        Font* font = console.font;

        // rect setup
        f32 y = (f32)(-font->vertical_offset);
        v2 output_p0 = make_v2(0                , y);
        v2 output_p1 = make_v2((f32)console.window->width, y);
        v2 output_p2 = make_v2((f32)console.window->width, (console.open_t * (f32)console.window->height) + y);
        v2 output_p3 = make_v2(0                , (console.open_t * (f32)console.window->height) + y);

        v2 input_p0 = make_v2(0                , output_p2.y);
        v2 input_p1 = make_v2((f32)console.window->width, output_p2.y);
        v2 input_p2 = make_v2((f32)console.window->width, output_p2.y + (f32)font->vertical_offset);
        v2 input_p3 = make_v2(0                , output_p2.y + (f32)font->vertical_offset);

        String8 str = str8(console.input, (u64)console.cursor_index);

        v2 cursor_p0 = make_v2(console.text_left_pad + font_char_width(font, '>') + font_string_width(font, str), input_p0.y);
        v2 cursor_p1 = make_v2(cursor_p0.x + font_char_width(font, console_char_at_cursor()), input_p0.y);
        v2 cursor_p2 = make_v2(cursor_p0.x + font_char_width(font, console_char_at_cursor()), input_p2.y);
        v2 cursor_p3 = make_v2(console.text_left_pad + font_char_width(font, '>') + font_string_width(font, str), input_p2.y);

        // draw regions
        draw_quad(output_p0, output_p1, output_p2, output_p3, console.output_background_color);
        draw_quad(input_p0, input_p1, input_p2, input_p3, console.input_background_color);
        draw_quad(cursor_p0, cursor_p1, cursor_p2, cursor_p3, console.cursor_color);

        // draw input
        f32 input_pos_y = input_p2.y + ((f32)font->descent * font->scale);
        draw_text(console.font, str8_literal(">"), make_v2(console.text_left_pad, input_pos_y), console.input_color);
        if(console.input_count > 0){
            String8 input_str = str8(console.input, (u64)console.input_count);
            draw_text(console.font, input_str, make_v2(console.text_left_pad + font_char_width(font, '>'), input_pos_y), console.input_color);
        }

        // draw history (in reverse order and only if its on screen)
        if(console.output_history_count > 0){
            f32 output_pos_y = output_p2.y + ((f32)font->descent * font->scale);
            for(s32 i=console.output_history_count-1; i >= 0; --i){
                if(output_pos_y < (f32)console.window->height){
                    String8 next_string = console.output_history[i];
                    draw_text(console.font, next_string, make_v2(console.text_left_pad, output_pos_y), console.output_color);
                    output_pos_y -= (f32)font->vertical_offset;
                }
            }
        }
    }
};

#endif
