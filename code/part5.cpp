// Add layout infrastructure.

#include "base_inc.h"
#include "win32_base_inc.h"
//#include <stdint.h>
//#include <stdlib.h>
//#include <stddef.h>
//#include <stdbool.h>
//#include <string.h>

#include <windows.h>

/////////////////////////////////////////
// Definitions.
/////////////////////////////////////////

#define UPDATE_HOVERED (1)
#define UPDATE_PRESSED (2)

typedef struct RenderBuffer{
} RenderBuffer;

global bool global_running;

typedef enum Message {
	Message_layout,
	Message_user,
	Message_paint,
	Message_update,

    Message_mouse_move,
    Message_left_down,
    Message_left_up,
    Message_middle_down,
    Message_middle_up,
    Message_right_down,
    Message_right_up,
    Message_mouse_drag,
    Message_clicked,
} Message;

typedef struct Rect {
	f32 l, r, t, b;
} Rect;

typedef struct Painter{
    Rect clip;
    u32 *bits;
    s32 width, height;
} Painter;

struct Element;
typedef s32 (*MessageHandler)(struct Element *element, Message message, s32 di, void *dp);

typedef struct Element {
	u32 flags; // First 16 bits are element specific.
	u32 child_count;
	Rect bounds, clip;
	struct Element *parent;
	struct Element **children;
	struct Window *window;
	void *context_pointer; // Context pointer (for user).
	MessageHandler message_class;
	MessageHandler message_user;
} Element;

typedef struct Button{
    Element e; // The common element header.
    char* text; // THe button's label.
    size_t text_bytes;

} Button;

typedef struct Window {
	Element e;
	u32 *bits;
	u32 width, height;

	HWND hwnd;
	bool tracking_leave;

    Rect update_region;
    s32 cursor_x;
    s32 cursor_y;
    Element* hovered;
    Element* pressed;
    s32 pressed_button; // index of last pressed mouse button (1 = left, 2 = middle, 3 = right)
} Window;

typedef struct GlobalState {
	Window **windows;
	size_t window_count;

} GlobalState;
GlobalState global_state;

Element *element_create(size_t bytes, Element *parent, u32 flags, MessageHandler messageClass);
void element_move(Element *element, Rect bounds, bool always_layout);
s32 element_message(Element *element, Message message, s32 di, void *dp);

Window *window_create(const char *cTitle, s32 width, s32 height);

Rect rect_intersection(Rect a, Rect b);
Rect rect_bounding(Rect a, Rect b);
bool rect_valid(Rect a);
bool rect_equals(Rect a, Rect b);
bool rect_contains(Rect a, s32 x, s32 y);
// TODO Get rid of this and do it another way
void StringCopy(char **destination, size_t *destinationBytes, const char *source, ptrdiff_t sourceBytes);

void draw_string(Painter *painter, Rect r, const char *string, size_t bytes, u32 color, bool centerAlign);
void draw_rect(Painter *painter, Rect r, u32 fill, u32 outline);
void draw_block(Painter *painter, Rect r, u32 fill);


void element_repaint(Element* element, Rect* region);
/////////////////////////////////////////
// Helper functions.
/////////////////////////////////////////

Rect rect_intersection(Rect a, Rect b) {
	if (a.l < b.l) a.l = b.l;
	if (a.t < b.t) a.t = b.t;
	if (a.r > b.r) a.r = b.r;
	if (a.b > b.b) a.b = b.b;
	return a;
}

Rect rect_bounding(Rect a, Rect b) {
	if (a.l > b.l) a.l = b.l;
	if (a.t > b.t) a.t = b.t;
	if (a.r < b.r) a.r = b.r;
	if (a.b < b.b) a.b = b.b;
	return a;
}

bool rect_valid(Rect a) {
	return a.r > a.l && a.b > a.t;
}

bool rect_equals(Rect a, Rect b) {
	return a.l == b.l && a.r == b.r && a.t == b.t && a.b == b.b;
}

bool rect_contains(Rect a, s32 x, s32 y) {
	return a.l <= x && a.r > x && a.t <= y && a.b > y;
}

void StringCopy(char **destination, size_t *destinationBytes, const char *source, ptrdiff_t sourceBytes) {
	if (sourceBytes == -1) sourceBytes = strlen(source);
	*destination = (char *) realloc(*destination, sourceBytes);
	*destinationBytes = sourceBytes;
	memcpy(*destination, source, sourceBytes);
}

/////////////////////////////////////////
// Painting.
/////////////////////////////////////////

#define GLYPH_WIDTH (9)
#define GLYPH_HEIGHT (16)

const uint64_t _font[] = {
	0x0000000000000000UL, 0x0000000000000000UL, 0xBD8181A5817E0000UL, 0x000000007E818199UL, 0xC3FFFFDBFF7E0000UL, 0x000000007EFFFFE7UL, 0x7F7F7F3600000000UL, 0x00000000081C3E7FUL,
	0x7F3E1C0800000000UL, 0x0000000000081C3EUL, 0xE7E73C3C18000000UL, 0x000000003C1818E7UL, 0xFFFF7E3C18000000UL, 0x000000003C18187EUL, 0x3C18000000000000UL, 0x000000000000183CUL,
	0xC3E7FFFFFFFFFFFFUL, 0xFFFFFFFFFFFFE7C3UL, 0x42663C0000000000UL, 0x00000000003C6642UL, 0xBD99C3FFFFFFFFFFUL, 0xFFFFFFFFFFC399BDUL, 0x331E4C5870780000UL, 0x000000001E333333UL,
	0x3C666666663C0000UL, 0x0000000018187E18UL, 0x0C0C0CFCCCFC0000UL, 0x00000000070F0E0CUL, 0xC6C6C6FEC6FE0000UL, 0x0000000367E7E6C6UL, 0xE73CDB1818000000UL, 0x000000001818DB3CUL,
	0x1F7F1F0F07030100UL, 0x000000000103070FUL, 0x7C7F7C7870604000UL, 0x0000000040607078UL, 0x1818187E3C180000UL, 0x0000000000183C7EUL, 0x6666666666660000UL, 0x0000000066660066UL,
	0xD8DEDBDBDBFE0000UL, 0x00000000D8D8D8D8UL, 0x6363361C06633E00UL, 0x0000003E63301C36UL, 0x0000000000000000UL, 0x000000007F7F7F7FUL, 0x1818187E3C180000UL, 0x000000007E183C7EUL,
	0x1818187E3C180000UL, 0x0000000018181818UL, 0x1818181818180000UL, 0x00000000183C7E18UL, 0x7F30180000000000UL, 0x0000000000001830UL, 0x7F060C0000000000UL, 0x0000000000000C06UL,
	0x0303000000000000UL, 0x0000000000007F03UL, 0xFF66240000000000UL, 0x0000000000002466UL, 0x3E1C1C0800000000UL, 0x00000000007F7F3EUL, 0x3E3E7F7F00000000UL, 0x0000000000081C1CUL,
	0x0000000000000000UL, 0x0000000000000000UL, 0x18183C3C3C180000UL, 0x0000000018180018UL, 0x0000002466666600UL, 0x0000000000000000UL, 0x36367F3636000000UL, 0x0000000036367F36UL,
	0x603E0343633E1818UL, 0x000018183E636160UL, 0x1830634300000000UL, 0x000000006163060CUL, 0x3B6E1C36361C0000UL, 0x000000006E333333UL, 0x000000060C0C0C00UL, 0x0000000000000000UL,
	0x0C0C0C0C18300000UL, 0x0000000030180C0CUL, 0x30303030180C0000UL, 0x000000000C183030UL, 0xFF3C660000000000UL, 0x000000000000663CUL, 0x7E18180000000000UL, 0x0000000000001818UL,
	0x0000000000000000UL, 0x0000000C18181800UL, 0x7F00000000000000UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000000018180000UL, 0x1830604000000000UL, 0x000000000103060CUL,
	0xDBDBC3C3663C0000UL, 0x000000003C66C3C3UL, 0x1818181E1C180000UL, 0x000000007E181818UL, 0x0C183060633E0000UL, 0x000000007F630306UL, 0x603C6060633E0000UL, 0x000000003E636060UL,
	0x7F33363C38300000UL, 0x0000000078303030UL, 0x603F0303037F0000UL, 0x000000003E636060UL, 0x633F0303061C0000UL, 0x000000003E636363UL, 0x18306060637F0000UL, 0x000000000C0C0C0CUL,
	0x633E6363633E0000UL, 0x000000003E636363UL, 0x607E6363633E0000UL, 0x000000001E306060UL, 0x0000181800000000UL, 0x0000000000181800UL, 0x0000181800000000UL, 0x000000000C181800UL,
	0x060C183060000000UL, 0x000000006030180CUL, 0x00007E0000000000UL, 0x000000000000007EUL, 0x6030180C06000000UL, 0x00000000060C1830UL, 0x18183063633E0000UL, 0x0000000018180018UL,
	0x7B7B63633E000000UL, 0x000000003E033B7BUL, 0x7F6363361C080000UL, 0x0000000063636363UL, 0x663E6666663F0000UL, 0x000000003F666666UL, 0x03030343663C0000UL, 0x000000003C664303UL,
	0x66666666361F0000UL, 0x000000001F366666UL, 0x161E1646667F0000UL, 0x000000007F664606UL, 0x161E1646667F0000UL, 0x000000000F060606UL, 0x7B030343663C0000UL, 0x000000005C666363UL,
	0x637F636363630000UL, 0x0000000063636363UL, 0x18181818183C0000UL, 0x000000003C181818UL, 0x3030303030780000UL, 0x000000001E333333UL, 0x1E1E366666670000UL, 0x0000000067666636UL,
	0x06060606060F0000UL, 0x000000007F664606UL, 0xC3DBFFFFE7C30000UL, 0x00000000C3C3C3C3UL, 0x737B7F6F67630000UL, 0x0000000063636363UL, 0x63636363633E0000UL, 0x000000003E636363UL,
	0x063E6666663F0000UL, 0x000000000F060606UL, 0x63636363633E0000UL, 0x000070303E7B6B63UL, 0x363E6666663F0000UL, 0x0000000067666666UL, 0x301C0663633E0000UL, 0x000000003E636360UL,
	0x18181899DBFF0000UL, 0x000000003C181818UL, 0x6363636363630000UL, 0x000000003E636363UL, 0xC3C3C3C3C3C30000UL, 0x00000000183C66C3UL, 0xDBC3C3C3C3C30000UL, 0x000000006666FFDBUL,
	0x18183C66C3C30000UL, 0x00000000C3C3663CUL, 0x183C66C3C3C30000UL, 0x000000003C181818UL, 0x0C183061C3FF0000UL, 0x00000000FFC38306UL, 0x0C0C0C0C0C3C0000UL, 0x000000003C0C0C0CUL,
	0x1C0E070301000000UL, 0x0000000040607038UL, 0x30303030303C0000UL, 0x000000003C303030UL, 0x0000000063361C08UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000FF0000000000UL,
	0x0000000000180C0CUL, 0x0000000000000000UL, 0x3E301E0000000000UL, 0x000000006E333333UL, 0x66361E0606070000UL, 0x000000003E666666UL, 0x03633E0000000000UL, 0x000000003E630303UL,
	0x33363C3030380000UL, 0x000000006E333333UL, 0x7F633E0000000000UL, 0x000000003E630303UL, 0x060F0626361C0000UL, 0x000000000F060606UL, 0x33336E0000000000UL, 0x001E33303E333333UL,
	0x666E360606070000UL, 0x0000000067666666UL, 0x18181C0018180000UL, 0x000000003C181818UL, 0x6060700060600000UL, 0x003C666660606060UL, 0x1E36660606070000UL, 0x000000006766361EUL,
	0x18181818181C0000UL, 0x000000003C181818UL, 0xDBFF670000000000UL, 0x00000000DBDBDBDBUL, 0x66663B0000000000UL, 0x0000000066666666UL, 0x63633E0000000000UL, 0x000000003E636363UL,
	0x66663B0000000000UL, 0x000F06063E666666UL, 0x33336E0000000000UL, 0x007830303E333333UL, 0x666E3B0000000000UL, 0x000000000F060606UL, 0x06633E0000000000UL, 0x000000003E63301CUL,
	0x0C0C3F0C0C080000UL, 0x00000000386C0C0CUL, 0x3333330000000000UL, 0x000000006E333333UL, 0xC3C3C30000000000UL, 0x00000000183C66C3UL, 0xC3C3C30000000000UL, 0x0000000066FFDBDBUL,
	0x3C66C30000000000UL, 0x00000000C3663C18UL, 0x6363630000000000UL, 0x001F30607E636363UL, 0x18337F0000000000UL, 0x000000007F63060CUL, 0x180E181818700000UL, 0x0000000070181818UL,
	0x1800181818180000UL, 0x0000000018181818UL, 0x18701818180E0000UL, 0x000000000E181818UL, 0x000000003B6E0000UL, 0x0000000000000000UL, 0x63361C0800000000UL, 0x00000000007F6363UL,
};

static void
draw_block(Painter* painter, Rect rect, u32 color){
    rect = rect_intersection(painter->clip, rect);

    for(u32 y = rect.t; y < rect.b; ++y){
        for(u32 x = rect.l; x < rect.r; ++x){
            painter->bits[y * painter->width + x] = color;
        }
    }
}

static void
draw_rect(Painter* painter, Rect r, u32 main_color, u32 border_color){
    draw_block(painter, (Rect){r.l, r.r, r.t, r.t + 1}, border_color);
	draw_block(painter, (Rect){r.l, r.l + 1, r.t + 1, r.b - 1}, border_color);
	draw_block(painter, (Rect){r.r - 1, r.r, r.t + 1, r.b - 1}, border_color);
	draw_block(painter, (Rect){r.l, r.r, r.b - 1, r.b}, border_color);
	draw_block(painter, (Rect){r.l + 1, r.r - 1, r.t + 1, r.b - 1}, main_color);
}

static void
draw_string(Painter* painter, Rect bounds, const char *string, size_t string_length, u32 color, bool center_align){
    Rect old_clip = painter->clip;
    painter->clip = rect_intersection(bounds, old_clip);
    s32 x = bounds.l;
    s32 y = (bounds.t + bounds.b - GLYPH_HEIGHT) / 2;

    if (center_align){
        x += (bounds.r - bounds.l - string_length * GLYPH_WIDTH) / 2;
    }

    for(u32 i=0; i < string_length; ++i){
        u8 c = string[i];
        if(c > 127){ c = '?'; }

        Rect rect = rect_intersection(painter->clip, (Rect){(f32)x, (f32)x + 8, (f32)y, (f32)y + 16});
        u8 *data = (u8*)_font + c * 16;

        for(s32 i=rect.t; i < rect.b; ++i){
            u32 *bits = painter->bits + i * painter->width + (s32)rect.l;
            u8 byte = data[i - y];

            for(s32 j= rect.l; j < rect.r; ++j){
                if(byte & (1 << (j - x))){
                    *bits = color;
                }
                bits++;
            }
        }
        x += GLYPH_WIDTH;
    }
    painter->clip = old_clip;
}

/////////////////////////////////////////
// Core user interface logic.
/////////////////////////////////////////

static s32
_button_message(Element* element, Message message, s32 di, void* dp){
    Button* button = (Button*) element;

    if(message == Message_paint){
        Painter* painter = (Painter*) dp;
        bool pressed = element->window->pressed == element && element->window->hovered == element;

        u32 c = 0xFFFFFF;
        u32 c1 = pressed ? 0xFFFFFF : 0x000000;
        u32 c2 = pressed ? 0x000000 : c;

        draw_rect(painter, element->bounds, c2, c1);
        draw_string(painter, element->bounds, button->text, button->text_bytes, c1, true);
    }
    else if (message == Message_update){
        element_repaint(element, 0);
    }
    return(0);
}

static Button*
button_create(Element* parent, u32 flags, // by convention, all element creation functions start with these 2 paremeters
              const char* text, // the button's label
              size_t text_bytes /* -1 indicates a zero terminated string)*/){

    Button* button = (Button*)element_create(sizeof(Button), parent, flags, _button_message);

    StringCopy(&button->text, &button->text_bytes, text, text_bytes);

    return(button);
}

static Element*
element_find_by_point(Element* element, s32 x, s32 y){
    for(u32 i=0; i < element->child_count; ++i){
        if(rect_contains(element->children[i]->clip, x, y)){
            return(element_find_by_point(element->children[i], x, y));
        }
    }

    return(element);
}

static void
_element_paint(Element* element, Painter* painter){
    Rect clip = rect_intersection(element->clip, painter->clip);

    if(!rect_valid(clip)){
        return;
    }

    painter->clip = clip;
    element_message(element, Message_paint, 0, painter);

    for(u32 i=0; i < element->child_count; ++i){
        painter->clip = clip;
        _element_paint(element->children[i], painter);
    }
}

static void
_window_end_paint(Window* window, Painter* painter){
    (void) painter;
    HDC dc = GetDC(window->hwnd);
    BITMAPINFOHEADER info = {0};
    info.biSize = sizeof(info);
    info.biWidth = window->width;
    info.biHeight = window->height;
    info.biPlanes = 1;
    info.biBitCount = 32;

    StretchDIBits(dc,
		window->update_region.l, window->update_region.t,
		window->update_region.r - window->update_region.l, window->update_region.b - window->update_region.t,
		window->update_region.l, window->update_region.b + 1,
		window->update_region.r - window->update_region.l, window->update_region.t - window->update_region.b,
		window->bits, (BITMAPINFO *) &info, DIB_RGB_COLORS, SRCCOPY);
	ReleaseDC(window->hwnd, dc);
}

static void
_update(){
    for(u32 i=0; i < global_state.window_count; ++i){
        Window* window = global_state.windows[i];

        if(rect_valid(window->update_region)){
            Painter painter;
            painter.bits = window->bits;
            painter.width = window->width;
            painter.height = window->height;
            painter.clip = rect_intersection((Rect){0, (f32)window->width, 0, (f32)window->height}, window->update_region);

            _element_paint(&window->e, &painter);
            _window_end_paint(window, &painter);

            window->update_region = (Rect){0, 0, 0, 0};
        }
    }
}

static void
_window_set_pressed(Window* window,
                    Element* element, // null if the mouse button is not being pressed
                    s32 button // the button that went up or down
                    ){
    Element* previous = window->pressed;

    window->pressed = element;
    window->pressed_button = button;

    if(previous){
        element_message(previous, Message_update, UPDATE_PRESSED, 0);
    }
    if(element){
        element_message(element, Message_update, UPDATE_PRESSED, 0);
    }
}

static void
_window_input_event(Window* window, Message message, s32 di, void* dp){

    if(window->pressed){
        if(message == Message_mouse_move){
            element_message(window->pressed, Message_mouse_drag, di, dp);
        }
        else if(message == Message_left_up && window->pressed_button == 1){
            if(window->hovered == window->pressed){
                element_message(window->pressed, Message_clicked, di, dp);
            }

            element_message(window->pressed, Message_left_up, di, dp);
            _window_set_pressed(window, 0, 1);
        }
        else if(message == Message_middle_up && window->pressed_button == 2){
            element_message(window->pressed, Message_middle_up, di, dp);
            _window_set_pressed(window, 0 , 2);
        }
        else if(message == Message_right_up && window->pressed_button == 3){
            element_message(window->pressed, Message_right_up, di, dp);
            _window_set_pressed(window, 0 , 3);
        }
    }

    if(window->pressed){
        // While a mouse button is held, the hovered element is either the pressed element,
		// or the window element (at the root of the hierarchy).
		// Other elements are not allowed to be considered hovered until the button is released.
		// Here, we update the hovered field and send out MSG_UPDATE messages as necessary.

        bool inside = rect_contains(window->pressed->clip, window->cursor_x, window->cursor_y);
        if(inside && window->hovered == &window->e){
            window->hovered = window->pressed;
            element_message(window->pressed, Message_update, UPDATE_HOVERED, 0);
        }
        else if(!inside && window->hovered == window->pressed){
            window->hovered = &window->e;
            element_message(window->pressed, Message_update, UPDATE_HOVERED, 0);
        }
    }
    else{
        // no element is pressed. find the hovered element
        Element* hovered = element_find_by_point(&window->e, window->cursor_x, window->cursor_y);

        if(message == Message_mouse_move){
            element_message(hovered, Message_mouse_move, di, dp);
        }
        else if(message == Message_left_down){
            _window_set_pressed(window, hovered, 1);
            element_message(hovered, message, di, dp);
        }
        else if(message == Message_middle_down){
            _window_set_pressed(window, hovered, 2);
            element_message(hovered, message, di, dp);
        }
        else if(message == Message_right_down){
            _window_set_pressed(window, hovered, 3);
            element_message(hovered, message, di, dp);
        }

        if(hovered != window->hovered){
            Element* previous = window->hovered;
            window->hovered = hovered;
            element_message(previous, Message_update, UPDATE_HOVERED, 0);
            element_message(window->hovered, Message_update, UPDATE_HOVERED, 0);
        }
    }

    _update();
}

static void
element_repaint(Element* element, Rect* region){
    // if region not specified, use the whole bounds of the element.
    if(!region){
        region = &element->bounds;
    }

    // intersect the region to repaint with the elements clip
    Rect r = rect_intersection(*region, element->clip);

    if(rect_valid(r)){
        if(rect_valid(element->window->update_region)){
            element->window->update_region = rect_bounding(element->window->update_region, r);
        }
        else{
            element->window->update_region = r;
        }
    }
}

static void
element_move(Element *element, Rect bounds, bool always_layout) {
	Rect oldClip = element->clip;
	element->clip = rect_intersection(element->parent->clip, bounds);

	if (!rect_equals(element->bounds, bounds) || !rect_equals(element->clip, oldClip) || always_layout) {
		element->bounds = bounds;
		element_message(element, Message_layout, 0, 0);
	}
}

static s32
element_message(Element *element, Message message, s32 di, void *dp) {
	if (element->message_user) {
		s32 result = element->message_user(element, message, di, dp);

		if (result) {
			return result;
		}
	}

	if (element->message_class) {
		return element->message_class(element, message, di, dp);
	} else {
		return 0;
	}
}

Element *element_create(size_t bytes, Element *parent, u32 flags, MessageHandler messageClass)  {
	Element *element = (Element *)calloc(1, bytes);
	element->flags = flags;
	element->message_class = messageClass;

	if (parent) {
		element->window = parent->window;
		element->parent = parent;
		parent->child_count++;
		parent->children = (Element**)realloc(parent->children, sizeof(Element *) * parent->child_count);
		parent->children[parent->child_count - 1] = element;
	}

	return element;
}

/////////////////////////////////////////
// Platform specific code.
/////////////////////////////////////////

s32 default_window_message(Element *element, Message message, s32 di, void *dp) {
	(void) di;
	(void) dp;

	if (message == Message_layout && element->child_count) {
		element_move(element->children[0], element->bounds, false);
        element_repaint(element, 0);
	}

	return 0;
}

static Window*
win32_create_window(wchar *window_title, s32 width, s32 height) {
	Window *window = (Window*)element_create(sizeof(Window), NULL, 0, default_window_message);
    window->hovered = &window->e;
	window->e.window = window;
	global_state.window_count++;
	global_state.windows = (Window**)realloc(global_state.windows, sizeof(Window *) * global_state.window_count);
	global_state.windows[global_state.window_count - 1] = window;

    window->hwnd = CreateWindowW(L"window class", window_title, WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, GetModuleHandle(0), 0);
    assert(IsWindow(window->hwnd));

	SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR) window);
	PostMessage(window->hwnd, WM_SIZE, 0, 0);

	return(window);
}

/////////////////////////////////////////
// Test usage code.
/////////////////////////////////////////

#include <stdio.h>

Element *element_a, *element_b, *element_c, *element_d;

s32 element_a_message(Element *element, Message message, s32 di, void *dp) {
	(void) di;

	Rect bounds = element->bounds;

	if (message == Message_paint) {
        draw_block((Painter*)dp, bounds, 0xFF77FF);
    }
    else if (message == Message_layout) {
		print("layout A with bounds (%f->%f;%f->%f)\n", bounds.l, bounds.r, bounds.t, bounds.b);
		element_move(element_b, (Rect){bounds.l + 20, bounds.r - 20, bounds.t + 20, bounds.b - 20}, false);
	}

	return 0;
}

s32 element_b_message(Element *element, Message message, s32 di, void *dp) {
	(void) di;

	Rect bounds = element->bounds;

	if (message == Message_paint) {
        draw_block((Painter*)dp, bounds, 0xDDDDE0);
    }
    else if (message == Message_layout) {
		print("layout B with bounds (%f->%f;%f->%f)\n", bounds.l, bounds.r, bounds.t, bounds.b);
		element_move(element_c, (Rect){bounds.l - 40, bounds.l + 40, bounds.t + 40, bounds.b - 40}, false);
		element_move(element_d, (Rect){bounds.r - 40, bounds.r + 40, bounds.t + 40, bounds.b - 40}, false);
	}

	return 0;
}

s32 element_c_message(Element *element, Message message, s32 di, void *dp) {
	(void) di;

	Rect bounds = element->bounds;
	Rect clip = element->clip;

	if (message == Message_paint) {
        draw_block((Painter*)dp, bounds, 0x3377FF);
    }
    else if (message == Message_layout) {
		print("layout C with bounds (%f->%f;%f->%f)\n", bounds.l, bounds.r, bounds.t, bounds.b);
		print("\tclipped to (%f->%f;%f->%f)\n", clip.l, clip.r, clip.t, clip.b);
	}

	return 0;
}

s32 element_d_message(Element *element, Message message, int di, void *dp) {
	(void) di;

	Rect bounds = element->bounds;
	Rect clip = element->clip;

	if (message == Message_paint) {
        draw_block((Painter*)dp, bounds, 0x33CC33);
    }
    else if (message == Message_layout) {
		print("layout D with bounds (%f->%f;%f->%f)\n", bounds.l, bounds.r, bounds.t, bounds.b);
		print("\tclipped to (%f->%f;%f->%f)\n", clip.l, clip.r, clip.t, clip.b);
	}

	return 0;
}

LRESULT win_message_callback(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param){
    LRESULT result = 0;

    Window* window = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(!window){
        return(DefWindowProcW(hwnd, message, w_param, l_param));
    }

    switch(message){
        case WM_SIZE:{
            RECT client;
            GetClientRect(hwnd, &client);
            window->width = client.right;
            window->height = client.bottom;
            window->bits = (u32*)realloc(window->bits, window->width * window->height * 4);
            window->e.bounds = (Rect){0, (f32)window->width, 0, (f32)window->height};
            window->e.clip = (Rect){0, (f32)window->width, 0, (f32)window->height};
            element_message(&window->e, Message_layout, 0, 0);
            _update();
        } break;
        case WM_PAINT:{
            PAINTSTRUCT paint;
            //BeginPaint(hwnd, &paint);
            HDC dc = BeginPaint(hwnd, &paint);
            BITMAPINFOHEADER info = {0};
            info.biSize = sizeof(info);
            info.biWidth = window->width, info.biHeight = -window->height;
            info.biPlanes = 1, info.biBitCount = 32;
            StretchDIBits(dc, 0, 0, window->e.bounds.r - window->e.bounds.l, window->e.bounds.b - window->e.bounds.t,
                    0, 0, window->e.bounds.r - window->e.bounds.l, window->e.bounds.b - window->e.bounds.t,
                    window->bits, (BITMAPINFO *) &info, DIB_RGB_COLORS, SRCCOPY);
            //update_window(hwnd, render_buffer);
            EndPaint(hwnd, &paint);
        } break;
        case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:{
            OutputDebugStringA("quiting\n");
            global_running = false;
        } break;
        case WM_MOUSEMOVE: {
            if(!window->tracking_leave){
                window->tracking_leave = true;
                TRACKMOUSEEVENT leave = {0};
                leave.cbSize = sizeof(TRACKMOUSEEVENT);
                leave.dwFlags = TME_LEAVE;
                leave.hwndTrack = hwnd;
                TrackMouseEvent(&leave);
            }

            POINT cursor;
            GetCursorPos(&cursor);
            ScreenToClient(hwnd, &cursor);
            window->cursor_x = cursor.x;
            window->cursor_y = cursor.y;
            _window_input_event(window, Message_mouse_move, 0, 0);
        } break;
        case WM_MOUSELEAVE:{
            window->tracking_leave = false;
            if(!window->pressed){
                window->cursor_x = -1;
                window->cursor_y = -1;
            }
            _window_input_event(window, Message_mouse_move, 0, 0);
        } break;
        case WM_LBUTTONDOWN:{
            SetCapture(hwnd);
            _window_input_event(window, Message_left_down, 0, 0);
        } break;
        case WM_LBUTTONUP:{
            if (window->pressed_button == 1) ReleaseCapture();
            _window_input_event(window, Message_left_up, 0, 0);
        } break;
        case WM_MBUTTONDOWN:{
            SetCapture(hwnd);
            _window_input_event(window, Message_middle_down, 0, 0);
        } break;
        case WM_MBUTTONUP:{
            if (window->pressed_button == 2) ReleaseCapture();
            _window_input_event(window, Message_middle_up, 0, 0);
        } break;
        case WM_RBUTTONDOWN:{
            SetCapture(hwnd);
            _window_input_event(window, Message_right_down, 0, 0);
        } break;
        case WM_RBUTTONUP:{
            if (window->pressed_button == 3) ReleaseCapture();
            _window_input_event(window, Message_right_up, 0, 0);
        } break;
        //case WM_LBUTTONUP:{ controller.m_left.held = false; } break;
        //case WM_RBUTTONUP:{ controller.m_right.held = false; } break;
        //case WM_MBUTTONUP:{ controller.m_middle.held = false; } break;
        //case WM_LBUTTONDOWN:{ controller.m_left = {true, true}; } break;
        //case WM_RBUTTONDOWN:{ controller.m_right = {true, true}; } break;
        //case WM_MBUTTONDOWN:{ controller.m_middle = {true, true}; } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:{
            bool was_down = ((l_param & (1 << 30)) != 0);
            bool is_down = ((l_param & (1 << 31)) == 0);
            if(is_down != was_down){
                switch(w_param){
                //    case 'W':{
                //        controller.up.held = is_down;
                //        controller.up.pressed = is_down;
                //    } break;
                //    case 'S':{
                //        controller.down.held = is_down;
                //        controller.down.pressed = is_down;
                //    } break;
                //    case 'A':{
                //        controller.left.held = is_down;
                //        controller.left.pressed = is_down;
                //    } break;
                //    case 'D':{
                //        controller.right.held = is_down;
                //        controller.right.pressed = is_down;
                //    } break;
                //    case '1':{
                //        controller.one.held = is_down;
                //        controller.one.pressed = is_down;
                //    } break;
                //    case '2':{
                //        controller.two.held = is_down;
                //        controller.two.pressed = is_down;
                //    } break;
                //    case '3':{
                //        controller.three.held = is_down;
                //        controller.three.pressed = is_down;
                //    } break;
                //    case '4':{
                //        controller.four.held = is_down;
                //        controller.four.pressed = is_down;
                //    } break;
                    case VK_ESCAPE:{
                        OutputDebugStringA("quiting\n");
                        global_running = false;
                    } break;
                }
            }
        } break;
        default:{
            result = DefWindowProcW(hwnd, message, w_param, l_param);
        } break;
    }
    return(result);
}

static bool
win32_init(){
    WNDCLASSW window_class = {
        .style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC,
        .lpfnWndProc = win_message_callback,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = L"window class",
    };

    if(RegisterClassW(&window_class)){
        return(true);
    }

    return(false);
}

Element *parent_element, *child_element;

s32 parent_element_message(Element *element, Message message, s32 di, void *dp) {
	if (message == Message_paint) {
		draw_block((Painter*) dp, element->bounds, 0xFFCCFF);
	} else if (message == Message_layout) {
        print("layout parent with bounds (%d->%d;%d->%d)\n", element->bounds.l, element->bounds.r, element->bounds.t, element->bounds.b);
		element_move(child_element, (Rect){50, 100, 50, 100}, false);
    } else if (message == Message_mouse_move) {
		print("mouse move at (%d,%d)\n", element->window->cursor_x, element->window->cursor_y);
	} else if (message == Message_mouse_drag) {
		print("mouse drag at (%d,%d)\n", element->window->cursor_x, element->window->cursor_y);
	} else if (message == Message_update) {
		print("update %d\n", di);
	} else if (message == Message_left_down) {
		print("left down\n");
	} else if (message == Message_right_down) {
		print("right down\n");
	} else if (message == Message_middle_down) {
		print("middle down\n");
	} else if (message == Message_left_up) {
		print("left up\n");
	} else if (message == Message_right_up) {
		print("right up\n");
	} else if (message == Message_middle_up) {
		print("middle up\n");
	} else if (message == Message_clicked) {
		print("clicked\n");
	}

	return 0;
}

s32 child_element_message(Element *element, Message message, s32 di, void *dp) {
    (void) di;

    if(message == Message_paint){
        draw_block((Painter*)dp, element->bounds, 0x444444);
    }
    return(0);
}

s32 WinMain(HINSTANCE instance, HINSTANCE pinstance, LPSTR command_line, s32 window_type){
    bool succeed = win32_init();
    assert(succeed);

	Window *window = win32_create_window(L"Hello, world", 300, 200);
    assert(IsWindow(window->hwnd));

    global_running = true;

    parent_element = element_create(sizeof(Element), &window->e, 0, parent_element_message);
	child_element = element_create(sizeof(Element), parent_element, 0, child_element_message);

    while(global_running){
        MSG message;
        while(PeekMessageW(&message, window->hwnd, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
}

