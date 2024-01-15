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
    u32 cursor_index;

    RGBA output_background_color;
    RGBA input_background_color;
    RGBA input_color;
    RGBA output_color;
    RGBA cursor_color;

    Input input;
    OutputHistory output_history;
    InputHistory input_history;
    u32 input_history_index;

    Font output_font;
    Font input_font;
} Console;
global Console console;

// some size constraints
global f32 input_height;
global f32 input_pos_x;
global f32 output_pos_x;
global f32 cursor_height;
global f32 cursor_vertical_padding;
global u8 input_prefix_char;

// how much/fast to open
global f32 console_speed;
global f32 console_t;
global f32 y_closed;
global f32 y_open;
global f32 y_open_big;

static void init_console(Arena* arena);
static bool console_is_open();
static bool console_is_visible();

static void console_cursor_update_pos(u8 c, s32 dir);
static void console_cursor_update_width();
static void console_clear_input();

static void input_add_char(u8 c);
static   u8 input_remove_char();

static void draw_console();
static void update_console();
static bool handle_console_events(Event event);

#endif
