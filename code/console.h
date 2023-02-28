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
    f32 y0 = y_closed * resolution.h;
    f32 x1 = resolution.w;
    f32 y1 = resolution.h;
    output_rect = make_rect(x0, y0, x1, y1);
    input_rect  = make_rect(x0, y0 - 10, x1, y1);

    output_color = {0.50f, 0.50, 0.50,  1.0f};
    input_color  = {0.25f, 0.25, 0.23,  1.0f};
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

static void
draw_console(Arena* command_arena){
    push_rect(command_arena, output_rect, output_color);
    push_rect(command_arena, input_rect, input_color);
}

static void
update_console(){
    f32 lerp_speed =  open_speed * clock.dt;
    if(console_state == CLOSED){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_closed * resolution.h, console_t);
            input_rect.y0 = lerp(output_rect.y0, y_closed * resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;
        }
    }
    else if(console_state == OPEN){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_open * resolution.h, console_t);
            input_rect.y0 = lerp(output_rect.y0, y_open * resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;
        }
    }
    else if(console_state == OPEN_BIG){
        if(console_t < 1) {
            console_t += lerp_speed;
            output_rect.y0 = lerp(output_rect.y0, y_open_big * resolution.h, console_t);
            input_rect.y0 = lerp(output_rect.y0, y_open_big * resolution.h, console_t);
            input_rect.y1 = input_rect.y0 + input_height;
        }
    }
}

#endif
