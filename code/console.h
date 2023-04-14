#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;

#define INPUT_MAX_COUNT KB(4)
#define CONSOLE_HISTORY_MAX KB(1)
typedef struct Console{
    ConsoleState state;

    Rect output_rect;
    Rect input_rect;
    Rect cursor_rect;
    v2 history_pos;

    RGBA output_background_color;
    RGBA input_background_color;
    RGBA cursor_color;

    u8 input[INPUT_MAX_COUNT];
    u32 input_char_count;

    String8 history[CONSOLE_HISTORY_MAX];
    u32 history_count;

    Font output_font;
    Font input_font;
} Console;
global Console console;

// some size constraints
global f32 input_height  = 28;
global f32 cursor_height = 24;
global f32 cursor_width  = 10;
global f32 cursor_vertical_padding = 2;

// console colors
// how much/fast to open
global f32 console_speed = 0.5f;
global f32 console_t  = 0.0f;
global f32 y_closed   = 1.0f;
global f32 y_open     = .7f;
global f32 y_open_big = .2f;

static void
init_console(PermanentMemory* pm){
    // everything is positioned relative to the output_rect
    console.state = CLOSED;
    f32 x0 = 0;
    f32 x1 = (f32)resolution.w;
    f32 y0 = (f32)resolution.h;
    f32 y1 = (f32)resolution.h;
    console.output_rect = make_rect(x0, y0, x1, y1);
    console.input_rect  = make_rect(x0, y0, x1, y1 + input_height);
    console.cursor_rect = make_rect(x0 + 10, y0 + cursor_vertical_padding, x0 + 10 + cursor_width, y0 + cursor_height);
    console.history_pos = make_v2(x0 + 30, y0 + 40);

    // some colors
    console.output_background_color = {1/255.0f, 57/255.0f, 90/255.0f, 1.0f};
    console.input_background_color = {0/255.0f, 44/255.0f, 47/255.0f, 1.0f};
    console.cursor_color = {125/255.0f, 125/255.0f, 125/255.0f, 1.0f};

    // init and load fonts
    console.input_font.name = str8_literal("GolosText-Regular.ttf");
    console.input_font.size = 24;
    console.input_font.color = TEAL;

    console.output_font.name = str8_literal("Inconsolata-Regular.ttf");
    console.output_font.size = 24;
    console.output_font.color = ORANGE;

    bool succeed;
    succeed = load_font_ttf(&pm->arena, pm->fonts_dir, &console.input_font);
    assert(succeed);
    load_font_glyphs(&pm->arena, &console.input_font);

    succeed = load_font_ttf(&pm->arena, pm->fonts_dir, &console.output_font);
    assert(succeed);
    load_font_glyphs(&pm->arena, &console.output_font);
}

static bool
console_is_open(){
    return(console.state != CLOSED);
}

static bool
console_is_visible(){
    return(console.output_rect.y0 < resolution.h);
}

static void
console_cursor_reset(){
    console.cursor_rect.x0 = console.output_rect.x0 + 10;
    console.cursor_rect.x1 = console.output_rect.x0 + 10 + cursor_width;
}

static void
input_add_char(u8 c){
    if(console.input_char_count < INPUT_MAX_COUNT){
        console.input[console.input_char_count++] = c;

        Glyph glyph = console.input_font.glyphs[c];
        console.cursor_rect.x0 += (glyph.advance_width * console.input_font.scale);
        console.cursor_rect.x1 += (glyph.advance_width * console.input_font.scale);
    }
}

static void
input_remove_char(){
    if(console.input_char_count > 0){
        u8 c = console.input[--console.input_char_count];
        Glyph glyph = console.input_font.glyphs[c];
        console.cursor_rect.x0 -= (glyph.advance_width * console.input_font.scale);
        console.cursor_rect.x1 -= (glyph.advance_width * console.input_font.scale);
    }
}

static void
console_history_add(String8 str){
    if(console.history_count < CONSOLE_HISTORY_MAX){
        console.history[console.history_count] = str;
        console.history_count++;
        console.input_char_count = 0;
        console_cursor_reset();
    }
}

static void
push_console(Arena* command_arena){
    if(console_is_visible()){
        // push console rects
        push_rect(command_arena, console.output_rect, console.output_background_color);
        push_rect(command_arena, console.input_rect, console.input_background_color);
        push_rect(command_arena, console.cursor_rect, console.cursor_color);

        // push input string
        if(console.input_char_count > 0){
            String8 input_str = str8(console.input, console.input_char_count);
            push_text(command_arena, make_v2(console.input_rect.x0 + 10, console.input_rect.y0 + 6), &console.input_font, input_str);
        }

        // push history in reverse order, but only if its on screen
        f32 unscaled_y_offset = 0.0f;
        for(s32 i=console.history_count-1; i >= 0; --i){
            if(console.history_pos.y + (unscaled_y_offset * console.output_font.scale) < resolution.h){
                String8 next_string = console.history[i];
                v2 new_pos = make_v2(console.history_pos.x, console.history_pos.y + (unscaled_y_offset * console.output_font.scale));
                push_text(command_arena, new_pos, &console.output_font, next_string);
                unscaled_y_offset += console.output_font.vertical_offset;
            }
        }
    }
}

static void
update_console(){
    // lerp to appropriate position based on state. Everything is positioned based on output_rect.
    f32 lerp_speed =  console_speed * (f32)clock.dt;
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
handle_console_event(Event event){
    if(event.type == TEXT_INPUT){
        if(event.keycode != '`' && event.keycode != '~'){
            input_add_char(event.keycode);
            return(true);
        }
        return(false);
    }
    if(event.type == KEYBOARD){
        if(event.key_pressed){
            if(event.keycode == BACKSPACE){
                input_remove_char();
                return(true);
            }
            if(event.keycode == ENTER){
                u8* str = (u8*)push_array(global_arena, u8, console.input_char_count + 1);
                mem_copy(str, console.input, console.input_char_count);

                String8 input_str = {str, console.input_char_count};
                console_history_add(input_str);
                return(true);
            }
        }
    }
    return(false);
}

#endif
