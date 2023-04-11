#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;


global Rect output_rect;
global Rect input_rect;
global Rect cursor_rect;
global v2 history_pos;

global RGBA output_color;
global RGBA input_color;
global RGBA cursor_color;

global ConsoleState console_state = OPEN_BIG;

global f32 console_t  = 0.0f;
global f32 y_closed   = 1.0f;
global f32 y_open     = .7f;
global f32 y_open_big = .2f;
global f32 open_speed = 0.5f;

global f32 input_height  = 28;
global f32 cursor_height = 20;
global f32 cursor_width  = 10;
global f32 cursor_vertical_padding  = 2;
global f32 cursor_index  = 0;

global f32 start_position = y_closed;

#define INPUT_MAX_COUNT KB(4)
global u8 console_input[INPUT_MAX_COUNT] = {0};
global u32 input_count = 0;

static String8 console_history[KB(1)] = {0};
static u32 history_count = 0;

static void
cursor_increment_index(){
    if(cursor_index < INPUT_MAX_COUNT){
        cursor_index++;
    }
}

static void
cursor_decrement_index(){
    if(cursor_index > 0){
        cursor_index--;
    }
}

static s32
get_cursor_offset(String8 string, Font* font){
    s32 x_offset = 0;
    s32 kern;

    for(s32 i=0; i < cursor_index; ++i){
        u8* c = string.str + i;

        s32 advance_width, lsb;
        stbtt_GetCodepointHMetrics(&font->info, *c, &advance_width, &lsb);
        // advance x + kern
        x_offset += advance_width;
        if(string.str[i + 1]){
            kern = stbtt_GetCodepointKernAdvance(&font->info, *c, string.str[i+1]);
            x_offset += kern;
        }

    }
    x_offset = round_f32_s32(x_offset * font->scale);
    return(x_offset);
}

static void
input_add_char(u8 c){
    assert(input_count < INPUT_MAX_COUNT);

    console_input[input_count++] = c;
    cursor_increment_index();
    //s32 x_offset = get_cursor_offset(input_str, &font_incon);
    //cursor_rect.x0 += x_offset;
    //cursor_rect.x1 += x_offset;
}

static void
input_remove_char(){
    if(input_count > 0){
        input_count--;
    }
}

static void
init_console(){
    f32 x0 = 0;
    f32 x1 = (f32)resolution.w;
    f32 y0 = (f32)resolution.h;
    f32 y1 = (f32)resolution.h;
    output_rect = make_rect(x0, y0, x1, y1);
    input_rect  = make_rect(x0, y0 - input_height, x1, y1);
    cursor_rect = make_rect(x0 + 10, y0 - input_height + cursor_vertical_padding, x0 + 10 + cursor_width, input_rect.y1 + cursor_height);
    history_pos = make_v2(x0 + 10, y0 + 10);

    output_color = {1/255.0f, 57/255.0f, 90/255.0f, 1.0f};
    input_color = {0/255.0f, 44/255.0f, 47/255.0f, 1.0f};
    cursor_color = {125/255.0f, 125/255.0f, 125/255.0f, 1.0f};

    input_count = 0;
}

static bool
console_is_open(){
    return(output_rect.y0 > 0);
}

static void
console_set_state(ConsoleState s){
    console_state = s;
}


// maybe draw_console should just draw? What is imm in this case
static void
push_console(Arena* command_arena){
    push_rect(command_arena, output_rect, output_color);
    push_rect(command_arena, input_rect, input_color);
    push_rect(command_arena, cursor_rect, cursor_color);
    //for(u32 i=0; i < history_length; ++i){
    //    String8 next_string = history_length[i];
    //    push_text(command_arena, next_string);
    //}
    //push_text(command_arena, String8 console_input_string);
}

// CONSIDER, INCOMPLETE: Maybe we don't want to enlarge the rect, and simple move it down and up as a solid piece?
static void
update_console(){
    f32 lerp_speed =  open_speed * (f32)clock.dt;
    if(console_state == CLOSED){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_closed * (f32)resolution.h, console_t);

            input_rect.y0 = lerp(input_rect.y0, y_closed * (f32)resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;

            cursor_rect.y0 = lerp(cursor_rect.y0, (y_closed * (f32)resolution.h) + cursor_vertical_padding, console_t);
            cursor_rect.y1 = input_rect.y0 + input_height - cursor_vertical_padding;

            history_pos.y = lerp(history_pos.y, (y_closed * (f32)resolution.h) + cursor_vertical_padding, console_t);
        }
    }
    else if(console_state == OPEN){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_open * (f32)resolution.h, console_t);

            input_rect.y0 = lerp(input_rect.y0, y_open * (f32)resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;

            cursor_rect.y0 = lerp(cursor_rect.y0, (y_open * (f32)resolution.h) + cursor_vertical_padding, console_t);
            cursor_rect.y1 = input_rect.y0 + input_height - cursor_vertical_padding;

            history_pos.y = lerp(history_pos.y, (y_open * (f32)resolution.h) + cursor_vertical_padding, console_t);
        }
    }
    else if(console_state == OPEN_BIG){
        if(console_t < 1) {
            console_t += lerp_speed;

            output_rect.y0 = lerp(output_rect.y0, y_open_big * (f32)resolution.h, console_t);

            input_rect.y0 = lerp(input_rect.y0, (y_open_big * (f32)resolution.h) - 10, console_t);
            input_rect.y1 = input_rect.y0 + input_height;

            cursor_rect.y0 = lerp(cursor_rect.y0, (y_open_big * (f32)resolution.h) + cursor_vertical_padding, console_t);
            cursor_rect.y1 = input_rect.y0 + input_height - cursor_vertical_padding;

            history_pos.y = output_rect.y0;
        }
    }
}

#endif
