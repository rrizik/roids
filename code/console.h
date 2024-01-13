#ifndef CONSOLE_H
#define CONSOLE_H

#include "command.h"

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;

#define INPUT_MAX_COUNT KB(4)
#define CONSOLE_HISTORY_MAX KB(1)
#define COMMAND_HISTORY_MAX KB(1)
typedef struct Console{
    ConsoleState state;

    Rect output_rect;
    Rect input_rect;
    Rect cursor_rect;
    v2 history_pos;

    RGBA history_background_color;
    RGBA history_color;
    RGBA input_background_color;
    RGBA input_color;
    RGBA output_color;
    RGBA cursor_color;
    u32  cursor_index;

    // TODO:INCOMPLETE:why not do String8 here?
    u8 input[INPUT_MAX_COUNT];
    u32 input_char_count;

    String8 output_history[CONSOLE_HISTORY_MAX];
    u32 output_history_count;
    u32 output_history_at;

    String8 command_history[COMMAND_HISTORY_MAX];
    u32 command_history_count;
    u32 command_history_at;

    Font output_font;
    Font input_font;
    Font command_history_font;
} Console;
global Console console;

// some size constraints
global f32 input_height  = 28;
global f32 cursor_height = 24;
global f32 cursor_width  = 10;
global f32 cursor_vertical_padding = 2;

// how much/fast to open
global f32 console_speed = 0.5f;
global f32 console_t  = 0.0f;
global f32 y_closed   = 1.0f;
global f32 y_open     = .7f;
global f32 y_open_big = .2f;

static void init_console(Arena* arena);
static bool console_is_open();
static bool console_is_visible();
static void console_cursor_reset();
static void console_clear_input();
static void input_add_char(u8 c);
static void input_remove_char();
static void console_store_output(String8 str);
static void console_store_command(String8 str);
static void draw_console();
static void update_console();
static bool handle_console_event(Event event);

#endif
