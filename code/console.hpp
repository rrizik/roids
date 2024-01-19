#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;

//array_define(String8, KB(1), OutputHistory);
//array_define(String8, KB(1), InputHistory);
//array_define(u8, KB(1), Input);

typedef struct Console{
    ConsoleState state;
    Font font;

    f32 text_left_pad;

    // timings
    f32 open_dt;
    f32 open_t;
    f32 open_t_target;

    // preset defines
    f32 y_closed;
    f32 y_open;
    f32 y_open_big;


    RGBA output_background_color;
    RGBA input_background_color;
    RGBA input_color;
    RGBA output_color;
    RGBA cursor_color;

#define OUTPUT_HISTORY_COUNT_MAX KB(1)
    String8 output_history[OUTPUT_HISTORY_COUNT_MAX];
    s32 output_history_count;

#define INPUT_HISTORY_COUNT_MAX KB(1)
    String8 input_history[INPUT_HISTORY_COUNT_MAX];
    s32 input_history_count;
    s32 input_history_index;

#define INPUT_COUNT_MAX KB(1)
    u8 input[INPUT_COUNT_MAX];
    s32 input_count;
    s32 cursor_index;
} Console;
global Console console;

static void init_console(Arena* arena);
static bool console_is_open();
static bool console_is_visible();

static void input_add_char(u8 c);
static   u8 input_remove_char();

static void console_set_state(ConsoleState state);
static void console_draw();
static void console_update_openess();
static bool handle_console_events(Event event);
static   u8 console_char_at_cursor();

#endif
