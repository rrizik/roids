#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum ConsoleState{
    CLOSED,
    OPEN,
    OPEN_BIG,
} ConsoleState;

typedef struct Console{
    ConsoleState state;
    Font* font;
    Window* window;
    Arena* arena;

    f32 text_left_pad;

    // time
    f32 open_dt;
    f32 open_t;
    f32 open_t_target;

    // preset defines
    f32 y_closed;
    f32 y_open;
    f32 y_open_big;

    // colors
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

static void init_console(Arena* arena, Window* window, Assets* assets);
static bool console_is_open(void);
static bool console_is_visible(void);
static   u8 console_char_at_cursor(void);
static   u8 console_char_at_cursor(void);

static void input_add_char(u8 c);
static void input_remove_char(void);

static void console_set_state(ConsoleState state);
static bool handle_console_events(Event event);
static void console_update(void);
static void console_draw(void);

#endif
