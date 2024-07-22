/*
INFLUENCE & INSPIRATION:
    A large part of this UI library is heavily influenced and inspired by Ryan Fleury's work, UI Series, code base and mentorship.
    If you are interested in learning more about Ryan's work, you can view his website and UI series here:
    - Website: https://www.rfleury.com
    - UI Series: https://www.rfleury.com/p/ui-part-1-the-interaction-medium
    - Code Base (subscription required): https://git.rfleury.com/
*/

#ifndef UI_H
#define UI_H

typedef u32 Axis;
enum{
    Axis_X,
    Axis_Y,
    Axis_Count,
};

typedef u32 UI_SizeType;
enum {
    UI_SizeType_None,
    UI_SizeType_Pixel,
    UI_SizeType_TextContent,
    UI_SizeType_PercentOfParent,
    UI_SizeType_ChildrenSum,
    UI_SizeType_Count,
};

typedef struct UI_Size{
  UI_SizeType type;
  f32 value;
  f32 strictness;
} UI_Size;

static UI_Size ui_size_make(UI_SizeType type, f32 value, f32 strictness);
#define ui_size_pixel(v, s) ui_size_make(UI_SizeType_Pixel, (v), (s))
#define ui_size_text(s) ui_size_make(UI_SizeType_TextContent, (0), (s))
#define ui_size_percent(v, s) ui_size_make(UI_SizeType_PercentOfParent, (v), (s))
#define ui_size_children(s) ui_size_make(UI_SizeType_ChildrenSum, (0), (s))

typedef u32 UI_BoxFlags;
enum {
  UI_BoxFlag_Clickable       = (1<<0),
  UI_BoxFlag_ViewScroll      = (1<<1),
  UI_BoxFlag_DrawText        = (1<<2),
  UI_BoxFlag_DrawBorder      = (1<<3),
  UI_BoxFlag_DrawBackground  = (1<<4),
  UI_BoxFlag_DrawDropShadow  = (1<<5),
  UI_BoxFlag_Clip            = (1<<6),
  UI_BoxFlag_HotAnimation    = (1<<7),
  UI_BoxFlag_ActiveAnimation = (1<<8),
};

typedef struct TextInfo{
    s32 text_width;
    s32 vertical_offset;
} TextInfo;

typedef struct UI_Box{
    UI_Box *first;
    UI_Box *last;
    UI_Box *next;
    UI_Box *prev;
    UI_Box *parent;

    // hash links
    //UI_Widget *hash_next;
    //UI_Widget *hash_prev;

    // key+generation info
    //UI_Key key;
    //U64 last_frame_touched_index;

    UI_BoxFlags flags;
    String8 string;
    UI_Size semantic_size[Axis_Count];
    f32 pos[Axis_Count];
    f32 rel_pos[Axis_Count];
    f32 size[Axis_Count];
    Axis layout_axis;

    Rect rect;

    bool hot;
    bool active;

    // stack parameters
    RGBA background_color;
} UI_Box;

Arena* ui_arena;
static void init_ui(Arena* arena);
static UI_Size ui_size_make(UI_SizeType type, f32 value, f32 strictness);
static UI_Box* ui_make_box(String8 str, UI_BoxFlags flags);
static bool ui_layout(String8 str);
static bool ui_button(String8 str);
static void ui_traverse_pixel(UI_Box* node, Axis axis);
static void ui_traverse_text(UI_Box* node, Axis axis);
static void ui_traverse_children(UI_Box* node, Axis axis);
static void ui_traverse_position_nodes(UI_Box* node, Axis axis);
static void ui_traverse_construct_rects(UI_Box* node);
//static void ui_traverse(UI_Box* node);
//static void ui_traverse_reverse(UI_Box* node);
static void ui_reset_stacks();


#define ui_stack_push_impl(arena, type, name, v) \
    UI_##type##Node* node = push_struct(arena, UI_##type##Node); \
    node->v = v; \
    node->next = ui_##name##_top; \
    ui_##name##_top = node; \
    return(node->v);

#define ui_stack_pop_impl(type, name) \
    UI_##type##Node* popped = ui_##name##_top; \
    ui_##name##_top = ui_##name##_top->next; \
    return(popped->v);

#define ui_stack_top_impl(name) return(ui_##name##_top->v);

typedef struct UI_BoxNode             { UI_BoxNode* next; UI_Box* v; } UI_BoxNode;
typedef struct UI_PosXNode            { UI_PosXNode* next; f32 v; } UI_PosXNode;
typedef struct UI_PosYNode            { UI_PosYNode* next; f32 v; } UI_PosYNode;
typedef struct UI_SizeWNode           { UI_SizeWNode* next; UI_Size v; } UI_SizeWNode;
typedef struct UI_SizeHNode           { UI_SizeHNode* next; UI_Size v; } UI_SizeHNode;
typedef struct UI_LayoutAxisNode      { UI_LayoutAxisNode* next; Axis v; } UI_LayoutAxisNode;
typedef struct UI_BackgroundColorNode { UI_BackgroundColorNode* next; RGBA v; } UI_BackgroundColorNode;

//UI_Box              ui_box_null = {0};
UI_BoxNode*         ui_box_top = 0;
UI_PosXNode*        ui_pos_x_top = 0;
UI_PosYNode*        ui_pos_y_top = 0;
UI_SizeWNode*       ui_size_w_top = 0;
UI_SizeHNode*       ui_size_h_top = 0;
UI_LayoutAxisNode*  ui_layout_axis_top = 0;
UI_BackgroundColorNode* ui_background_color_top = 0;

static UI_Box* ui_push_box(UI_Box* v)           { ui_stack_push_impl(ui_arena, Box, box, v) }
static f32     ui_push_pos_x(f32 v)             { ui_stack_push_impl(ui_arena, PosX, pos_x, v) }
static f32     ui_push_pos_y(f32 v)             { ui_stack_push_impl(ui_arena, PosY, pos_y, v) }
static UI_Size ui_push_size_w(UI_Size v)        { ui_stack_push_impl(ui_arena, SizeW, size_w, v) }
static UI_Size ui_push_size_h(UI_Size v)        { ui_stack_push_impl(ui_arena, SizeH, size_h, v) }
static Axis    ui_push_layout_axis(Axis v)      { ui_stack_push_impl(ui_arena, LayoutAxis, layout_axis, v) }
static RGBA    ui_push_background_color(RGBA v) { ui_stack_push_impl(ui_arena, BackgroundColor, background_color, v) }

static UI_Box* ui_pop_box(void)              { ui_stack_pop_impl(Box, box) }
static f32     ui_pop_pos_x(void)            { ui_stack_pop_impl(PosX, pos_x) }
static f32     ui_pop_pos_y(void)            { ui_stack_pop_impl(PosY, pos_y) }
static UI_Size ui_pop_size_w(void)           { ui_stack_pop_impl(SizeW, size_w) }
static UI_Size ui_pop_size_h(void)           { ui_stack_pop_impl(SizeH, size_h) }
static Axis    ui_pop_layout_axis(Axis v)    { ui_stack_pop_impl(LayoutAxis, layout_axis) }
static RGBA    ui_pop_background_color(void) { ui_stack_pop_impl(BackgroundColor, background_color) }

static UI_Box* ui_top_box(void)              { ui_stack_top_impl(box) }
static f32     ui_top_pos_x(void)            { ui_stack_top_impl(pos_x) }
static f32     ui_top_pos_y(void)            { ui_stack_top_impl(pos_y) }
static UI_Size ui_top_size_w(void)           { ui_stack_top_impl(size_w) }
static UI_Size ui_top_size_h(void)           { ui_stack_top_impl(size_h) }
static Axis    ui_top_layout_axis(void)      { ui_stack_top_impl(layout_axis) }
static RGBA    ui_top_background_color(void) { ui_stack_top_impl(background_color) }

static void ui_rest_stacks();

#endif
//typedef struct UI_ParentNode UI_ParentNode; struct UI_ParentNode{UI_ParentNode *next; UI_Box * v;};
//typedef struct UI_FlagsNode UI_FlagsNode; struct UI_FlagsNode{UI_FlagsNode *next; UI_BoxFlags v;};
//typedef struct UI_OpacityNode UI_OpacityNode; struct UI_OpacityNode{UI_OpacityNode *next; F32 v;};
//typedef struct UI_TextColorNode UI_TextColorNode; struct UI_TextColorNode{UI_TextColorNode *next; Vec4F32 v;};
//typedef struct UI_BorderColorNode UI_BorderColorNode; struct UI_BorderColorNode{UI_BorderColorNode *next; Vec4F32 v;};
//typedef struct UI_OverlayColorNode UI_OverlayColorNode; struct UI_OverlayColorNode{UI_OverlayColorNode *next; Vec4F32 v;};
//typedef struct UI_FillColorNode UI_FillColorNode; struct UI_FillColorNode{UI_FillColorNode *next; Vec4F32 v;};
//typedef struct UI_CursorColorNode UI_CursorColorNode; struct UI_CursorColorNode{UI_CursorColorNode *next; Vec4F32 v;};
//typedef struct UI_CornerRadius00Node UI_CornerRadius00Node; struct UI_CornerRadius00Node{UI_CornerRadius00Node *next; F32 v;};
//typedef struct UI_CornerRadius01Node UI_CornerRadius01Node; struct UI_CornerRadius01Node{UI_CornerRadius01Node *next; F32 v;};
//typedef struct UI_CornerRadius10Node UI_CornerRadius10Node; struct UI_CornerRadius10Node{UI_CornerRadius10Node *next; F32 v;};
//typedef struct UI_CornerRadius11Node UI_CornerRadius11Node; struct UI_CornerRadius11Node{UI_CornerRadius11Node *next; F32 v;};
//typedef struct UI_BorderThicknessNode UI_BorderThicknessNode; struct UI_BorderThicknessNode{UI_BorderThicknessNode *next; F32 v;};
//typedef struct UI_Slice2F32Node UI_Slice2F32Node; struct UI_Slice2F32Node{UI_Slice2F32Node *next; R_Slice2F32 v;};
//typedef struct UI_FontNode UI_FontNode; struct UI_FontNode{UI_FontNode *next; F_Tag v;};
//typedef struct UI_FontSizeNode UI_FontSizeNode; struct UI_FontSizeNode{UI_FontSizeNode *next; F32 v;};
//typedef struct UI_HoverCursorNode UI_HoverCursorNode; struct UI_HoverCursorNode{UI_HoverCursorNode *next; OS_CursorKind v;};
//typedef struct UI_TextAlignNode UI_TextAlignNode; struct UI_TextAlignNode{UI_TextAlignNode *next; UI_TextAlignment v;};
//typedef struct UI_TextEdgePaddingNode UI_TextEdgePaddingNode; struct UI_TextEdgePaddingNode{UI_TextEdgePaddingNode *next; F32 v;};
//typedef struct UI_SeedKeyNode UI_SeedKeyNode; struct UI_SeedKeyNode{UI_SeedKeyNode *next; UI_Key v;};
//typedef struct UI_FocusHotNode UI_FocusHotNode; struct UI_FocusHotNode{UI_FocusHotNode *next; UI_FocusKind v;};
//typedef struct UI_FocusActiveNode UI_FocusActiveNode; struct UI_FocusActiveNode{UI_FocusActiveNode *next; UI_FocusKind v;};
//
