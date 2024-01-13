#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;

array_define(String8, KB(1), OutputHistory);
array_define(String8, KB(1), InputHistory);
array_define(u8, KB(4), Input);

#define INPUT_MAX_COUNT KB(4)
typedef struct Console{
    ConsoleState state;

    Rect output_rect;
    Rect input_rect;
    Rect cursor_rect;
    v2 history_pos;

    RGBA output_background_color;
    RGBA input_background_color;
    RGBA input_color;
    RGBA output_color;
    RGBA cursor_color;
    u32  cursor_index;

    // TODO:INCOMPLETE:why not do String8 here?
    //u8 input[INPUT_MAX_COUNT];
    //u32 input_char_count;

    Input input;
    OutputHistory output_history;
    InputHistory input_history;
    u32 input_history_at;

    Font output_font;
    Font input_font;
    Font command_history_font;
} Console;
global Console console;

// some size constraints
global f32 input_height;
global f32 cursor_height;
global f32 cursor_width;
global f32 cursor_vertical_padding;
global u8 prefix_char;

// how much/fast to open
global f32 console_speed;
global f32 console_t;
global f32 y_closed;
global f32 y_open;
global f32 y_open_big;

static void init_console(Arena* arena);
static bool console_is_open();
static bool console_is_visible();
static void console_cursor_reset();
static void console_clear_input();
static void input_add_char(u8 c);
static void input_remove_char();
static void draw_console();
static void update_console();
static bool handle_console_event(Event event);

#include "command.h"
#endif
