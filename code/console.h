#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;

// rects/pos definitions
global Rect output_rect;
global Rect input_rect;
global Rect cursor_rect;
global v2 history_pos;

// some size constraints
global f32 input_height  = 28;
global f32 cursor_height = 24;
global f32 cursor_width  = 10;
global f32 cursor_vertical_padding = 2;

// fonts
global Font output_font;
global Font input_font;

// console colors
global RGBA output_background_color;
global RGBA input_background_color;
global RGBA cursor_color;

// console state
global ConsoleState console_state = CLOSED;

// how much/fast to open
global f32 console_speed = 0.5f;
global f32 console_t  = 0.0f;
global f32 y_closed   = 1.0f;
global f32 y_open     = .7f;
global f32 y_open_big = .2f;

// input u8 array
#define INPUT_MAX_COUNT KB(4)
global u8 console_input[INPUT_MAX_COUNT] = {0};
global u32 input_char_count = 0;

// console history array
#define CONSOLE_HISTORY_MAX KB(1)
static String8 console_history[CONSOLE_HISTORY_MAX] = {0};
static u32 history_count = 0;

static void
init_console(PermanentMemory* pm){
    // everything is positioned relative to the output_rect
    f32 x0 = 0;
    f32 x1 = (f32)resolution.w;
    f32 y0 = (f32)resolution.h;
    f32 y1 = (f32)resolution.h;
    output_rect = make_rect(x0, y0, x1, y1);
    input_rect  = make_rect(x0, y0, x1, y1 + input_height);
    cursor_rect = make_rect(x0 + 10, y0 + cursor_vertical_padding, x0 + 10 + cursor_width, y0 + cursor_height);
    history_pos = make_v2(x0 + 30, y0 + 40);

    // some colors
    output_background_color = {1/255.0f, 57/255.0f, 90/255.0f, 1.0f};
    input_background_color = {0/255.0f, 44/255.0f, 47/255.0f, 1.0f};
    cursor_color = {125/255.0f, 125/255.0f, 125/255.0f, 1.0f};

    // init and load fonts
    input_font.name = str8_literal("GolosText-Regular.ttf");
    input_font.size = 24;
    input_font.color = TEAL;

    output_font.name = str8_literal("Inconsolata-Regular.ttf");
    output_font.size = 24;
    output_font.color = ORANGE;

    bool succeed;
    succeed = load_font_ttf(&pm->arena, pm->fonts_dir, &input_font);
    assert(succeed);
    load_font_glyphs(&pm->arena, &input_font);

    succeed = load_font_ttf(&pm->arena, pm->fonts_dir, &output_font);
    assert(succeed);
    load_font_glyphs(&pm->arena, &output_font);
}

static bool
console_is_open(){
    return(console_state != CLOSED);
}

static bool
console_is_visible(){
    return(output_rect.y0 < resolution.h);
}

static void
console_cursor_reset(){
    cursor_rect.x0 = output_rect.x0 + 10;
    cursor_rect.x1 = output_rect.x0 + 10 + cursor_width;
}

static void
input_add_char(u8 c){
    if(input_char_count < INPUT_MAX_COUNT){
        console_input[input_char_count++] = c;

        Glyph glyph = input_font.glyphs[c];
        cursor_rect.x0 += (glyph.advance_width * input_font.scale);
        cursor_rect.x1 += (glyph.advance_width * input_font.scale);
    }
}

static void
input_remove_char(){
    if(input_char_count > 0){
        u8 c = console_input[--input_char_count];
        Glyph glyph = input_font.glyphs[c];
        cursor_rect.x0 -= (glyph.advance_width * input_font.scale);
        cursor_rect.x1 -= (glyph.advance_width * input_font.scale);
    }
}

static void
console_history_add(String8 str){
    if(history_count < CONSOLE_HISTORY_MAX){
        console_history[history_count] = str;
        history_count++; // these two (history_index + history_count) dont feel correct. Look at draw_console() to understand why I'm doing it.
        input_char_count = 0;
        console_cursor_reset();
    }
}

// maybe draw_console should just draw? What is imm in this case
static void
draw_console(Arena* command_arena){
    if(console_is_visible()){
        // push console rects
        push_rect(command_arena, output_rect, output_background_color);
        push_rect(command_arena, input_rect, input_background_color);
        push_rect(command_arena, cursor_rect, cursor_color);

        // push input string
        if(input_char_count > 0){
            String8 input_str = str8(console_input, input_char_count);
            push_text(command_arena, make_v2(input_rect.x0 + 10, input_rect.y0 + 6), &input_font, input_str);
        }

        // push history in reverse order
        v2 unscaled_offset = {0, 0};
        for(s32 i=history_count-1; i >= 0; --i){
            String8 next_string = console_history[i];
            v2 new_pos = make_v2(history_pos.x, history_pos.y + (unscaled_offset.y * output_font.scale));
            push_text(command_arena, new_pos, &output_font, next_string);
            unscaled_offset.y += output_font.vertical_offset;
        }
    }
}

static void
update_console(){
    // lerp to appropriate position based on state. Everything is positioned based on output_rect.
    f32 lerp_speed =  console_speed * (f32)clock.dt;
    f32 output_rect_bottom = 0;
    if(console_state == CLOSED){
        output_rect_bottom = y_closed * (f32)resolution.h;
    }
    else if(console_state == OPEN){
        output_rect_bottom = y_open * (f32)resolution.h;
    }
    else if(console_state == OPEN_BIG){
        output_rect_bottom = y_open_big * (f32)resolution.h;
    }

    if(console_t < 1) {
        console_t += lerp_speed;

        output_rect.y0 = lerp(output_rect.y0, output_rect_bottom, (console_t));

        input_rect.y0 = output_rect.y0;
        input_rect.y1 = output_rect.y0 + input_height;

        cursor_rect.y0 = output_rect.y0 + cursor_vertical_padding;
        cursor_rect.y1 = output_rect.y0 + cursor_height;

        history_pos.y = output_rect.y0 + 40;
    }
}

static bool
console_handle_events(Event event){
    // handle console events, called from event loop
    if(event.type == TEXT_INPUT){
        if(!console_is_open()) return(false);

        // ignore ~ and ` as they open and close the console
        if(event.keycode != '`' && event.keycode != '~'){
            input_add_char(event.keycode);
            return(true);
        }
        return(false);
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(console_is_open()){
                if(event.keycode == BACKSPACE){
                    input_remove_char();
                    return(true);
                }
                if(event.keycode == ENTER){
                    u8* str = (u8*)push_array(global_arena, u8, input_char_count + 1);
                    mem_copy(str, console_input, input_char_count);

                    String8 input_str = {str, input_char_count};
                    console_history_add(input_str);
                    return(true);
                }
            }
            if(event.keycode == TILDE && !event.repeat){
                console_t = 0;

                if(event.shift_pressed){
                    if(console_state == OPEN_BIG){
                        console_state = CLOSED;
                    }
                    else{ console_state = OPEN_BIG; }
                }
                else{
                    if(console_state == OPEN || console_state == OPEN_BIG){
                        console_state = CLOSED;
                    }
                    else{ console_state = OPEN; }

                }
                return(true);
            }
        }
    }
    return(false);
}

#endif
