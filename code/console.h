#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;


Rect output_rect;
Rect input_rect;
ConsoleState console_state = OPEN_BIG;

f32 console_t  = 0.0f;
f32 y_closed   = 1.0f;
f32 y_open     = .7f;
f32 y_open_big = .2f;
f32 open_speed = 0.5f;
f32 input_height = 28;

f32 start_position = y_closed;

RGBA output_color;
RGBA input_color;

static void
init_console(){
    f32 x0 = 0;
    f32 y0 = y_closed * (f32)resolution.h;
    f32 x1 = (f32)resolution.w;
    f32 y1 = (f32)resolution.h;
    output_rect = make_rect(x0, y0, x1, y1);
    input_rect  = make_rect(x0, y0 - 10, x1, y1);

    output_color = {0.50f, 0.50f, 0.50f,  1.0f};
    input_color  = {0.25f, 0.25f, 0.23f,  1.0f};
}

static bool
console_is_open(){
    return(output_rect.y0 > 0);
}

static void
console_set_state(ConsoleState s){
    console_state = s;
}

static void
console_mark(){
    console_t = 0;
    start_position = output_rect.y0;
}

// maybe draw_console should just draw? What is imm in this case
static void
push_console(Arena* command_arena){
    push_rect(command_arena, output_rect, output_color);
    push_rect(command_arena, input_rect, input_color);
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
            input_rect.y0 = lerp(output_rect.y0, y_closed * (f32)resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;
        }
    }
    else if(console_state == OPEN){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_open * (f32)resolution.h, console_t);
            input_rect.y0 = lerp(output_rect.y0, y_open * (f32)resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;
        }
    }
    else if(console_state == OPEN_BIG){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_open_big * (f32)resolution.h, console_t);
            input_rect.y0 = lerp(output_rect.y0, y_open_big * (f32)resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;
        }
    }
}

#endif
