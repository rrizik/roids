#ifndef INPUT_H
#define INPUT_H

typedef struct Button{
    bool pressed;
    bool held;
} Button;

typedef struct Mouse{
    v2s32 pos;
    f32 dx;
    f32 dy;
    f32 wheel_direction;
    bool tracking_leave;
} Mouse;
global Mouse mouse;

typedef struct Controller{
    Mouse mouse;
    Button q;
    Button e;
    Button up;
    Button down;
    Button left;
    Button right;
    Button three;
    Button four;
    Button m_left;
    Button m_right;
    Button m_middle;
} Controller;
global Controller controller;

static void
clear_controller_pressed(Controller* controller){
    controller->q.pressed = false;
    controller->e.pressed = false;
    controller->left.pressed = false;
    controller->up.pressed = false;
    controller->down.pressed = false;
    controller->left.pressed = false;
    controller->right.pressed = false;
    controller->three.pressed = false;
    controller->four.pressed = false;
    controller->m_left.pressed = false;
    controller->m_right.pressed = false;
    controller->m_middle.pressed = false;
}

// CLEANUP: everything that is commented out, I dont think needs to exist.
typedef enum KeyCode{
    UNKOWN = 0,

    // INCOMPLETE: these need to be looked at again when we do mouse
    MOUSE_BUTTON_LEFT   = 1,
    MOUSE_BUTTON_RIGHT  = 2,
    MOUSE_BUTTON_MIDDLE = 3,

    //SL_MOUSE_BUTTON  = 1,
    //SR_MOUSE_BUTTON  = 2,
    //SLR_MOUSE_BUTTON = 3,
    //SM_MOUSE_BUTTON  = 16,

    BACKSPACE = 8,
    TAB       = 9,
    ENTER     = 13,
    ESCAPE    = 27,
    SPACEBAR  = 32,
    PAGE_UP   = 33,
    PAGE_DOWN = 34,
    END       = 35,
    HOME      = 36,
    //DELETE    = 46,

	//EXCLAMATION = 33,
	//QUOTATION =   34,
	//OCTOTHORP =   35,
	//DOLLAR =      36,
	ARROW_LEFT =     37,
	ARROW_UP =   38,
	ARROW_RIGHT =  39,
	ARROW_DOWN  = 40,
	//CLOSE_PARENTHESIS = 41,
	//ASTERISK =  42,
	//PLUS_SIGN = 43,
    INSERT    = 45,
	//COMMA =  44,
	//HYPHEN = 45,
	//PERIOD = 46,
	//SLASH =  47,
	ZERO  = 48,
	ONE   = 49,
	TWO   = 50,
	THREE = 51,
	FOUR  = 52,
	FIVE  = 53,
	SIX   = 54,
	SEVEN = 55,
	EIGHT = 56,
	NINE  = 57,
	//COLON = 58,
	//SEMICOLON = 59,
	LESS_THAN = 60,
	//EQUALS_TO = 61,
	GREATER_THAN = 62,
	QUESTION_MARK = 63,
	//AT_SIGN = 64,
	A_UPPER = 65,
	B_UPPER = 66,
	C_UPPER = 67,
	D_UPPER = 68,
	E_UPPER = 69,
	F_UPPER = 70,
	G_UPPER = 71,
	H_UPPER = 72,
	I_UPPER = 73,
	J_UPPER = 74,
	K_UPPER = 75,
	L_UPPER = 76,
	M_UPPER = 77,
	N_UPPER = 78,
	O_UPPER = 79,
	P_UPPER = 80,
	Q_UPPER = 81,
	R_UPPER = 82,
	S_UPPER = 83,
	T_UPPER = 84,
	U_UPPER = 85,
	V_UPPER = 86,
	W_UPPER = 87,
	X_UPPER = 88,
	Y_UPPER = 89,
	Z_UPPER = 90,
	//LEFT_SQUARE_BRACKET = 91,
	//BACKSLASH = 92,
	//RIGHT_SQUARE_BRACKET = 93,
	//CARET = 94,
	//UNDERSCORE = 95,
	//GRAVE_ACCENT = 96,
	//A_LOWER = 97,
	//B_LOWER = 98,
	//C_LOWER = 99,
 	//D_LOWER = 100,
 	//E_LOWER = 101,
 	//F_LOWER = 102,
 	//G_LOWER = 103,
 	//H_LOWER = 104,
 	//I_LOWER = 105,
 	//J_LOWER = 106,
 	//K_LOWER = 107,
 	//L_LOWER = 108,
 	//M_LOWER = 119,
 	//N_LOWER = 110,
 	//O_LOWER = 111,
 	//P_LOWER = 112,
 	//Q_LOWER = 113,
 	//R_LOWER = 114,
 	//S_LOWER = 115,
 	//T_LOWER = 116,
 	//U_LOWER = 117,
 	//V_LOWER = 118,
 	//W_LOWER = 119,
 	//X_LOWER = 120,
 	//Y_LOWER = 121,
 	//Z_LOWER = 122,
 	//LEFT_CURLY_BRACE =  123,
 	//VERTICAL_BAR =      124,
 	//RIGHT_CURLY_BRACE = 125,
    F1 = 112,
    F2 = 113,
    F3 = 115,
    F4 = 116,
    F5 = 117,
    F6 = 118,
    F7 = 119,
    F8 = 120,
    F9 = 121,
    F10 = 122,

 	TILDE = 192,
} KeyCode;

// TODO: Verify if I need this.
global bool alt_pressed;
global bool shift_pressed;
global bool ctrl_pressed;

typedef enum EventType{
    QUIT,
    KEYBOARD,
    MOUSE,
    TEXT_INPUT,
} EventType;

typedef struct Event{
    EventType type;
    u64 keycode;

    bool key_pressed;
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool repeat;

    s32 mouse_wheel_dir;
    v2s32 mouse_pos;
    f32 mouse_dx;
    f32 mouse_dy;
} Event;

// NOTE: Must be a size that is a power of 2
typedef struct Events{
    Event e[256];
    u32 write;
    u32 read;
    u32 size;
} Events;
global Events events;

static void
events_init(Events* events){
    events->size = array_count(events->e);
}

static u32
events_count(Events* events){
    u32 result = events->write - events->read;
    return(result);
}

static bool
events_full(Events* events){
    bool result = (events_count(events) == events->size);
    return(result);
}

static bool
events_empty(Events* events){
    bool result = (events->write == events->read);
    return(result);
}

static u32
mask(Events* events, u32 idx){
    u32 result = idx & (events->size - 1);
    return(result);
}

static void
events_add(Events* events, Event event){
    assert(!events_full(events));

    u32 masked_idx = mask(events, events->write++);
    events->e[masked_idx] = event;
}

static Event
events_next(Events* events){
    assert(!events_empty(events));

    u32 masked_idx = mask(events, events->read++);
    Event event = events->e[masked_idx];
    return(event);
}

#endif
